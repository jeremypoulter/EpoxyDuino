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
static std::atomic<bool> stop_start(true);  // kind of mutex for start() and stop()
std::atomic<unsigned long> Injector::time_forward_us{0};

static std::atomic<int> instances(0); // Only one instance allowed

Injector::Injector()
  : thr(Injector::loop)
{
  EpoxyTest::registerReset(strong_stop);
  assert(instances == 0);
  instances++;
}

Injector::~Injector()
{
  run = false;
  thr.join();
  events.clear();
  instances--;
  auto us = time_forward_us.exchange(0);
  epoxy_micros += us;
}

void Injector::start()
{
  if (stop_start.exchange(false))
  {
    if (injector == nullptr)
    {
      injector = std::unique_ptr<Injector>(new Injector());
      while(not run);
    }
    stop_start = true;
  }
}

void Injector::strong_stop()
{
  while(eventsSize())
  {
    Injector::stop();
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
    {
      std::lock_guard<std::mutex> lock(events_mutex);
      unsigned long us = micros();
      while(events.size() and events.begin()->second == nullptr)
        events.erase(events.begin());
      if  (events.size())
      {
        auto it = events.begin();
        unsigned long next = it->first;
        if (us >= it->first)
        {
          Events to_insert;
          while(events.size() and it->first == next)
          {
            if (it->second)
            {
              auto& event = it->second;
              long jitter = micros() - next;
              ep_debug("injector raise expected " << next << ", jitter=" << jitter << ", max=" << maxJitter_ << ": " << event->name());
              if (jitter > maxJitter_) maxJitter_ = jitter;
              long sched = event->raise();
              auto& chain = event->chain;
              us = micros();
              if (chain)
              {
                ep_debug("injector chain " << chain->us() << ", us=" << us << chain->name());
                to_insert.insert(std::make_pair(chain->us(), std::move(chain)));
              }
              if (sched)
              {
                ep_debug("injector sched " << sched << ", sz=" << events.size());
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
      }
      // Simulated time
      if (time_forward_us)
      {
        auto it = events.begin();
        unsigned long add_us = 10;
        if (it != events.end())
        {
          while(it != events.end())
          {
            if (it->second)
            {
              if (it->first <= micros())
                add_us = 0;
              break;
            }
            it++;
          }
        }
        if (add_us > time_forward_us)
        {
          epoxy_micros += time_forward_us;
          time_forward_us = 0;
        }
        else
        {
          epoxy_micros+=add_us;
          time_forward_us -= add_us;
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

void Injector::delay_us(unsigned long us)
{
  if (injector)
  {
    time_forward_us += us;
    while(time_forward_us);
  }
  else
    epoxy_micros += us;
}

}
