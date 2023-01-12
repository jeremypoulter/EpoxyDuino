#pragma once
#include "../ScriptEvent.h"

namespace EpoxyTest
{

class BitStream : public ScriptEvent
{
  public:
    static long outOfSyncCount() { return out_of_sync; } 
  protected:
    BitStream(const char* command) : ScriptEvent(command) {};

    virtual unsigned long raise_();
    virtual Script::EventPtr clone(unsigned long /* us */, Script*, std::string& /* parameters */);

  private:

    BitStream(unsigned long us, Script*, std::string& params);

    static BitStream registry;

    int pin;

    unsigned long period_us;
    unsigned long next_us = 0;
    unsigned long next_out_of_sync_display = 0;
    static long out_of_sync;
};

}
