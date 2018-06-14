#include <fc/fc.hpp>

namespace Famicom {

VSSystem vssystem;

#include "control.cpp"
#include "serialization.cpp"

VSSystem::VSSystem() {
  forceSubRAM = false;
  gameCount = GameCount::Uni;
}

auto VSSystem::Enter() -> void {
  while(true) scheduler.synchronize(), vssystem.main();
}

auto VSSystem::main() -> void {
  //if(++watchdog > system.cpuFrequency() * 4 / 3) {
  //  watchdog = 0;
  //  if(gameCount == GameCount::Dual) cpuM.reset();
  //  cpuS.reset();
  //}
  step(1);
  if(gameCount == GameCount::Dual) synchronize(cpuM);
  synchronize(cpuS);
}

auto VSSystem::load() -> bool {
  return true;
}

auto VSSystem::power(bool reset) -> void {
  create(VSSystem::Enter, system.frequency());

  if(!reset) {
    function<auto (uint16, uint8) -> uint8> reader;
    function<auto (uint16, uint8) -> void> writer;

    reader = [](uint16 addr, uint8 data) -> uint8 { return vssystem.read(0, addr, data); };
    writer = [](uint16 addr, uint8 data) -> void { vssystem.write(0, addr, data); };
    busM.map(reader, writer, "4016-4017");
    busM.map(reader, writer, "4020-5fff", 0, 0, 0x0020);
    busM.map(reader, writer, "6000-7fff");

    reader = [](uint16 addr, uint8 data) -> uint8 { return vssystem.read(1, addr, data); };
    writer = [](uint16 addr, uint8 data) -> void { vssystem.write(1, addr, data); };
    busS.map(reader, writer, "4016-4017");
    busS.map(reader, writer, "4020-5fff", 0, 0, 0x0020);
    busS.map(reader, writer, "6000-7fff");

    random.array(ram, sizeof(ram));
  }

  ramSide = forceSubRAM ? 1 : 0;
  resetButtons();
}

auto VSSystem::setDip(bool side, uint8 dip) -> void {
  if(side == 0) dipM = dip;
  if(side == 1) dipS = dip;
}

auto VSSystem::read(bool side, uint16 addr, uint8 data) -> uint8 {
  if(addr == 0x4016) {
    data.bit (  0) = data1(side);
    data.bit (  1) = 0;
    data.bit (  2) = poll(side, ServiceButton);
    data.bits(3,4) = (side ? dipS : dipM).bits(0,1);
    data.bit (  5) = poll(side, Coin1);
    data.bit (  6) = poll(side, Coin2);
    data.bit (  7) = !side;
    return data;
  }

  if(addr == 0x4017) {
    if(side == 1) watchdog = 0;
    data.bit (  0) = data2(side);
    data.bit (  1) = 0;
    data.bits(2,7) = (side ? dipS : dipM).bits(2,7);
    return data;
  }

  if(addr >= 0x4020 && addr <= 0x5fff) {
    write(side, addr, data);
    return data;
  }

  if(addr >= 0x6000 && addr <= 0x7fff) {
    return side == ramSide ? ram[addr & 0x07ff] : data;
  }
}

auto VSSystem::write(bool side, uint16 addr, uint8 data) -> void {
  if(addr == 0x4016) {
    (side ? controllerPortS1 : controllerPortM1).device->latch(data.bit(0));
    (side ? controllerPortS2 : controllerPortM2).device->latch(data.bit(0));
    latch(side, data.bit(0));
    if(side == 0 && !forceSubRAM) ramSide = !data.bit(1);
    (side ? cpuM : cpuS).irqLine(!data.bit(1));
  }

  if(addr >= 0x4020 && addr <= 0x5fff) {
    //increment coin counter
  }

  if(addr >= 0x6000 && addr <= 0x7fff) {
    if(side == ramSide) ram[addr & 0x07ff] = data;
  }
}

}
