//VS

struct VS : Board {
  VS(Markup::Node& boardNode) : Board(boardNode),
  mmc1(*this, boardNode),
  n108(*this, boardNode) {
    chipType = ChipType::None;
    string type = boardNode["chip/type"].text();
    if(type.match("74HC32")) chipType = ChipType::_74HC32;
    if(type.match("MMC1*" )) chipType = ChipType::MMC1;
    if(type.match("108"   )) chipType = ChipType::N108;
    if(type.match("109"   )) chipType = ChipType::N108;
    if(type.match("118"   )) chipType = ChipType::N108;
    if(type.match("119"   )) chipType = ChipType::N108;
  }

  auto main() -> void {
    if(chipType == ChipType::MMC1) return mmc1.main();
    tick();
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    switch(chipType) {
    case ChipType::None: {
      if(addr & 0x8000) {
        if(addr < 0xe000 && prgrom.size() < 0x2000) return data;
        if(addr < 0xc000 && prgrom.size() < 0x4000) return data;
        if(addr < 0xa000 && prgrom.size() < 0x6000) return data;
        addr &= 0x7fff;
        if(prgrom.size() > 0x8000) {  //Games with oversize 1D such as VS. Gumshoe
          if(addr >= 0x2000 || bank == 1) addr += 0x2000;
        }
        return read(prgrom, addr);
      }
      break;
    }

    case ChipType::_74HC32: {
      if(addr & 0x8000) {
        if((addr & 0xc000) == 0x8000)
          return read(prgrom, (bank << 14) | (addr & 0x3fff));
        else
          return read(prgrom, (0x0f << 14) | (addr & 0x3fff));
      }
      break;
    }

    case ChipType::MMC1: {
      if(addr & 0x8000) return read(prgrom, mmc1.prgAddress(addr));
      //if(revision == Revision::SUROM || revision == Revision::SXROM) {
      //  addr |= ((mmc1.chrBank[lastCHRBank] & 0x10) >> 4) << 18;
      //}
      break;
    }

    case ChipType::N108: {
      if(addr & 0x8000) return read(prgrom, n108.prgAddress(addr));
      break;
    }

    }
    if((addr & 0xe000) == 0x6000 && prgram.size() > 0) return read(prgram, addr);
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    switch(chipType) {
    case ChipType::None:
      if(addr == 0x4016) bank = data.bit(2);
      break;
    case ChipType::_74HC32:
      //TODO: Check if VS. UNROM has bus conflicts
      //data &= readPRG(addr, data);
      if(addr & 0x8000) bank = data.bits(0,3);
      break;
    case ChipType::MMC1:
      if(addr & 0x8000) return mmc1.mmioWrite(addr, data);
      break;
    case ChipType::N108:
      if(addr & 0x8000) return n108.regWrite(addr, data);
      break;
    }
    if((addr & 0xe000) == 0x6000 && prgram.size() > 0) return write(prgram, addr, data);
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) return ppu.readCIRAM(addr);
    switch(chipType) {
    case ChipType::None:
      if(chrrom.size() < bank << 13) return data;
      return read(chrrom, ((bank << 13) + (addr & 0x1fff)));
    case ChipType::_74HC32:
      return Board::readCHR(addr, data);
    case ChipType::MMC1:
      return Board::readCHR(mmc1.chrAddress(addr), data);
    case ChipType::N108:
      return Board::readCHR(n108.chrAddress(addr), data);
    }
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) return ppu.writeCIRAM(addr, data);
    switch(chipType) {
    case ChipType::None:
      break;
    case ChipType::_74HC32:
      return Board::writeCHR(addr, data);
    case ChipType::MMC1:
      return Board::writeCHR(mmc1.chrAddress(addr), data);
    case ChipType::N108:
      return Board::writeCHR(n108.chrAddress(addr), data);
    }
  }

  auto power(bool reset) -> void {
    bank = 0;
    switch(chipType) {
    case ChipType::MMC1: mmc1.power(reset); break;
    case ChipType::N108: n108.power(reset); break;
    }
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    s.integer(bank);
    switch(chipType) {
    case ChipType::MMC1: mmc1.serialize(s); break;
    case ChipType::N108: n108.serialize(s); break;
    }
  }

  enum class ChipType : uint {
    None,
    _74HC32,
    MMC1,
    N108,
  } chipType;

  uint4 bank;
  MMC1 mmc1;
  N108 n108;
};
