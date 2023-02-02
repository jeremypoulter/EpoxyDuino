#pragma once

#include <epoxy_test/injection>

#include "ScriptParser.h"
#include <string>
#include <iostream>

namespace EpoxyTest
{

class Script : public ScriptParser
{
  public:
    // url is a pseudo url of the form
    // type:data
    // default type is file
    // Supported types
    //   file:{filename}
    //   script:script command
    Script(std::string url);
    Script(std::istream* in);
    virtual ~Script();

    using EventPtr = std::unique_ptr<EpoxyInjection::Event>;

  private:
    void init(std::istream*);
    friend class ScriptEvent;
    EventPtr step(unsigned long delay);

  private:
    std::string   input_name;
    std::istream* input;
    bool          delete_input; /// Oouch !!!!! FIXME
};

}
