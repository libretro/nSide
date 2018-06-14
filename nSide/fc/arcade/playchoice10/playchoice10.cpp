#include <fc/fc.hpp>

namespace Famicom {

PlayChoice10 playchoice10;

#include "cpu.cpp"
#include "video-circuit.cpp"
#include "serialization.cpp"

auto PlayChoice10::load(Markup::Node node) -> bool {
  if(auto firmware = node["pc10/cpu/rom/name"].text()) {
    if(auto fp = platform->open(ID::System, firmware, File::Read, File::Required)) {
      fp->read(bios, 16384);
    } else return false;
  }

  if(auto character = node["pc10/video-circuit/vrom/name"].text()) {
    if(auto fp = platform->open(ID::System, character, File::Read, File::Required)) {
      fp->read(videoCircuit.chrrom, 24576);
    } else return false;
  }

  if(auto palette = node["pc10/video-circuit/cgrom/name"].text()) {
    if(auto fp = platform->open(ID::System, palette, File::Read, File::Required)) {
      fp->read(videoCircuit.cgrom, 768);
    } else return false;
  }

  screenConfig = min(max(node["pc10/screen/mode"].integer(), 1), 2);

  dip = platform->dipSettings(node["pc10"]);

  return true;
}

auto PlayChoice10::power(bool reset) -> void {
  pc10cpu.power(reset);
  videoCircuit.power(reset);

  if(!reset) {
    function<auto (uint16, uint8) -> uint8> reader;
    function<auto (uint16, uint8) -> void> writer;

    reader = {&PlayChoice10::readController1, this};
    writer = {&PlayChoice10::latchControllers, this};
    busM.map(reader, writer, "4016-4016");

    nmiDetected = false;

    vramAccess      = 0;  //0: Z80,                  1: video circuit
    gameSelectStart = 0;  //0: disable START/SELECT, 1: enable START/SELECT
    ppuOutput       = 0;  //0: disable,              1: enable
    apuOutput       = 0;  //0: disable,              1: enable
    cpuReset        = 0;  //0: reset,                1: run
    cpuStop         = 0;  //0: stop,                 1: run
    display         = 0;  //0: video circuit,        1: PPU
    z80NMI          = 0;  //0: disable,              1: enable
    watchdog        = 0;  //0: enable,               1: disable
    ppuReset        = 0;  //0: reset,                1: run

    random.array(wram, sizeof(wram));

    channel  = 0;  //channel 1 on-screen
    sramBank = 1;

    promAddress = 0;
  }
}

auto PlayChoice10::read(uint16 addr) -> uint8 {
  if(addr < 0x8000) return bios[addr & 0x3fff];
  if(addr < 0x8800) return wram[addr & 0x07ff];
  if(addr < 0x8c00) return sram[addr & 0x03ff];
  if(addr < 0x9000) return sram[(addr & 0x03ff) | (sramBank << 10)];
  if(addr < 0x9800) return 0x00;  //VRAM is write-only
  if(addr < 0xc000) return 0x00;  //open bus
  if(channel >= cartridgeSlot.size()) return 0xff;
  if(addr < 0xe000) return cartridgeSlot[busM.slot].board->instrom.read(addr & 0x1fff);

  //PROM
  uint8 data = 0xe7;
  uint8 byte;
  if(!promTest || !promAddress.bit(6)) {
    byte = cartridgeSlot[busM.slot].board->keyrom.read((promAddress & 0x3f) >> 3);
  } else {
    byte = promAddress.bit(4) ? (uint8)0x00 : cartridgeSlot[busM.slot].board->keyrom.read(8);
  }
  data.bit(3) = !byte.bit(promAddress & 7);
  data.bit(4) = !promAddress.bit(5);
  return data;
}

auto PlayChoice10::write(uint16 addr, uint8 data) -> void {
  if(addr < 0x8000) return;
  if(addr < 0x8800) { wram[addr & 0x07ff] = data; return; }
  if(addr < 0x8c00) { sram[addr & 0x03ff] = data; return; }
  if(addr < 0x9000) { sram[(addr & 0x03ff) | (sramBank << 10)] = data; return; }
  if(addr < 0x9800) { videoCircuit.writeVRAM(addr, data); return; }
  if(addr < 0xc000) return;
  if(addr < 0xe000) return;

  promTest = data.bit(4);
  if(promClock && data.bit(3) == 0) promAddress++;
  promClock = data.bit(3);
  if(data.bit(0) == 0) promAddress = 0;
  return;
}

auto PlayChoice10::in(uint8 addr) -> uint8 {
  bool channelSelect = poll(ChannelSelect);
  bool enter         = poll(Enter);
  bool reset         = poll(Reset);
  bool nmi           = !nmiDetected;
  bool primeTime     = false;
  bool coin2         = poll(Coin2);
  bool service       = poll(ServiceButton);
  bool coin1         = poll(Coin1);
  switch(addr & 0x03) {
  case 0x00:
    return channelSelect << 0 | enter << 1 | reset << 2 | nmi << 3
    | primeTime << 4 | coin2 << 5 | service << 6 | coin1 << 7;
  case 0x01: return dip.byte(0);
  case 0x02: return dip.byte(1);
  case 0x03: return nmiDetected = false, 0x00;
  }
  unreachable;
}

auto PlayChoice10::out(uint8 addr, uint8 data) -> void {
  data &= 0x01;
  switch(addr & 0x1f) {
  case 0x00: {
    vramAccess = data;
    break;
  }
  case 0x01: {
    gameSelectStart = data;
    break;
  }
  case 0x02: {
    ppuOutput = data;
    break;
  }
  case 0x03: {
    apuOutput = data;
    break;
  }
  case 0x04: {
    if(!cpuReset && data) {
      cpuM.power(/* reset = */ true);
      apuM.power(/* reset = */ true);
    };
    cpuReset = data;
    break;
  }
  case 0x05: {
    cpuStop = data;
    print(data ? "Start" : "Stop", " CPU\n");
    break;
  }
  case 0x06: {
    display = data;
    break;
  }
  case 0x08: {
    z80NMI = data;
    break;
  }
  case 0x09: {
    watchdog = data;
    print(data ? "Enable" : "Disable", " Watchdog\n");
    break;
  }
  case 0x0a: {
    if(!ppuReset && data) ppuM.power(/* reset = */ true);
    ppuReset = data;
    break;
  }
  case 0x0b: {
    uint4 newChannel = channel;
    newChannel.bit(0) = data;
    changeChannel(newChannel);
    break;
  }
  case 0x0c: {
    uint4 newChannel = channel;
    newChannel.bit(1) = data;
    changeChannel(newChannel);
    break;
  }
  case 0x0d: {
    uint4 newChannel = channel;
    newChannel.bit(2) = data;
    changeChannel(newChannel);
    break;
  }
  case 0x0e: {
    uint4 newChannel = channel;
    newChannel.bit(3) = data;
    changeChannel(newChannel);
    break;
  }
  case 0x0f: {
    sramBank = data;
    break;
  }

  }
  switch(addr & 0x13) {
  case 0x10: break;
  case 0x11: break;
  case 0x12: break;
  case 0x13: break;
  }
}

auto PlayChoice10::changeChannel(uint4 newChannel) -> void {
  if(newChannel == channel) return;
  scheduler.remove(cartridgeSlot[channel]);
  channel = newChannel;
  if(channel < cartridgeSlot.size()) busM.slot = channel;
  cartridgeSlot[busM.slot].power(false);
}

auto PlayChoice10::poll(uint input) -> int16 {
  return platform->inputPoll(ID::Port::Hardware, ID::Device::PlayChoice10Controls, (uint)input);
}

auto PlayChoice10::readController1(uint16 addr, uint8 data) -> uint8 {
  auto gamepad = static_cast<Gamepad*>(controllerPortM1.device);
  uint counter = gamepad->counter;
  uint8 input = cpuM.readCPU(addr, data);
  switch(gamepad->latched ? 0 : counter) {
  case 2: data.bit(0) = controller1GameSelect; break;
  case 3: data.bit(0) = controller1Start; break;
  default: data.bit(0) = input.bit(0); break;
  }
  return data;
}

auto PlayChoice10::latchControllers(uint16 addr, uint8 data) -> void {
  auto gamepad = static_cast<Gamepad*>(controllerPortM1.device);
  bool old_latched = gamepad->latched;
  cpuM.writeCPU(addr, data);
  if(old_latched == data.bit(0)) return;
  if(gamepad->latched == 0) {
    controller1GameSelect = gameSelectStart & poll(GameSelect);
    controller1Start      = gameSelectStart & poll(Start);
  }
}

}
