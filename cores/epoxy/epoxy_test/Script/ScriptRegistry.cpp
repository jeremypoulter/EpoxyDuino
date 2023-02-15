#include "ScriptRegistry.h"

namespace EpoxyTest
{
  std::unique_ptr<ScriptRegistry::Factory> ScriptRegistry::factory;

  ScriptRegistry::ScriptRegistry(const char* command, Builder builder)
  {
    if (factory == nullptr)
      factory = std::make_unique<Factory>();

    if (factory->find(command) != factory->end())
    {
      std::cerr << "*** ScriptEvent, registring more than once: " << command << std::endl;
    }
    ep_debug("=== Registering Script Command '" << command << "'");
    (*factory)[command] = builder;
  }

  Script::EventPtr ScriptRegistry::build(std::string line, Script* script, unsigned long delay)
  {
    unsigned long start = micros() + delay;
    if (factory)
    {
      std::string action = EpoxyTest::getWord(line);
      auto it = factory->find(action);
      if (it != factory->end())
      {
        return it->second(script, start, line);
      }
    }
    return {};
  }
}
