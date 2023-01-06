#ifdef EPOXY_TEST

#include <Arduino.h>
#include <fstream>
#include <iostream>
#include <map>
#include <ctype.h>

#include "Script.h"
#include "ScriptEvent.h"

namespace EpoxyTest
{

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

int ScriptParser::getPinNumber()
{
	if (isdigit(line[0]))
	{
		int pin;
		if (getNumber(pin))
			return pin;
		else
			return -1;
	}

	std::string pin = getWord();
	auto it = pins_arduino.find(pin);
	if (it != pins_arduino.end())
		return it->second;
	
	error(std::string("Not a valid pin name: '"+pin+'\''));
	return -1;
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

}

#endif // EPOXY_TEST
