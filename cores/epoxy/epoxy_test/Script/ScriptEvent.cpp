#ifdef EPOXY_TEST

#include "ScriptEvent.h"
#include "ScriptParser.h"

namespace EpoxyTest
{

ScriptEvent::ScriptEvent(unsigned long us, Script* script)
    : Event(us)
    , ScriptParser(script->getLine(), script->getLineNr())
    , parent(script)
{
}

unsigned long ScriptEvent::raise()
{
  unsigned long next = 0;
  unsigned long resched = 0;
  std::cout << micros() << " raise " << line << std::endl << std::flush;
  std::string action = getWord();
  if (action=="#" or action=="//")
  {
    line.clear();
  }
  else if (action == "at")
  {
    getDuration(resched);
  }
  else if (action == "pin")
  {
    int pin = getPinNumber();
    int val = getPinNumber();
    // std::cout << micros() << " Setting pin " << pin << " to " << val << std::endl;
    digitalReadValue(pin, val);
  }
  else if (action == "wait")
  {
    getDuration(next);
  }
  else if (action == "sync")
  {
    std::string org = "sync "+line;
    auto us = micros();
    unsigned long t;
    getDuration(t);
    if (us < t)
    {
      line = org;
      return t; // rechedule
    }
  }
  else
  {
    error(std::string("Unknown script command: ")+line);
  }
  trim();
  if (eat("//", true)) line.clear();
  if (eat("#", true)) line.clear();
  if (line.length() and resched == 0)
    error("Garbage");
  chain = parent->step(next);
  return resched;
}

} // ns

#endif // EPOXY_TEST
