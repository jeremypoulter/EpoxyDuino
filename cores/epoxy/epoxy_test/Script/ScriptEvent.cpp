#include "ScriptEvent.h"
#include "ScriptParser.h"

namespace EpoxyTest
{

std::unique_ptr<ScriptEvent::Commands> ScriptEvent::commands;

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

ScriptEvent::ScriptEvent(const char* command)
{
  if (commands == nullptr)
    commands = std::unique_ptr<Commands>(new Commands);

  if (commands->find(command) != commands->end())
  {
    std::cerr << "*** ScriptEvent, registring more than once: " << command << std::endl;
  }
  ep_debug("=== Registering Script Command '" << command << "'");
  (*commands)[command] = this;
}

Script::EventPtr ScriptEvent::build(Script *script, unsigned long delay)
{
  unsigned long start = micros() + delay;
  if (commands)
  {
    std::string line(script->getLine());
    auto it=commands->find(EpoxyTest::getWord(line));
    if (it != commands->end())
    {
      return it->second->clone(start, script, line);
    }
  }

  return Script::EventPtr(new ScriptEvent(start, script));
}

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
    // std::cout << micros() << " Setting pin " << pin << " to " << val << std::endl;
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
    error(std::string("Unknown script command: ") + action);
    line = "";
  }
 return 0;
}

} // ns
