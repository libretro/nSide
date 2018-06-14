#include <a2600/a2600.hpp>

namespace Atari2600 {

#include "serialization.cpp"
Cartridge cartridge;

auto Cartridge::load() -> bool {
  information = Information();

  if(auto loaded = platform->load(ID::Atari2600, "Atari 2600", "a26", {"Auto", "NTSC", "PAL", "SECAM"})) {
    information.pathID = loaded.pathID();
    information.region = loaded.option();
  } else return false;

  if(auto fp = platform->open(pathID(), "manifest.bml", File::Read, File::Required)) {
    information.manifest = fp->reads();
  } else return false;

  auto document = BML::unserialize(information.manifest);
  information.title = document["game/label"].text();

  if(information.region == "Auto") {
    if(auto region = document["game/region"].text()) {
      information.region = region.upcase();
    } else {
      information.region = "NTSC";
    }
  }

  if(auto memory = Emulator::Game::Memory{document["game/board/memory(type=ROM,content=Program)"]}) {
    rom.size = memory.size;
    rom.mask = bit::round(rom.size) - 1;
    if(rom.size) {
      rom.data = new uint8[rom.mask];
      if(auto fp = platform->open(pathID(), memory.name(), File::Read, File::Required)) {
        fp->read(rom.data, rom.size);
      }
    }
  }

  if(auto memory = Emulator::Game::Memory{document["game/board/memory(type=RAM,content=Save)"]}) {
    ram.size = memory.size;
    ram.mask = bit::round(ram.size) - 1;
    if(ram.size) {
      ram.data = new uint8[ram.mask];
      if(memory.nonVolatile) {
        if(auto fp = platform->open(pathID(), memory.name(), File::Read)) {
          fp->read(ram.data, ram.size);
        }
      }
    }
  }

  information.sha256 = Hash::SHA256(rom.data, rom.size).digest();
  return true;
}

auto Cartridge::save() -> void {
  auto document = BML::unserialize(information.manifest);

  if(auto memory = Emulator::Game::Memory{document["game/board/memory(type=RAM,content=Save)"]}) {
    if(memory.nonVolatile) {
      if(auto fp = platform->open(pathID(), memory.name(), File::Write)) {
        fp->write(ram.data, ram.size);
      }
    }
  }
}

auto Cartridge::unload() -> void {
  delete[] rom.data;
  delete[] ram.data;
  rom = Memory();
  ram = Memory();
}

auto Cartridge::power() -> void {
}

auto Cartridge::reset() -> void {
}

auto Cartridge::access(uint13 addr, uint8 data) -> uint8 {
  if(!addr.bit(12)) return data;

  if(ram.size) {
         if((addr & rom.mask) < ram.size << 0) return ram.data[addr & ram.mask] = data;
    else if((addr & rom.mask) < ram.size << 1) return ram.data[addr & ram.mask];
  }

  return rom.data[addr & rom.mask];
}

}
