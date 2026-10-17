// Host emulation of the ESP32 Arduino core's loop watchdog.
//
// Like the real thing, a monitor thread watches a shared timestamp that the
// loop task refreshes (epoxyduino_main() calls feedLoopWDT() after each
// loop(), as the core's loopTask does). If the timestamp is older than the
// timeout while the watchdog is armed, the loop task is considered frozen: its
// stack is dumped and the process is terminated.
//
// The stack is dumped by signalling the frozen thread, so backtrace() runs on
// that thread's own stack. Only available with glibc/macOS (execinfo.h).
//
// Timeout: 5 s (the ESP32 default), overridden by EPOXY_LOOP_WDT_MS (0 = never
// expire) or loopWDTSetTimeout().
#if defined(EPOXY_CORE_ESP32)

#include "esp32_compat.h"
#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#if defined(__GLIBC__) || defined(__APPLE__)
#include <execinfo.h>
#define EPOXY_HAVE_BACKTRACE 1
#endif

namespace {

const unsigned kDefaultTimeoutMs = 5000;
const unsigned kPollMs = 20; // expiry is detected within this much of timeout

// The ESP32 core leaves the loop WDT off until the sketch calls enableLoopWDT().
std::atomic<bool> armed(false);
std::atomic<long long> lastFeedMs(0);
std::atomic<unsigned> timeoutMs(kDefaultTimeoutMs);
std::atomic<bool> timeoutExplicit(false);
std::atomic<void (*)()> expiryHandler(nullptr);
pthread_t loopThread; // written before `armed` is set, read after it is seen
std::once_flag monitorStarted;

long long nowMs()
{
  using namespace std::chrono;
  return duration_cast<milliseconds>(steady_clock::now().time_since_epoch())
      .count();
}

// Runs on the frozen loop thread.
void dumpStackAndAbort(int)
{
#if defined(EPOXY_HAVE_BACKTRACE)
  void *frames[64];
  int n = backtrace(frames, 64);
  static const char hdr[] = "Loop task stack at watchdog expiry:\n";
  ssize_t r = write(STDERR_FILENO, hdr, sizeof(hdr) - 1);
  (void)r;
  backtrace_symbols_fd(frames, n, STDERR_FILENO);
#endif
  signal(SIGABRT, SIG_DFL);
  abort();
}

void terminateLoopTask()
{
  fprintf(stderr, "Loop watchdog: loop task not fed for %u ms, terminating\n",
          timeoutMs.load());
  fflush(stderr);

  struct sigaction sa = {};
  sa.sa_handler = dumpStackAndAbort;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGUSR2, &sa, nullptr);
  pthread_kill(loopThread, SIGUSR2);

  sleep(1); // the loop task is stuck somewhere that ignores signals
  abort();
}

void monitor()
{
  for (;;) {
    std::this_thread::sleep_for(std::chrono::milliseconds(kPollMs));
    unsigned timeout = timeoutMs.load();
    if (!armed.load() || timeout == 0 || nowMs() - lastFeedMs.load() <= timeout) {
      continue;
    }
    if (armed.exchange(false)) { // fire once; the handler decides what's next
      void (*handler)() = expiryHandler.load();
      (handler ? handler : terminateLoopTask)();
    }
  }
}

} // namespace

void enableLoopWDT()
{
  std::call_once(monitorStarted, [] {
    const char *env = getenv("EPOXY_LOOP_WDT_MS");
    if (env && !timeoutExplicit.load()) {
      timeoutMs = (unsigned)strtoul(env, nullptr, 10);
    }
#if defined(EPOXY_HAVE_BACKTRACE)
    void *warm[1];
    backtrace(warm, 1); // load libgcc now, not inside the signal handler
#endif
    std::thread(monitor).detach();
  });
  loopThread = pthread_self();
  lastFeedMs = nowMs();
  armed = true;
}

void disableLoopWDT() { armed = false; }

void feedLoopWDT() { lastFeedMs = nowMs(); }

bool loopWDTArmed() { return armed.load(); }

void loopWDTSetTimeout(unsigned ms)
{
  timeoutMs = ms;
  timeoutExplicit = true;
}

void loopWDTSetExpiryHandler(void (*handler)()) { expiryHandler = handler; }

#endif // EPOXY_CORE_ESP32
