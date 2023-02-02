#include "Script.h"
#include "ScriptEvent.h"

#include <Arduino.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <ctype.h>

namespace EpoxyTest
{

Script::Script(std::string url)
{
  input_name = url;
  size_t dp = url.find(':');
  std::string type="file";
  if (dp != std::string::npos)
  {
    type=url.substr(0, dp);
    url.erase(0, dp+1);
  }

  delete_input = true;  // FIXME What a bad design !
  if (type == "file")
  {
    std::ifstream* in = new std::ifstream(url);
    init(in);
  }
  else if (type == "script")
  {
    std::stringstream* in = new std::stringstream;
    *in << url;
    init(in);
  }
  else
  {
    delete_input = false; // FIXME
  }
}

Script::~Script()
{
  if (delete_input)
    delete input;
}
 
void Script::init(std::istream* in)
{
  input = in;
  if (not input) return;
  if (not input->good())
  {
    error(std::string("EpoxyTest::Script unable to open file ") + input_name);
    return;
  }
  auto evt = step(0);
  if (evt)
      EpoxyInjection::Injector::addEvent(std::move(evt));
}

Script::EventPtr Script::step(unsigned long delay)
{
  if (not input) return {};
  if (input->good())
  {
    if (std::getline(*input, line))
    {
      line_nr++;
      return ScriptEvent::build(this, delay);
    }
  }
  return {};
}

}
