#pragma once

//based on higan's Super Famicom emulator by byuu,
//with contributions from:
// Andreas Naive     (S-DD1 decompression algorithm)
// anomie
// AWJ               (PPUcounter NTSC/PAL timing)
// _Demo_
// Derrick Sobodash
// DMV27
// Dr. Decapitator   (DSP-1,2,3,4, ST-010,011 ROM dumps)
// FirebrandX
// FitzRoy
// ilari             (Super Famicom CPU MDR tests)
// GIGO
// Jonas Quinn       (DSP fixes, Game Boy fixes)
// kode54
// krom
// Ladida            (HDMA timing discovery, mosaic hires fix)
// LostTemplar       (ST-018 program ROM analysis)
// Matthew Callis
// Nach
// neviksti          (SPC7110 decompression algorithm)
// Overload          (Cx4 data ROM dump)
// p4plus2           (Super Famicom CPU MDR tests)
// RedDwarf
// Richard Bannister
// segher            (Cx4 reverse engineering)
// Talarubi          (ST-018 discoveries and bug fixes)
// tetsuo55
// Total             (Super Famicom CPU MDR tests)
// TRAC
// zones
// hex_usr           (exportMemory PPU register/expansion chip support, Super Scope turbo cursor)
//license: GPLv3
//original project started: 2004-10-14

//nSide's accuracy profile is not for use as a libretro core. Use higan instead.

#include <emulator/emulator.hpp>
#include <emulator/thread.hpp>
#include <emulator/scheduler.hpp>
#include <emulator/random.hpp>
#include <emulator/cheat.hpp>

#include <processor/arm7tdmi/arm7tdmi.hpp>
#include <processor/gsu/gsu.hpp>
#include <processor/hg51b/hg51b.hpp>
#include <processor/spc700/spc700.hpp>
#include <processor/upd96050/upd96050.hpp>
#include <processor/wdc65816/wdc65816.hpp>

#if defined(SFC_SUPERGAMEBOY)
  #include <gb/gb.hpp>
#endif

namespace SuperFamicom {
  #define platform Emulator::platform
  namespace File = Emulator::File;
  using Scheduler = Emulator::Scheduler;
  using Random = Emulator::Random;
  using Cheat = Emulator::Cheat;
  extern Scheduler scheduler;
  extern Random random;
  extern Cheat cheat;

  struct Thread : Emulator::Thread {
    auto create(auto (*entrypoint)() -> void, double frequency) -> void {
      Emulator::Thread::create(entrypoint, frequency);
      scheduler.append(*this);
    }

    inline auto synchronize(Thread& thread) -> void {
      if(clock() >= thread.clock()) scheduler.resume(thread);
    }
  };

  struct Region {
    static inline auto NTSC() -> bool;
    static inline auto PAL() -> bool;
  };

  #include <sfc/memory/memory.hpp>
  #include <sfc/ppu/counter/counter.hpp>

  #include <sfc/cpu/cpu.hpp>
  #include <sfc/smp/smp.hpp>
  #include <sfc/dsp/dsp.hpp>
  #include <sfc/ppu/ppu.hpp>

  #include <sfc/controller/controller.hpp>
  #include <sfc/expansion/expansion.hpp>
  #include <sfc/system/system.hpp>
  #include <sfc/coprocessor/coprocessor.hpp>
  #include <sfc/slot/slot.hpp>
  #include <sfc/cartridge/cartridge.hpp>

  #include <sfc/memory/memory-inline.hpp>
  #include <sfc/ppu/counter/counter-inline.hpp>
}

#include <sfc/interface/interface.hpp>
