// snes_spc 0.9.0 user configuration file. Don't replace when updating library.

// snes_spc 0.9.0
#ifndef BLARGG_CONFIG_H
#define BLARGG_CONFIG_H

// Debugging checks are enabled and disabled based on build optimization level (nall/intrinsics.hpp).

// Uncomment to enable platform-specific (and possibly non-portable) optimizations
//#define BLARGG_NONPORTABLE 1

// Endianness is checked within nall/intrinsics.hpp.

// Uncomment if you get errors in the bool section of blargg_common.h
//#define BLARGG_COMPILER_HAS_BOOL 1

// Use standard config.h if present
#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#endif
