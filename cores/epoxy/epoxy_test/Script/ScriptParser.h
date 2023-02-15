#pragma once

#include <string>

namespace EpoxyTest
{

std::string getWord(std::string& line);
void trim(std::string& line);
bool startsWith(std::string& line, const char* start);

/**
  * This class allows to parse text file, and get argument values
  * with units or map replacement.
  *
  * All functions are eating some characters from the line member of the instance.
  */
class ScriptParser
{
  public:
    ScriptParser(std::string line, int nr) : line(line), line_nr(nr) {}
    ScriptParser() = default;
    virtual ~ScriptParser() = default;

    std::string getLine() const { return line; }
    int getLineNr() const { return line_nr; }

    std::string getWord() { return EpoxyTest::getWord(line); }
    char getChar();
    template<typename T> bool getNumber(T &t);
    /**
      * Resolve the symbolic pin with pins_arduino.h if value is text
      */
    int getPinNumber();   // -1 if bad pin
    /**
      * Get a duration is microseconds. Time conversion is done.
      * ex: 1ms will return 1000. The time unit is expected.
      */
    bool getDuration(unsigned long &t);
    void trim() { EpoxyTest::trim(line); }
    bool eatWord(const std::string &start, bool optional=false);
    void error(const std::string&);

    /** Try to convert something to a number of microseconds.
      * This function uses ratioUs() and periodUs() in order
      * to get the right value.
      * Duration: 1ms will return 1000 etc.
      * Frequency: 100Hz will return 10000
      * @return 0 if no conversion is possible
      */
    int getPeriodUs();

    /** Gives the number of us in the duration unit
      * 1ms will return 1000 etc.
      * @return 0 if no unit
      */
    int ratioUs();

    /** Gives the period of frequency unit.
      * 100Hz will give 10000, it is planned to add bauds
      * @return 0 if no unit
      */
    int periodUs();

    /**
      * @return Total number of errors encountered */
    int errorsCount() const { return errors; }

  protected:
    std::string line;
    int line_nr=0;
    int errors=0;
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
