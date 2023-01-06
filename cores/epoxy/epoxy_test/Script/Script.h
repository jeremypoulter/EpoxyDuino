#pragma once

#ifndef EPOXY_TEST
#error "You must use -DEPOXY_TEST to include this file"
#endif

#include <epoxy_test/injection>

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
    bool getDuration(unsigned long &t);
    void trim();
    bool eat(const std::string &start, bool optional=false);
    void error(const std::string&);

  protected:
    std::string line;
    int line_nr=0;
};

class Script : public ScriptParser
{
  public:
    Script(const std::string filename);
    ~Script();

    using EventPtr = std::unique_ptr<EpoxyInjection::Event>;

  private:
    friend class ScriptEvent;
    EventPtr step(unsigned long delay);

  private:
    std::istream* input = nullptr;
};

class ScriptEvent : public EpoxyInjection::Event, ScriptParser
{
  public:
    ScriptEvent(unsigned long us, Script*);
    Script::EventPtr next() { return std::move(chain); }

    unsigned long raise() override;
    const char* name() const override { return line.c_str(); }

  private:
    Script* parent;
};


}
