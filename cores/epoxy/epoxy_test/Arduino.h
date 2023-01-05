#include "Injection.h"

namespace EpoxyTest
{
  using Reset = void(*)();
  void registerReset(Reset);
  // Attempt to reset closest to a cold reset as possible
  void reset();

  // Simulated time
  void set_millis(unsigned long);
  void add_millis(unsigned long);
  void set_micros(unsigned long);
  void add_micros(unsigned long);
  void add_seconds(unsigned long);
  void set_seconds(unsigned long);
  void set_real_time();
}
