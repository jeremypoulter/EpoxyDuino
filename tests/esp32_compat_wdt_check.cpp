// Self-check for the host loop watchdog (cores/epoxy/esp32_wdt.cpp).
// Build and run:
//   g++ -std=c++11 -pthread -DEPOXY_CORE_ESP32 -I cores/epoxy tests/esp32_compat_wdt_check.cpp cores/epoxy/esp32_wdt.cpp -o /tmp/wdt && /tmp/wdt
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <atomic>
#include <chrono>
#include <thread>
#include "esp32_compat.h"

static std::atomic<int> expired(0);
static void onExpire() { expired++; }

static void sleepMs(int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

// Never returns when the watchdog terminates us.
__attribute__((noinline)) static void frozenLoopTask() { for (;;) { asm volatile(""); } }

// Must run before the monitor thread exists in this process (it isn't forked).
static void checkTerminatesAndDumpsStack()
{
  int fds[2];
  assert(pipe(fds) == 0);
  pid_t pid = fork();
  assert(pid >= 0);
  if (pid == 0) {
    dup2(fds[1], STDERR_FILENO);
    loopWDTSetTimeout(100);
    enableLoopWDT();
    frozenLoopTask();
  }
  close(fds[1]);
  char buf[4096] = {0};
  ssize_t n = 0, r;
  while ((r = read(fds[0], buf + n, sizeof(buf) - 1 - n)) > 0) n += r;
  int status = 0;
  waitpid(pid, &status, 0);
  assert(WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT);
  assert(strstr(buf, "Loop watchdog"));
#if defined(__GLIBC__) || defined(__APPLE__)
  assert(strstr(buf, "Loop task stack at watchdog expiry"));
  assert(strstr(buf, "frozenLoopTask") || strstr(buf, "wdt"));  // a frame of ours
#endif
}

int main()
{
  checkTerminatesAndDumpsStack();

  assert(!loopWDTArmed());       // off until the sketch enables it, like the ESP32 core
  loopWDTSetExpiryHandler(onExpire);
  loopWDTSetTimeout(200);

  enableLoopWDT();
  assert(loopWDTArmed());
  for (int i = 0; i < 25; i++) { feedLoopWDT(); sleepMs(20); }
  assert(expired == 0);          // fed in time: no expiry

  sleepMs(400);
  assert(expired == 1);          // starved: expired exactly once
  assert(!loopWDTArmed());

  enableLoopWDT();               // OTA handshake: suspend, then resume
  disableLoopWDT();
  sleepMs(400);
  assert(expired == 1);          // disarmed: no expiry however long
  enableLoopWDT();
  sleepMs(100);
  assert(expired == 1);          // re-arming restarts the timer
  disableLoopWDT();
  return 0;
}
