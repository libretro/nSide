#pragma once

namespace Emulator {

#if defined(DEBUGGER)
  #define debug(id, ...) if(debugger.id) debugger.id(__VA_ARGS__)
  #define debug_private public
#else
  #define debug(id, ...)
  #define debug_private private
#endif

}
