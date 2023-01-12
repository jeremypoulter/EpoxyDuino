#pragma once

#ifndef EPOXY_TEST
#error "You should include <epoxy_test/script> instead of including this file."
#endif

#include <epoxy_test/Injection/Event.h>
#include "Script.h"
#include "ScriptParser.h"

namespace EpoxyTest
{


class ScriptEvent : public EpoxyInjection::Event, ScriptParser
{
  public:
    ScriptEvent(unsigned long us, Script*);
    Script::EventPtr next() { return std::move(chain); }

    unsigned long raise() final;
    virtual const char* name() const override { return line.c_str(); }

    static Script::EventPtr build(Script *, unsigned long delay);

  protected:
    // Implement here the handling of the script event
    // return 0 to end the event, or delay in us to reschedule
    // a call to raise_ after this delay.
    virtual unsigned long raise_();

    // next_step is the time at which the next script command
    // will be scheduled.
    unsigned long next_step;

    // registering and clone, used for build()
    ScriptEvent(unsigned long us, Script*, std::string& params);
    ScriptEvent(const char* command);
    // parameters must not contain the command keyword itself
    virtual Script::EventPtr clone(unsigned long /* us */, Script*, std::string& /* parameters */) { return {}; }

  private:
    using Commands = std::map<std::string, ScriptEvent*>;
    Script* script;
    static std::unique_ptr<Commands> commands;

};

} // ns
