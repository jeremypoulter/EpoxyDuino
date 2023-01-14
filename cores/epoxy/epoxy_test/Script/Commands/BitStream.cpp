#include "BitStream.h"

namespace EpoxyTest
{

long BitStream::out_of_sync = 0;

BitStream BitStream::registry("bitstream");

unsigned long BitStream::raise_()
{
  if (pin == -1) return 0;

  unsigned long curr_us = micros();
  next_us += period_us;
  char bit = getChar();
  if (bit == '-' or bit==' ') bit = getChar();  // separators allowed
  if (bit == 0) return 0;

  ep_debug("BITSTREAM char(" << bit << ")");
  if (bit == '0')
  {
    digitalReadValue(pin, 0);
  }
  else if (bit == '1')
  {
    digitalReadValue(pin, 1);
  }
  else
  {
    error(std::string("Unexpected bit in bitstream (") + std::to_string((int)bit) + "), allowed are: 0 1 - spc."); 
    return 0;
  }

  if (next_us > curr_us)
  {
    return next_us;
  }
  else
  {
    if (curr_us > next_out_of_sync_display)
    {
      std::cerr << micros() << " *** BITSTREAM OUT OF SYNC #" << out_of_sync << ", curr_us=" << curr_us << ", next_us=" << next_us << ", period=" << period_us << ", delta=" << curr_us-next_us << std::endl << std::flush;
      next_out_of_sync_display += 1000000;
    }
    out_of_sync++;
    return curr_us;
  }
}

BitStream::BitStream(unsigned long us, Script* script, std::string& params)
  : ScriptEvent(us, script, params)
{
  getDuration(period_us);
  pin = getPinNumber();
  next_us = micros();
}

Script::EventPtr BitStream::clone(unsigned long us, Script* script, std::string& params)
{
  return Script::EventPtr(new BitStream(us, script, params));
}

}
