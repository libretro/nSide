#pragma once

//based on higan's Famicom emulator by byuu and Ryphecha,
//with contributions from:
// AWJ     (MMC5 CHR banking)
// Bisqwit (PPU color generation for NTSC systems)
// FitzRoy (Bug-testing)
// kevtris (PPU voltage levels in Bisqwit's color formula)
// hex_usr (Many code organization changes, PPU timing fixes, mapper implementations, controllers, etc.)
//No longer used:
// loopy   (MMC5 CHR banking)
//license: GPLv3
//original project started: 2011-09-05

#include <emulator/emulator.hpp>
#include <emulator/thread.hpp>
#include <emulator/scheduler.hpp>
#include <emulator/random.hpp>
#include <emulator/cheat.hpp>

#include <processor/mos6502/mos6502.hpp>
#include <processor/z80/z80.hpp>

namespace Famicom {
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

  struct Model {
    inline static auto Famicom() -> bool;
    inline static auto VSSystem() -> bool;
    inline static auto PlayChoice10() -> bool;
    inline static auto FamicomBox() -> bool;
  };

  struct Region {
    static inline auto NTSCJ() -> bool;
    static inline auto NTSCU() -> bool;
    static inline auto PAL() -> bool;
    static inline auto Dendy() -> bool;
  };

  #include <fc/controller/controller.hpp>
  #include <fc/expansion/expansion.hpp>

  #include <fc/memory/memory.hpp>
  #include <fc/ppu/counter/counter.hpp>

  #include <fc/cpu/cpu.hpp>
  #include <fc/apu/apu.hpp>
  #include <fc/ppu/ppu.hpp>

  #include <fc/system/system.hpp>
  #include <fc/cartridge/cartridge.hpp>
  #include <fc/arcade/arcade.hpp>

  #include <fc/memory/memory-inline.hpp>
  #include <fc/ppu/counter/counter-inline.hpp>
}

#include <fc/interface/interface.hpp>
