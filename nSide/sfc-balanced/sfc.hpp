#pragma once

//clone of higan's Super Famicom emulator's balanced profile.
//Credits for all components excpet for PPU and DSP are available in
//<sfc/sfc.hpp> in nSide's source.
// blargg            (Balanced/Performance DSP)
// hex_usr           (clean-up of Balanced PPU code)
//license: GPLv3
//original project started: 2004-10-14

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

  #include <sfc-balanced/memory/memory.hpp>
  #include <sfc-balanced/ppu/counter/counter.hpp>

  #include <sfc-balanced/cpu/cpu.hpp>
  #include <sfc-balanced/smp/smp.hpp>
  #include <sfc-balanced/dsp/dsp.hpp>
  #include <sfc-balanced/ppu/ppu.hpp>

  #include <sfc-balanced/controller/controller.hpp>
  #include <sfc-balanced/expansion/expansion.hpp>
  #include <sfc-balanced/system/system.hpp>
  #include <sfc-balanced/coprocessor/coprocessor.hpp>
  #include <sfc-balanced/slot/slot.hpp>
  #include <sfc-balanced/cartridge/cartridge.hpp>

  #include <sfc-balanced/memory/memory-inline.hpp>
  #include <sfc-balanced/ppu/counter/counter-inline.hpp>
}

#include <sfc-balanced/interface/interface.hpp>
