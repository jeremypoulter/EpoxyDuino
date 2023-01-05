#ifdef EPOXY_TEST

#include "Injection.h"
#include "../Arduino.h"

#include <unistd.h>
#include <iostream>
#include <thread>

extern std::atomic<unsigned long> epoxy_micros;
extern bool epoxy_real_time;

using namespace std;

namespace EpoxyInjection
{

Injector Injector::injector;
Injector::Events Injector::events;
std::mutex Injector::events_mutex;
std::atomic<bool> Injector::run{false};
std::atomic<bool> Injector::do_reset{false};

Event::Event(unsigned long time_us)
{
  us_ = time_us;
}

Injector::Injector()
{
  EpoxyTest::registerReset(reset);
  std::thread(*this).detach();
  while(not run);
}

Injector::~Injector()
{
  run = false;
}

void Injector::reset()
{
  do_reset = true;
  while(do_reset);
}

void Injector::addEvent(std::unique_ptr<Event> event)
{
  std::lock_guard<std::mutex> lock(events_mutex);
  events.insert(std::make_pair(event->us(), std::move(event)));
}

size_t Injector::eventsSize()
{
  std::lock_guard<std::mutex> lock(mutex);
  return events.size();
}

void Injector::operator()()
{
  run = true;
  while(run)
  {
    long sleep_us=100;
    {
      std::lock_guard<std::mutex> lock(events_mutex);
      auto us = micros();
      while (events.size())
      {
        auto it = events.begin();
        unsigned long next = it->first;
        if (us >= next)
        {
          unsigned long sched = it->second->raise();
          us = micros();
          if (it->second->chain)
          {
            sleep_us = std::min(sleep_us, (long)(it->second->chain->us())-(long)us);
            auto evt = std::move(it->second->chain);
            events.erase(it);
            events.insert(std::make_pair( evt->us(), std::move(evt)));
            break;
          }
          if (sched)
          {
            auto evt = std::move(it->second);
            sleep_us = std::min(sleep_us, (long)sched);
            events.erase(it);
            events.insert(std::make_pair( sched, std::move(evt)));
            break;
          }
          events.erase(it);
        }
        else
        {
          sleep_us = std::min(sleep_us, (long)next -  (long)us);
          break;
        }
      }
      if (do_reset)
      {
        events.clear();
        do_reset = false;
      }
    }
    if (sleep_us > 0) usleep(sleep_us);
  }
}

}

#endif
