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

std::string getWord(std::string& line)
{
  std::string word;
  while(line[0] && line[0] != ' ')
  {
    word += line[0];
    line.erase(0,1);
  }
  trim(line);
  return word;
}

bool startsWith(std::string& line, const char* start)
{
  const char* p=line.c_str();
  while(*p == *start)
  {
    if (*p==0 or *start==0) break;
    p++; start++;
  }
  if (*start!=0) return false;
  return *p==' ' or *p=='\t' or *p==0;
}

void trim(std::string& line)
{
  while(line[0] == ' ' or line[0]=='\t') line.erase(0,1);
}

char ScriptParser::getChar()
{
  char c = 0;
  if (line.length())
  {
    c = line[0];
    line.erase(0,1);
  }
  return c;
}

bool ScriptParser::eatWord(const std::string &start, bool optional)
{
  if (line == start)
  {
    line = "";
    return true;
  }
  if (line.substr(0, start.length()+1) == start+' ')
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

int ScriptParser::getPeriodUs()
{
  if (not isdigit(line[0]))
  {
    error("Expected number");
    return 0;
  }

  int val;
  getNumber(val);

  int period = periodUs();
  if (period == 0) period = ratioUs();

  if (period == 0)
  {
    error("Expected delay or frequency");
    return 0;
  }

  return val * period;
}

int ScriptParser::periodUs()
{
  if (eatWord("Mhz", true)) return 1;
  else if (eatWord("kHz", true)) return 1000;
  else if (eatWord("Hz", true)) return 1000000;
  return 0;
}

int ScriptParser::ratioUs()
{
  if (eatWord("us", true)) return 1;
  else if (eatWord("ms", true)) return 1000;
  else if (eatWord("s", true)) return 1000000;
  else if (eatWord("mn", true)) return 1000000 * 60;
  return 0;
}

// Parse a line of script
bool ScriptParser::getDuration(unsigned long &t)
{
  t = 0;
  if (isdigit(line[0]))
  {
    if (not getNumber(t))
      return false;
    int ratio = ratioUs();
    if (ratio)
    {
      t *= ratio;
      return true;
    }
  }
  error("time unit expected (us, ms, s, mn");
  return false;
}

void ScriptParser::error(const std::string& s)
{
  if (displayErrors)
  {
    std::cerr << "***" << std::endl;
    std::cerr << "*** Script error at line #" << line_nr << ", (" << line << ") : " << s << std::endl;
    std::cerr << "***" << std::endl;
  }
  errors++;
}

}

#endif // EPOXY_TEST
