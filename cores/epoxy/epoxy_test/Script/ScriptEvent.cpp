#include "ScriptEvent.h"
#include "ScriptParser.h"

namespace EpoxyTest
{

ScriptEvent::ScriptEvent(unsigned long us, Script* script)
  : Event(us)
  , ScriptParser(script->getLine(), script->getLineNr())
  , script(script)
{
}

// registry constructors only
ScriptEvent::ScriptEvent(unsigned long us, Script* script, std::string& params)
  : Event(us)
  , ScriptParser(params, script->getLineNr())
  , script(script)
{}

unsigned long ScriptEvent::raise()
{
  ep_debug(" raise : " << name());
  next_step = 0;
  unsigned long resched = raise_();
  trim();
  if (eatWord("//", true)) line.clear();
  if (eatWord("#", true)) line.clear();
  if (line.length() and resched == 0) error("Garbage");
  if (not chain)
  {
    if (next_step > 0)
    {
      auto us = micros();
      if (us > next_step)
      {
        ep_debug("desync " << (us - next_step) << "us...");
        next_step = 0;
      }
      else
        next_step -= us;
    }

    chain = script->step(next_step);  // next_step is a delay here
  }
  return resched;
}

unsigned long ScriptEvent::raise_()
{
  std::string action = getWord();
  if (action=="")
  {
  }
  else if (action=="#" or action=="//")
  {
    line.clear();
  }
  else if (action == "at")
  {
    unsigned long resched = 0;
    getDuration(resched);
    return resched;
  }
  else if (action == "pin")
  {
    int pin = getPinNumber();
    int val = getPinNumber();
    ep_debug(micros() << " Setting pin " << pin << " to " << val);
    digitalReadValue(pin, val);
  }
  else if (action == "wait")
  {
    getDuration(next_step);
    next_step += micros();
  }
  else if (action == "sync")
  {
    getDuration(next_step);
  }
  else
  {
    chain = ScriptRegistry::build(action+' '+line, script, 0);
    // std::cout << "Chain " << line << " = " << chain.get() << std::endl;
    // FIXME, if build returns {} here, we may loop forever because we may return here each loop
    line = "";
  }
 return 0;
}

} // ns
