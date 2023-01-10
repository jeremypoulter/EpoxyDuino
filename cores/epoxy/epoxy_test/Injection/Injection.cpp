#ifdef EPOXY_TEST

#include <unistd.h>
#include <iostream>
#include <thread>
#include <vector>

#include <Arduino.h>
#include "Injection.h"
#include "Event.h"

#if 0
#define debug(args) { std::cout  << micros() << ' ' << args << std::endl; }
#else
#define debug(args)
#endif

using namespace std;

extern std::atomic<unsigned long> epoxy_micros;
extern bool epoxy_real_time;

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
      if  (events.size())
      {
        std::vector<Injector::Events::iterator> to_delete;
        std::vector<Injector::Events::value_type> to_insert;
        auto it = events.begin();
        unsigned long next = it->first;
        if (us >= next)
        {
          while(it->first == next)
          {
            debug("injector raise " << next);
            unsigned long sched = it->second->raise();
            us = micros();
            auto& chain = it->second->chain;
            if (chain or sched)
            {
              if (chain)
              {
                debug("injector chain " << chain->name());
                sleep_us = std::min(sleep_us, (long)(chain->us())-(long)us);
                auto evt = std::move(chain);
                to_delete.push_back(it);
                to_insert.push_back(std::make_pair( evt->us(), std::move(evt)));
              }
              if (sched)
              {
                debug("injector sched " << sched);
                auto evt = std::move(it->second);
                sleep_us = std::min(sleep_us, (long)sched);
                to_delete.push_back(it);
                to_insert.push_back(std::make_pair( sched, std::move(evt)));
              }
            }
            else
              to_delete.push_back(it);
            it++;
          }
          if (to_delete.size()) for(auto& it: to_delete) events.erase(it);
          if (to_insert.size()) for(auto& pair: to_insert) events.insert(std::move(pair));
        }
        else
        {
          sleep_us = std::min(sleep_us, (long)next -  (long)us);
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
