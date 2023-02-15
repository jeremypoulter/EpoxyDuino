#pragma once
#include "../ScriptEvent.h"
#include "../ScriptRegistry.h"

namespace EpoxyTest
{

class BitStream : public ScriptEvent
{
  public:
    BitStream(Script*, unsigned long us, std::string& params);
    static long outOfSyncCount() { return out_of_sync; } 

  protected:
    virtual unsigned long raise_();

  private:

    int pin;

    unsigned long period_us;
    unsigned long next_us = 0;
    unsigned long next_out_of_sync_display = 0;
    static long out_of_sync;

    static ScriptRegistry registry;
};

}
