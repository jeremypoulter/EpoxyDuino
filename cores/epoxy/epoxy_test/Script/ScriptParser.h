#pragma once

#ifndef EPOXY_TEST
#error "You should use <epoxy_test/script> instead of including this file."
#endif

#include <string>
#include <iostream>

namespace EpoxyTest
{

class ScriptParser
{
  public:
    ScriptParser(std::string line, int nr) : line(line), line_nr(nr) {}
    ScriptParser() {}

    std::string getLine() const { return line; }
    int getLineNr() const { return line_nr; }

  protected:
    std::string getWord();
    template<typename T> bool getNumber(T &t);
    int getPinNumber();
    bool getDuration(unsigned long &t);
    void trim();
    bool eat(const std::string &start, bool optional=false);
    void error(const std::string&);

  protected:
    std::string line;
    int line_nr=0;
};

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


}
