#include <md/md.hpp>

namespace MegaDrive {

Cartridge cartridge;
#include "serialization.cpp"

auto Cartridge::manifest() const -> string {
  string manifest = information.manifest;
  if(lockon && lockon->rom.size) manifest.append("\n", lockon->information.manifest);
  return manifest;
}

auto Cartridge::title() const -> string {
  string title = information.title;
  if(lockon && lockon->rom.size) title.append(" + ", lockon->information.title);
  return title;
}

auto Cartridge::load(bool lockedOn) -> bool {
  information = {};

  if(auto loaded = platform->load(ID::MegaDrive, "Mega Drive", "md", {"Auto", "NTSC-J", "NTSC-U", "PAL"})) {
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
      information.region = "NTSC-J";
    }
  }

  if(auto memory = Emulator::Game::Memory{document["game/board/memory(type=ROM,content=Program)"]}) {
    rom.size = memory.size >> 1;
    rom.mask = bit::round(rom.size) - 1;
    rom.data = new uint16[rom.mask + 1]();
    if(auto fp = platform->open(pathID(), memory.name(), File::Read, File::Required)) {
      for(uint n : range(rom.size)) rom.data[n] = fp->readm(2);
    }
  }

  //todo: handle mode, offset in Emulator::Game::Memory
  if(auto memory = document["game/board/memory(type=RAM,content=Save)"]) {
    if(auto mode = memory["mode"].text()) {
      if(mode == "lo"  ) ram.bits = 0x00ff;
      if(mode == "hi"  ) ram.bits = 0xff00;
      if(mode == "word") ram.bits = 0xffff;
    }
    ram.size = memory["size"].natural() >> (ram.bits == 0xffff);
    ram.mask = bit::round(ram.size) - 1;
    ram.data = new uint16[ram.mask + 1]();
    if(!(bool)memory["volatile"]) {
      if(auto fp = platform->open(pathID(), "save.ram", File::Read)) {
        for(uint n : range(ram.size)) {
          if(ram.bits != 0xffff) ram.data[n] = fp->readm(1) * 0x0101;
          if(ram.bits == 0xffff) ram.data[n] = fp->readm(2);
        }
      }
    }
  }

  if(document["game/board/lock-on"]) {
    lockon = new Cartridge;
    if(!lockedOn) lockon->load(true);
    if(auto memory = Emulator::Game::Memory{document["game/board/memory(type=ROM,content=UPMEM)"]}) {
      upmem.size = memory.size >> 1;
      upmem.mask = bit::round(upmem.size) - 1;
      upmem.data = new uint16[upmem.mask + 1]();
      if(auto fp = platform->open(pathID(), memory.name(), File::Read, File::Required)) {
        for(uint n : range(upmem.size)) upmem.data[n] = fp->readm(2);
      }
    }
  }

  Hash::SHA256 sha;
  for(uint index : range(rom.size)) {
    uint16 word = rom.data[index];
    sha.input((uint8_t)word.byte(1));
    sha.input((uint8_t)word.byte(0));
  }
  for(uint index : range(upmem.size)) {
    uint16 word = upmem.data[index];
    sha.input((uint8_t)word.byte(1));
    sha.input((uint8_t)word.byte(0));
  }
  information.sha256 = sha.digest();
  return true;
}

auto Cartridge::save() -> void {
  if(lockon) lockon->save();

  auto document = BML::unserialize(information.manifest);

  if(auto memory = Emulator::Game::Memory{document["game/board/memory(type=RAM,content=Save)"]}) {
    if(memory.nonVolatile) {
      if(auto fp = platform->open(pathID(), memory.name(), File::Write)) {
        for(uint n : range(ram.size)) {
          if(ram.bits != 0xffff) fp->writem(ram.data[n], 1);
          if(ram.bits == 0xffff) fp->writem(ram.data[n], 2);
        }
      }
    }
  }
}

auto Cartridge::unload() -> void {
  if(lockon) lockon->unload();

  delete[] rom.data;
  delete[] ram.data;
  rom = {};
  ram = {};
}

auto Cartridge::power() -> void {
  if(lockon) lockon->power();

  ramEnable = 1;
  ramWritable = 1;
  for(auto n : range(8)) bank[n] = n;
}

auto Cartridge::read(uint24 address) -> uint16 {
  if(lockon && address.bit(21)) {
    if(ramEnable && address.bit(20)) return upmem.data[address >> 1 & upmem.mask];
    return lockon->read(address);
  }

  uint romAddress = bank[address.bits(19,21)] << 19 | address.bits(0,18);
  uint16 data = rom.data[romAddress >> 1 & rom.mask];
  if(address.bit(21) && ram.size && ramEnable) {
    data &= ~ram.bits;
    data |= ram.data[address >> 1 & ram.mask] & ram.bits;
  }
  return data;
}

auto Cartridge::write(uint24 address, uint16 data) -> void {
  if(lockon && address.bit(21)) {
    if(ramEnable && address.bit(20)) return;
    return lockon->write(address, data);
  }

  //emulating RAM write protect bit breaks some commercial software
  if(address.bit(21) && ram.size && ramEnable /* && ramWritable */) {
    if(ram.bits == 0x00ff) data = data.byte(0) * 0x0101;
    if(ram.bits == 0xff00) data = data.byte(1) * 0x0101;
    ram.data[address >> 1 & ram.mask] = data;
  }
}

auto Cartridge::readIO(uint24 addr) -> uint16 {
  return 0x0000;
}

auto Cartridge::writeIO(uint24 addr, uint16 data) -> void {
  if(lockon) lockon->writeIO(addr, data);

  if(addr == 0xa130f1) ramEnable = data.bit(0), ramWritable = data.bit(1);
  if(addr == 0xa130f3) bank[1] = data;
  if(addr == 0xa130f5) bank[2] = data;
  if(addr == 0xa130f7) bank[3] = data;
  if(addr == 0xa130f9) bank[4] = data;
  if(addr == 0xa130fb) bank[5] = data;
  if(addr == 0xa130fd) bank[6] = data;
  if(addr == 0xa130ff) bank[7] = data;
}

}
