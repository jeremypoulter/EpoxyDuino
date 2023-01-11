#pragma once

#ifndef EPOXY_TEST
#error "You should use <epoxy_test/script> instead of including this file."
#endif

#include <string>

namespace EpoxyTest
{

class ScriptParser
{
  public:
    ScriptParser(std::string line, int nr, bool display_errors=true)
    : line(line), line_nr(nr), displayErrors(display_errors) {}
    ScriptParser() {}

    std::string getLine() const { return line; }
    int getLineNr() const { return line_nr; }

    std::string getWord();
    template<typename T> bool getNumber(T &t);
    int getPinNumber();
    bool getDuration(unsigned long &t);
    void trim();
    bool eatWord(const std::string &start, bool optional=false);
    void error(const std::string&);

    // Convert 1ms to 1000, 1MHz to 1 etc...
    int getPeriodUs();

    // Return the ratio for conversion to microseconds or 0 (no unit)
    int ratioUs();
    // Return the period in microseconds of the unit or 0 (no unit)
    int periodUs();

    int errorsCount() const { return errors; }

  protected:
    std::string line;
    int line_nr=0;
    int errors=0;
    bool displayErrors;
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
