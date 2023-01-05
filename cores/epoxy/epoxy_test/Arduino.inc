// vim:syntax=cpp
#include <atomic>
#include <set>
#include <iostream>

extern std::atomic<unsigned long> epoxy_micros;
extern bool epoxy_real_time;

namespace EpoxyTest
{
std::set<Reset>* resets = nullptr;

void initTime()
{
  std::cout << "Not yet implemented : init time" << std::endl;
}

__attribute__((constructor))
void arduinoReset()
{
  registerReset(initTime);
}

void registerReset(Reset rst)
{
  if (resets == nullptr) resets = new std::set<Reset>;
  resets->insert(rst);
}

void reset()
{
  if (resets)
    for(auto rst: *resets)
      rst();
}

void set_real_time()
{
  epoxy_real_time = true;
}

void set_seconds(unsigned long s)
{
  epoxy_micros = s * 1000 * 1000;
  epoxy_real_time = false;
}

void add_seconds(unsigned long s)
{
  epoxy_micros += s * 1000 * 1000;
  epoxy_real_time = false;
}

void set_millis(unsigned long ms)
{
  epoxy_micros = 1000 * ms;
  epoxy_real_time = false;
}

void add_millis(unsigned long delta_ms)
{
  epoxy_micros += 1000 * delta_ms;
  epoxy_real_time = false;
}

void set_micros(unsigned long us)
{
  epoxy_micros = us;
  epoxy_real_time = false;
}

void add_micros(unsigned long delta_us)
{
  epoxy_micros += delta_us;
  epoxy_real_time = false;
}

}
