#ifdef EPOXY_TEST

#include "Script.h"
#include "ScriptEvent.h"

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

}

#endif
