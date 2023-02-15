#pragma once

#include <epoxy_test/Injection/Event.h>
#include "ScriptRegistry.h"
#include "Script.h"
#include "ScriptParser.h"

namespace EpoxyTest
{


class ScriptEvent : public EpoxyInjection::Event, public ScriptParser
{
  public:
    ScriptEvent(unsigned long us, Script*);
    Script::EventPtr next() { return std::move(chain); }

    unsigned long raise() final;
    virtual const char* name() const override { return line.c_str(); }

    // static Script::EventPtr build(Script *, unsigned long delay);

  protected:
    // Implement here the handling of the script event
    // return 0 to end the event, or delay in us to reschedule
    // a call to raise_ after this delay.
    virtual unsigned long raise_();

    // next_step is the time at which the next script command
    // will be scheduled.
    unsigned long next_step;

    ScriptEvent(unsigned long us, Script*, std::string& params);

  private:
    Script* script;

};

} // ns
