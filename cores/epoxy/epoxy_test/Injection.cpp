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
  us_ = micros()+time_us;
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
  std::lock_guard<std::mutex> lock(mutex);
  events.insert(std::make_pair(event->us(), std::move(event)));
}

size_t Injector::eventsSize()
{
  std::lock_guard<std::mutex> lock(mutex);
  return events.size();
}

void Injector::operator()()
{
  int thread=0;
  run = true;
  while(run)
  {
    unsigned long sleep_us=100;
    {
      std::lock_guard<std::mutex> lock(events_mutex);
      auto us = micros();
      while (events.size())
      {
        auto it = events.begin();
        unsigned long next = it->first;
        // std::cout << "microseconds: " << us << ", next: " << next << std::endl;
        if (us >= next)
        {
          it->second->raise();
          events.erase(it);
        }
        else
          break;
      }
      if (do_reset)
      {
        events.clear();
        do_reset = false;
      }
    }
    usleep(sleep_us);
  }
}

}

#endif
