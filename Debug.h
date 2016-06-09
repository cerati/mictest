#ifndef _debug_
#ifdef DEBUG
#define _debug_

#ifdef dprint
#undef dprint
#undef dcall
#undef dprintf
#endif
/*
  Usage: DEBUG must be defined before this header file is included, typically

  #define DEBUG
  #include "Debug.h"

  This defines macros dprint(), dcall() and dprintf();
  dprint(x) is equivalent to std::cout << x << std::endl;
    example: dprint("Hits in layer=" << ilayer);

  dcall(x) simply calls x
    example: dcall(pre_prop_print(ilay, mkfp));

  dprintf(x) is equivalent to printf(x)
    example: dprintf("Bad label for simtrack %d -- %d\n", itrack, track.label());

  All printouts are also controlled by a bool variable "debug"
  bool debug = true; is declared as a file global in an anonymous
  namespace, and thus can be overridden within any interior scope
  as needed, so one could change the global to false and only set
  a local to true within certain scopes.

  All are protected by a file scope mutex to avoid mixed printouts.
  This mutex can also be acquired within a block via dmutex_guard:

  if (debug) {
    dmutex_guard;
    [do complicated stuff]
  }

  The mutex is not reentrant, so avoid using dprint et al. within a scope
  where the mutex has already been acquired, as doing so will deadlock.
 */
#include <mutex>

#define dmutex_guard std::lock_guard<std::mutex> dlock(debug_mutex)
#define dprint(x) if (debug) { dmutex_guard; std::cout << x << std::endl; }
#define dprint_np(n,x) if(debug && n < N_proc) { dmutex_guard; std::cout << n << ": " << x << std::endl; }
#define dcall(x)  if (debug) { dmutex_guard; x; }
#define dprintf(...) if (debug) { dmutex_guard; printf(__VA_ARGS__); }
#define dprintf_np(n, ...) if (debug && n< N_proc) { dmutex_guard; std::cout << n << ": "; printf(__VA_ARGS__); }

namespace { 
  bool debug = true; // default, can be overridden locally
  std::mutex debug_mutex;
}

static void print(const TrackState& s)
{
  std::cout << "x:  "  << s.parameters[0] 
            << " y:  " << s.parameters[1]
            << " z:  " << s.parameters[2] << std::endl
            << "px: "  << s.parameters[3]
            << " py: " << s.parameters[4]
            << " pz: " << s.parameters[5] << std::endl
            << "valid: " << s.valid << " errors: " << std::endl;
  dumpMatrix(s.errors);
  std::cout << std::endl;
}

static void print(std::string label, int itrack, const Track& trk)
{
  std::cout << std::endl << label << ": " << itrack << " hits: " << trk.nFoundHits() << " State" << std::endl;
  print(trk.state());
}

static void print(std::string label, const TrackState& s)
{
  std::cout << label << std::endl;
  print(s);
}

static void print(std::string label, const MeasurementState& s)
{
  std::cout << label << std::endl;
  std::cout << "x: "  << s.parameters()[0] 
            << " y: " << s.parameters()[1]
            << " z: " << s.parameters()[2] << std::endl
            << "errors: " << std::endl;
  dumpMatrix(s.errors());
  std::cout << std::endl;
}

#else
#define dprint(x) (void(0))
#define dprint_np(n,x) (void(0))
#define dcall(x) (void(0))
#define dprintf(...) (void(0))
#define dprintf_np(n,...) (void(0))
#endif
#endif
