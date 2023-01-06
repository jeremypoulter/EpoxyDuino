#ifdef EPOXY_TEST

#include "Script.h"
#include <Arduino.h>
#include <fstream>
#include <iostream>
#include <map>
#include <ctype.h>

namespace EpoxyTest
{

Script::Script(const std::string filename)
{
  input = new std::ifstream(filename);
  if (not input->good())
  {
    std::cerr << "EpoxyTest::Script unable to open file " << filename << std::endl;
  }
  auto evt = step(0);
  if (evt)
      EpoxyInjection::Injector::addEvent(std::move(evt));
}

Script::~Script()
{
  delete input;
}

Script::EventPtr Script::step(unsigned long delay)
{
  if (input->good())
  {
    if (std::getline(*input, line))
    {
      line_nr++;
      return Script::EventPtr(new ScriptEvent(micros()+delay, this));
    }
  }
  return nullptr;
}

std::string ScriptParser::getWord()
{
  std::string word;
  while(line[0] && line[0] != ' ')
  {
    word += line[0];
    line.erase(0,1);
  }
  trim();
  return word;
}

void ScriptParser::trim()
{
  while(line[0] == ' ') line.erase(0,1);
}

bool ScriptParser::eat(const std::string &start, bool optional)
{
  if (line.substr(0, start.length()) == start)
  {
    line.erase(0, start.length());
    trim();
    return true;
  }
  if (optional) return false;
  error(std::string("expected: ")+start);
  return false;
}

template<typename T>
bool ScriptParser::getNumber(T &t)
{
  if (not isdigit(line[0]))
  {
    error("Expected number");
    return false;
  }
  t = 0;
  while(isdigit(line[0]))
  {
    t = t*10+line[0]-'0';
    line.erase(0,1);
  }
  trim();
  return true;
}


// Parse a line of script
bool ScriptParser::getDuration(unsigned long &t)
{
  t = 0;
  if (isdigit(line[0]))
  {
    if (not getNumber(t))
      return false;
    if (eat("us", true)) { return true; }
    else if (eat("ms", true)) { t *= 1000; return true; }
    else if (eat("s", true)) { t *= 1000000; return true; }
    else if (eat("mn", true)) { t *= 1000000 * 60; return true; }
  }
  error("time unit expected (us, ms, s, mn");
  return false;
}

void ScriptParser::error(const std::string& s)
{
  std::cout << "*** Script error at line #" << line_nr << ", (" << line << ") : " << s << std::endl;
}

ScriptEvent::ScriptEvent(unsigned long us, Script* script)
    : Event(us)
    , ScriptParser(script->getLine(), script->getLineNr())
    , parent(script)
{
}

unsigned long ScriptEvent::raise()
{
  std::cout << micros() << " raise " << line << std::endl << std::flush;
  unsigned long next = 0;
  std::string action = getWord();
  if (action == "pin")
  {
    uint8_t pin, val;
    getNumber(pin);
    getNumber(val);
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
      return (t);  // rechedule
    }
  }
  else
  {
    error(std::string("Unknown script command: ")+line);
  }
  trim();
  if (eat("//", true)) line.clear();
  if (eat("#", true)) line.clear();
  if (line.length())
    error("Garbage");
  chain = parent->step(next);
  return 0;
}

}

#endif
