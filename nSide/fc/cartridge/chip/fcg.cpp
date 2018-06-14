struct FCG : Chip {
  FCG(Board& board, Markup::Node& boardNode) : Chip(board) {
    string type = boardNode["chip[0]/type"].text();
    if(type == "FCG-1"  ) revision = Revision::FCG1;
    if(type == "FCG-2"  ) revision = Revision::FCG2;
    if(type == "LZ93D50") revision = Revision::LZ93D50;

    eeprom = EEPROM::None;
    type = boardNode["chip[1]/type"].text();
    if(type == "24C01") eeprom = EEPROM::_24C01;
    if(type == "24C02") eeprom = EEPROM::_24C02;
  }

  auto main() -> void {
    if(irqCounterEnable) {
      if(--irqCounter == 0xffff) {
        cpu.irqLine(1);
        irqCounterEnable = false;
      }
    }

    tick();
  }

  auto prgAddress(uint addr) const -> uint {
    bool region = addr & 0x4000;
    return ((region == 0 ? prgBank : (uint8)0x0f) << 14) | (addr & 0x3fff);
  }

  auto chrAddress(uint addr) const -> uint {
    return (chrBank[addr >> 10] << 10) | (addr & 0x03ff);
  }

  auto ciramAddress(uint addr) const -> uint {
    switch(mirror) {
    case 0: return ((addr & 0x0400) >> 0) | (addr & 0x03ff);
    case 1: return ((addr & 0x0800) >> 1) | (addr & 0x03ff);
    case 2: return 0x0000 | (addr & 0x03ff);
    case 3: return 0x0400 | (addr & 0x03ff);
    }
  }

  auto ramRead(uint addr, uint8 data) -> uint8 {
    return board.read(board.prgram, addr & 0x1fff);
  }

  auto ramWrite(uint addr, uint8 data) -> void {
    board.write(board.prgram, addr & 0x1fff, data);
  }

  auto regWrite(uint addr, uint8 data) -> void {
    switch(addr & 0x0f) {
    case 0x00: case 0x01: case 0x02: case 0x03:
    case 0x04: case 0x05: case 0x06: case 0x07:
      chrBank[addr & 7] = data;
      break;
    case 0x08:
      prgBank = data & 0x0f;
      break;
    case 0x09:
      mirror = data & 0x03;
      break;
    case 0x0a:
      cpu.irqLine(0);
      irqCounterEnable = data & 0x01;
      irqCounter = irqLatch;
      break;
    case 0x0b:
      irqLatch = (irqLatch & 0xff00) | (data << 0);
      break;
    case 0x0c:
      irqLatch = (irqLatch & 0x00ff) | (data << 8);
      break;
    case 0x0d:
      //TODO: serial EEPROM support
      eepromI2C_SCL = data & 0x20;
      eepromEnableRead = data & 0x80;
      break;
    }
  }

  auto power(bool reset) -> void {
    for(auto &n : chrBank) n = 0;
    prgBank = 0;
    mirror = 0;
    irqCounterEnable = 0;
    irqCounter = 0;
    irqLatch = 0;
    eepromI2C_SCL = 0;
    eepromEnableRead = 0;
  }

  auto serialize(serializer& s) -> void {
    s.array(chrBank);
    s.integer(prgBank);
    s.integer(mirror);
    s.integer(irqCounterEnable);
    s.integer(irqCounter);
    s.integer(irqLatch);
    s.integer(eepromI2C_SCL);
    s.integer(eepromEnableRead);
  }

  enum class Revision : uint {
    FCG1,
    FCG2,
    LZ93D50,
  } revision;

  enum class EEPROM : uint {
    None,
    _24C01,
    _24C02
  } eeprom;

  uint8 chrBank[8];
  uint8 prgBank;
  uint2 mirror;
  bool irqCounterEnable;
  uint16 irqCounter;
  uint16 irqLatch;
  bool eepromI2C_SCL;  //When using SRAM, enables RAM chip
  bool eepromEnableRead;
};
