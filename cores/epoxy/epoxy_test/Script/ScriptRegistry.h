#pragma once

#include <functional>
#include <string>
#include <memory>

#include "Script.h"

// factory of script commands

namespace EpoxyTest
{

class ScriptRegistry
{
  public:
    using Builder = std::function<Script::EventPtr(Script*, unsigned long /* us */, std::string& /* params */)>;
    using Factory = std::map<std::string, Builder>;

    ScriptRegistry(const char* command, Builder);
    static Script::EventPtr build(std::string line, Script* script, unsigned long delay);

  private:
    static std::unique_ptr<Factory> factory;
};

}
