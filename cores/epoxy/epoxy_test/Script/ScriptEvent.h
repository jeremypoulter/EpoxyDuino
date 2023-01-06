#pragma once

#ifndef EPOXY_TEST
#error "You should use <epoxy_test/script> instead of including this file."
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

    unsigned long raise() override;
    const char* name() const override { return line.c_str(); }

  private:
    Script* parent;
};

} // ns
