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
    error(std::string("EpoxyTest::Script unable to open file ") +filename);
    return;
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
      return ScriptEvent::build(this, delay);
    }
  }
  return nullptr;
}

}
