#include <fc/fc.hpp>

namespace Famicom {

FamicomBox famicombox;

#include "serialization.cpp"

auto FamicomBox::Enter() -> void {
  while(true) scheduler.synchronize(), famicombox.main();
}

auto FamicomBox::main() -> void {
  if(--attractionTimer == 0x7fff) trap(Exception::AttractionTimer);
  if(++watchdog        == 0x0000) trap(Exception::Watchdog);
  if(coinModule.timer && --coinModule.timer == 0) trap(Exception::Coin);
  step(3 * 0x2000);
  synchronize(cpuM);
}

auto FamicomBox::load(Markup::Node node) -> bool {
  if(auto bios_prg = node["prg/rom/name"].text()) {
    if(auto fp = platform->open(ID::System, bios_prg, File::Read, File::Required)) {
      fp->read(famicombox.bios_prg, 32768);
    } else return false;
  }

  if(auto bios_chr = node["chr/rom/name"].text()) {
    if(auto fp = platform->open(ID::System, bios_chr, File::Read, File::Required)) {
      fp->read(famicombox.bios_chr, 8192);
    } else return false;
  }

  dip = platform->dipSettings(node);
  keyswitch = 1;

  return true;
}

auto FamicomBox::unload() -> void {
  Emulator::video.removeSprite(keyswitchSprite);
}

auto FamicomBox::power(bool reset) -> void {
  if(!reset) {
    create(FamicomBox::Enter, system.frequency());

    exceptionEnable = 0x00;
    exceptionTrap = 0xff;

    zapperGND = false;
    warmboot = false;
    enableControllers = true;
    swapControllers = true;

    cartridgeSelect = 0;
    cartridgeRowSelect = 0;

    coinModule.timer = 0;
    coinModule.min10 = false;
    coinModule.min20 = false;

    function<auto (uint16, uint8) -> uint8> reader;
    function<auto (uint16, uint8) -> void> writer;

    reader = {&FamicomBox::readWRAM, this};
    writer = {&FamicomBox::writeWRAM, this};
    busM.map(reader, writer, "0800-1fff");

    reader = {&FamicomBox::readIO, this};
    writer = {&FamicomBox::writeIO, this};
    busM.map(reader, writer, "4016-4017");
    busM.map(reader, writer, "5000-5fff");

    reader = {&FamicomBox::readSRAM, this};
    writer = {&FamicomBox::writeSRAM, this};
    busM.map(reader, writer, "6000-7fff");

    reader = {&FamicomBox::readCartridge, this};
    writer = {&FamicomBox::writeCartridge, this};
    busM.map(reader, writer, "8000-ffff");
    //The cartridge is only mapped to $8000-ffff, not $4018-ffff.

    keyswitchSprite = Emulator::video.createSprite(16, 16);
    keyswitchSprite->setPixels(Resource::Sprite::FamicomBoxOff);
    keyswitchSprite->setPosition(0, 224);
    keyswitchSprite->setVisible(true);
  }

  ledSelect  = 0;
  ramProtect = 0;
  ledFlash   = false;

  attractionTimer.bits(7,14) = 0xff;
  watchdog.bits(10,13) = 0x00;

  changeSlot(0);
  registerLock = false;

  synchronize(cpuM);
  synchronize(apuM);
  synchronize(cartridgeSlot[busM.slot]);

  cpuM.power(/* reset = */ true);
  apuM.power(/* reset = */ true);
  cartridgeSlot[busM.slot].power(/* reset = */ true);
}

auto FamicomBox::changeSlot(uint4 newSlot) -> void {
  if(newSlot == cartridgeSelect) return;
  //if(cartridgeSelect > 0) scheduler.remove(cartridgeSlot[busM.slot]);
  cartridgeSelect = newSlot;
  cartridgeRowSelect = (newSlot + 4) / 5;
  //if(cartridgeSelect > 0 && cartridgeSelect <= cartridgeSlot.size()) busM.slot = newSlot - 1;
  //cartridgeSlot[busM.slot].power(/* reset = */ false);
}

auto FamicomBox::trap(Exception exceptionId) -> void {
  if(!exceptionEnable.bit((uint)exceptionId)) return;
  exceptionTrap.bit((uint)exceptionId) = 0;
  power(/* reset = */ true);
}

auto FamicomBox::pollInputs() -> void {
  static bool resetButton = false;
  static bool keyswitchLeft = false;
  static bool keyswitchRight = false;
  static bool coin = false;
  bool state;

  state = platform->inputPoll(ID::Port::Hardware, ID::Device::FamicomBoxControls, 0);
  if(state && !resetButton) trap(Exception::Reset);
  resetButton = state;

  //The Keyswitch has 6 positions: 1 OFF ON 2 3 4.
  //Keyswitch positions:
  //1,   0x20: Game Title & Count Display
  //OFF, 0x01: Attract mode; play games in coin mode and free play mode
  //ON,  0x02: Key mode: play games, Other modes: black screen
  //2,   0x04: Free Play Mode (for testing)
  //3,   0x08: Self Check Screen Display
  //4,   0x10: Black screen
  uint oldKeyswitch = keyswitch;
  state = platform->inputPoll(ID::Port::Hardware, ID::Device::FamicomBoxControls, 1);
  if(state && !keyswitchLeft) {
    if(keyswitch > 0) keyswitch--, trap(Exception::KeyswitchRotate);
  }
  keyswitchLeft = state;

  state = platform->inputPoll(ID::Port::Hardware, ID::Device::FamicomBoxControls, 2);
  if(state && !keyswitchRight) {
    if(keyswitch < 5) keyswitch++, trap(Exception::KeyswitchRotate);
  }
  keyswitchRight = state;

  if(keyswitch != oldKeyswitch) {
    switch(keyswitch) {
    case 0: keyswitchSprite->setPixels(Resource::Sprite::FamicomBox1); break;
    case 1: keyswitchSprite->setPixels(Resource::Sprite::FamicomBoxOff); break;
    case 2: keyswitchSprite->setPixels(Resource::Sprite::FamicomBoxOn); break;
    case 3: keyswitchSprite->setPixels(Resource::Sprite::FamicomBox2); break;
    case 4: keyswitchSprite->setPixels(Resource::Sprite::FamicomBox3); break;
    case 5: keyswitchSprite->setPixels(Resource::Sprite::FamicomBox4); break;
    }
  }

  state = platform->inputPoll(ID::Port::Hardware, ID::Device::FamicomBoxControls, 3);
  if(state && !coin) {
    trap(Exception::Coin);
    uint second = floor(system.frequency() / (3 * 0x2000));
    if(coinModule.min10) coinModule.timer += 10 * 60 * second;
    if(coinModule.min20) coinModule.timer += 20 * 60 * second;
  }
  coin = state;
}

auto FamicomBox::readWRAM(uint16 addr, uint8 data) -> uint8 {
  switch(addr & 0xf800) {
  case 0x0000: return cpuM.ram[addr];
  case 0x0800:
  case 0x1000:
  case 0x1800: return bios_ram[addr - 0x800];
  }
}

auto FamicomBox::readIO(uint16 addr, uint8 data) -> uint8 {
  if(addr == 0x4016 || addr == 0x4017) {
    watchdog.bits(10,13) = 0;
    if(!enableControllers) return data;
    if(addr == 0x4017) data.bits(3,4) = dip.bit(9) ? expansionPort.device->data2().bits(3,4) : 0;
    if(swapControllers) addr ^= 1;
    switch(addr) {
    case 0x4016: data.bit(0) = controllerPortM1.device->data().bit(0); break;
    case 0x4017: data.bit(0) = controllerPortM2.device->data().bit(0); break;
    }
    if(addr == 0x4016 && data.bit(0)) trap(Exception::Controller);
    return data;
  }

  if(!registerLock) {
    switch(addr & 0xf007) {

    //Exception trap flags
    case 0x5000: {
      data = exceptionTrap;
      exceptionTrap = 0xff;
      return data;
    }

    //Not used
    case 0x5001: {
      return data;
    }

    //DIP Switches
    case 0x5002: {
      return dip.bits(0,7);
    }

    //Keyswitch position and coin module status
    case 0x5003: {
      const uint6 keyswitchData[] = {0x20, 0x01, 0x02, 0x04, 0x08, 0x10};
      data.bits(0,5) = keyswitchData[keyswitch];
      data.bit (  6) = coinModule.timer > 0;  //Coin module pin 9
      data.bit (  7) = 0;  //Coin module pin 10
      return data;
    }

    //Test connector (25-pin) inputs 2, 15, 3, 16, 4, 17, 5, 18
    case 0x5004: {
      return data;
    }

    //Expansion connector (50-pin) read signal on input pin 28
    case 0x5005: {
      //Data comes through pins 36, 11, 37, 12, 38, 13, 39, and 14.
      return data;
    }

    //Expansion connector (50-pin) read signal on input pin 27
    case 0x5006: {
      //Data comes through pins 36, 11, 37, 12, 38, 13, 39, and 14.
      return data;
    }

    //Misc. status
    case 0x5007: {
      data.bit(0) = cartridgeSelect > cartridgeSlot.size();  //Must be 0. Is 1 TV mode or a trap flag?
      data.bit(1) = 1;  //When this is 0, the keyswitch is mid-turn.
      data.bit(2) = !zapperGND && settings.expansionPort == ID::Device::BeamGun;
      //data.bit(3);  //Expansion connector (50-pin) input pin 21 inverted
      data.bit(4) = 0;  //CATV connector pin 8
      data.bit(5) = 0;  //relay position: 0=A, 1=B
      //data.bit(6);  //Expansion connector (50-pin) input pin 22 inverted
      data.bit(7) = !warmboot;
      return data;
    }

    }
  }

  return data;
}

auto FamicomBox::readSRAM(uint16 addr, uint8 data) -> uint8 {
  return test_ram[addr & 0x1fff];
}

auto FamicomBox::readCartridge(uint16 addr, uint8 data) -> uint8 {
  if(cartridgeRowSelect == 0 && cartridgeSelect == 0) return bios_prg[addr & 0x7fff];
  if(cartridgeSelect >= cartridgeRowSelect * 5 - 4 && cartridgeSelect <= cartridgeRowSelect * 5) {
    if(cartridgeSelect > cartridgeSlot.size()) return data;
    return cartridgeSlot[cartridgeSelect - 1].readPRG(addr, data);
  }
  return data;
}

auto FamicomBox::writeWRAM(uint16 addr, uint8 data) -> void {
  switch(addr & 0xf800) {
  case 0x0000: {
    if(ramProtect >= 1) cpuM.ram[addr] = data;
    return;
  }

  case 0x0800: {
    if(ramProtect >= 2) bios_ram[addr - 0x800] = data;
    return;
  }

  case 0x1000: {
    if(ramProtect >= 3) bios_ram[addr - 0x800] = data;
    return;
  }

  case 0x1800: {
    if(ramProtect >= 4) bios_ram[addr - 0x800] = data;
    return;
  }

  }
}

auto FamicomBox::writeIO(uint16 addr, uint8 data) -> void {
  if(addr == 0x4016 || addr == 0x4017 && enableControllers) return cpuM.writeCPU(addr, data);

  if(!registerLock) {
    switch(addr & 0xf007) {

    case 0x5000: {  //Exception enable flags
      data ^ (1 << (uint)Exception::Interrupt6_82Hz);  //Exception disable flag
      exceptionEnable = (data & 0xbf) | (1 << (uint)Exception::Watchdog);
      break;
    }

    case 0x5001: {  //Coin module flags and CATV output
      coinModule.min10 = data.bit(0);  //pin 1
      coinModule.min20 = data.bit(1);  //pin 2
      //data.bit(2): pin  3: always 0 except in test mode
      //data.bit(3): pin  4: always 0 except in test mode
      //data.bit(4): pin 12: always 0 except in test mode
      //data.bit(5): pin 14: ???
      //data.bit(6): CATV: 0=free, 1=pay?
      //data.bit(7): CATV: 0=OK, 1=controller error?
      break;
    }

    case 0x5002: {  //Cartridge slot LED and RAM protect register
      ledSelect  = data.bits(0,3);
      ramProtect = data.bits(4,6);
      ledFlash   = data.bit (  7);
      break;
    }

    case 0x5003: {  //Attraction timer
      attractionTimer.bits(7,14) = data;
      break;
    }

    case 0x5004: {  //Cartridge selection register
      changeSlot(data.bits(0,3));
      cartridgeRowSelect = data.bits(4,5);
      registerLock |= data.bit(6);
      break;
    }

    case 0x5005: {  //Misc. control
      zapperGND = data.bit(2);
      warmboot = data.bit(5);
      enableControllers = !data.bit(6);
      swapControllers = !data.bit(7);
      break;
    }

    case 0x5006: {  //Test connector DB-25 outputs 6, 19, 7, 20, 8, 21, 9, 22
      break;
    }

    case 0x5007: {  //Expansion connector (50-pin) write signal on pin 26
      //Data goes through pins 36, 11, 37, 12, 38, 13, 39, and 14.
      break;
    }

    }
  }
}

auto FamicomBox::writeSRAM(uint16 addr, uint8 data) -> void {
  test_ram[addr & 0x1fff] = data;
}

auto FamicomBox::writeCartridge(uint16 addr, uint8 data) -> void {
  if(cartridgeRowSelect == 0 && cartridgeSelect == 0) return;
  if(cartridgeSelect >= cartridgeRowSelect * 5 - 4 && cartridgeSelect <= cartridgeRowSelect * 5) {
    if(cartridgeSelect > cartridgeSlot.size()) return;
    return cartridgeSlot[cartridgeSelect - 1].writePRG(addr, data);
  }
}

auto FamicomBox::readCHR(uint14 addr, uint8 data) -> uint8 {
  if(cartridgeRowSelect == 0 && cartridgeSelect == 0) {
    if(addr & 0x2000) return ppuM.readCIRAM((addr & 0x3ff) | (addr & 0x800) >> 1);
    return bios_chr[addr];
  }
  if(cartridgeSelect >= cartridgeRowSelect * 5 - 4 && cartridgeSelect <= cartridgeRowSelect * 5) {
    if(cartridgeSelect > cartridgeSlot.size()) return data;
    return cartridgeSlot[cartridgeSelect - 1].readCHR(addr, data);
  }
  return data;
}

auto FamicomBox::writeCHR(uint14 addr, uint8 data) -> void {
  if(cartridgeRowSelect == 0 && cartridgeSelect == 0) {
    if(addr & 0x2000) return ppuM.writeCIRAM((addr & 0x3ff) | (addr & 0x800) >> 1, data);
    return;
  }
  if(cartridgeSelect >= cartridgeRowSelect * 5 - 4 && cartridgeSelect <= cartridgeRowSelect * 5) {
    if(cartridgeSelect > cartridgeSlot.size()) return;
    return cartridgeSlot[cartridgeSelect - 1].writeCHR(addr, data);
  }
}

}
