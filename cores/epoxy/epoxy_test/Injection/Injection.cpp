#include <assert.h>
#include <unistd.h>
#include <iostream>
#include <thread>
#include <set>
#include <vector>
#include <chrono>

#include <Arduino.h>
#include "Injection.h"
#include "Event.h"

using namespace std;

extern std::atomic<unsigned long> epoxy_micros;
extern bool epoxy_real_time;

namespace EpoxyInjection
{

Injector::Events Injector::events;
std::mutex Injector::events_mutex;
std::atomic<bool> Injector::run{false};
std::atomic<bool> Injector::do_reset{false};
std::atomic<long> Injector::maxJitter_(0);
static std::unique_ptr<Injector> injector;
static std::atomic<bool> stop_start(true);

static std::atomic<int> instances(0); // Only one instance allowed

Injector::Injector()
  : thr(Injector::loop)
{
  assert(instances == 0);
  instances++;
}

Injector::~Injector()
{
  run = false;
  thr.join();
  events.clear();
  instances--;
}

void Injector::start()
{
  if (stop_start.exchange(false))
  {
    if (injector == nullptr)
    {
      injector = std::unique_ptr<Injector>(new Injector());
      EpoxyTest::registerReset(stop);
      while(not run);
    }
    stop_start = true;
  }
}

void Injector::stop()
{
  if (stop_start.exchange(false))
  {
    if (injector) injector.reset();
    stop_start = true;
  }
}

void Injector::addEvent(std::unique_ptr<Event> event)
{
  std::lock_guard<std::mutex> lock(events_mutex);
  events.insert(std::make_pair(event->us(), std::move(event)));
}

size_t Injector::eventsSize()
{
  std::lock_guard<std::mutex> lock(events_mutex);
  return events.size();
}

void sleepus(unsigned long microseconds)
{
  assert(microseconds < 1000000);
  struct timespec ts;
  ts.tv_sec = 0;
  ts.tv_nsec = microseconds * 1000;
  nanosleep(&ts, NULL);
}

void Injector::loop()
{
  run = true;
  while(run)
  {
    long sleep_us=500;
    {
      std::lock_guard<std::mutex> lock(events_mutex);
      long us = micros();
      while(events.size() and events.begin()->second == nullptr)
        events.erase(events.begin());
      if  (events.size())
      {
        auto it = events.begin();
        long next = it->first;
        if (us >= it->first)
        {
          Events to_insert;
          while(events.size() and it->first == next)
          {
            if (it->second)
            {
              auto& event = it->second;
              long jitter = micros() - next;
              ep_debug("injector raise expected " << next << ", jitter=" << jitter << ", max=" << maxJitter_);
              if (jitter > maxJitter_) maxJitter_ = jitter;
              long sched = event->raise();
              auto& chain = event->chain;
              us = micros();
              if (chain)
              {
                sleep_us = std::min(sleep_us, chain->us() - us);
                ep_debug("injector chain " << chain->us() << ", sleep=" << sleep_us << ", us=" << us << ": " << chain->name());
                to_insert.insert(std::make_pair(chain->us(), std::move(chain)));
              }
              if (sched)
              {
                ep_debug("injector sched " << sched << ", sz=" << events.size());
                sleep_us = std::min(sleep_us, sched - us);
                to_insert.insert(std::make_pair(sched, std::move(event)));
              }
              it->second.reset();
            }
            it++;
          }
          for(auto& pair: to_insert)
          {
            events.insert(std::move(pair));
          }
        }
        else
        {
          sleep_us = std::min(sleep_us, next -  us);
        }
      }
      if (do_reset)
      {
        events.clear();
        do_reset = false;
      }
    }
  }
}

}
