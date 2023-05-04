#include "Event.h"
#include "Injection.h"
#include <iostream>

using namespace std;

namespace EpoxyInjection
{

std::atomic<int> Event::count(0);

Event::Event(unsigned long us)
  : us_(us)
{
  count++;
  if (count == 1)
  {
    Injector::start();
  }
}

Event::~Event()
{
  count--;
}

}
