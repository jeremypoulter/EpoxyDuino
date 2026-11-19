// Lets code written for embedded platforms include <machine/endian.h>.
// On Apple the SDK ships its own <machine/endian.h> (included by system
// headers such as <sys/wait.h>) and there is no top-level <endian.h>, so
// forward to the SDK header instead of shadowing it.
#if defined(__APPLE__)
#include_next <machine/endian.h>
#else
#include <endian.h>
#endif
