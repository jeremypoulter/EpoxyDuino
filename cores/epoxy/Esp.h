#pragma once

#include <sys/time.h>
#include <epoxy_test/injection>
#include "Stream.h"

class EspClass
{
  public:
    EspClass() {
      gettimeofday(&start, NULL);
    }

    void reset()
    {
      EpoxyTest::reset();
    };

    // Very ugly approximation, this is freeStack
    unsigned long getFreeHeap() {
      int i;
      static int* h=40000+&i;
      return h-&i;
    }

    uint32_t getCpuFreqMHZ() { return 80; }

    uint32_t getCycleCount() {
      struct timeval now;
      gettimeofday(&now, NULL);
      return getCpuFreqMHZ()
          * ((now.tv_sec-start.tv_sec)*1000000+(now.tv_usec-start.tv_usec));
    }

    void restart() { reset(); };

  private:
    struct timeval start;
};

class Serial_ : public Stream
{
};

extern EspClass ESP;
