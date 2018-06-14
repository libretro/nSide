struct HVC_SxROM : Board {
  HVC_SxROM(Markup::Node& boardNode) : Board(boardNode), mmc1(*this, boardNode) {
    string type = boardNode["id"].text();
    if(type.match("*SAROM"   )) revision = Revision::SAROM;
    if(type.match("*SBROM"   )) revision = Revision::SBROM;
    if(type.match("*SCROM"   )) revision = Revision::SCROM;
    if(type.match("*SC1ROM"  )) revision = Revision::SC1ROM;
    if(type.match("*SEROM"   )) revision = Revision::SEROM;
    if(type.match("*SFROM"   )) revision = Revision::SFROM;
    if(type.match("*SF1ROM"  )) revision = Revision::SF1ROM;
    if(type.match("*SFEXPROM")) revision = Revision::SFEXPROM;
    if(type.match("*SGROM"   )) revision = Revision::SGROM;
    if(type.match("*SHROM"   )) revision = Revision::SHROM;
    if(type.match("*SH1ROM"  )) revision = Revision::SH1ROM;
    if(type.match("*SIROM"   )) revision = Revision::SIROM;
    if(type.match("*SJROM"   )) revision = Revision::SJROM;
    if(type.match("*SKROM"   )) revision = Revision::SKROM;
    if(type.match("*SLROM"   )) revision = Revision::SLROM;
    if(type.match("*SL1ROM"  )) revision = Revision::SL1ROM;
    if(type.match("*SL2ROM"  )) revision = Revision::SL2ROM;
    if(type.match("*SL3ROM"  )) revision = Revision::SL3ROM;
    if(type.match("*SLRROM"  )) revision = Revision::SLRROM;
    if(type.match("*SMROM"   )) revision = Revision::SMROM;
    if(type.match("*SNROM"   )) revision = Revision::SNROM;
    if(type.match("*SNWEPROM")) revision = Revision::SNWEPROM;
    if(type.match("*SOROM"   )) revision = Revision::SOROM;
    if(type.match("*SUROM"   )) revision = Revision::SUROM;
    if(type.match("*SXROM"   )) revision = Revision::SXROM;
  }

  auto main() -> void {
    mmc1.main();
  }

  auto ramAddress(uint addr) -> uint {
    uint bank = 0;
    if(revision == Revision::SOROM) bank = (mmc1.chrBank[0] & 0x08) >> 3;
    if(revision == Revision::SXROM) bank = (mmc1.chrBank[0] & 0x0c) >> 2;
    return (bank << 13) | (addr & 0x1fff);
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if((addr & 0xe000) == 0x6000) {
      if(revision == Revision::SNROM) {
        if((mmc1.chrAddress(ppu.io.chrAddressBus) >> 16) & 1) return data;
      }
      if(mmc1.ramDisable) return data;
      if(prgram.size() > 0) return read(prgram, ramAddress(addr));
    }

    if(addr & 0x8000) {
      switch(revision) {
      default:
        addr = mmc1.prgAddress(addr);
        if(revision == Revision::SUROM || revision == Revision::SXROM) {
          addr |= ((mmc1.chrAddress(ppu.io.chrAddressBus) >> 16) & 1) << 18;
        }
        break;
      case Revision::SEROM:
      case Revision::SHROM:
      case Revision::SH1ROM:
        break;
      case Revision::SFEXPROM:
        addr = mmc1.prgAddress(addr);
        if((expLock & 0x20) && (addr & 0x7fff) == 0x0180) return 0x05;
        break;
      }
      return read(prgrom, addr);
    }

    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if((addr & 0xe000) == 0x6000) {
      if(revision == Revision::SNROM) {
        if(mmc1.chrBank[0] & 0x10) return;
      } else if(revision == Revision::SFEXPROM) {
        expLock = data;
      }
      if(mmc1.ramDisable) return;
      if(prgram.size() > 0) return write(prgram, ramAddress(addr), data);
    }

    if(addr & 0x8000) return mmc1.mmioWrite(addr, data);
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) return ppu.readCIRAM(mmc1.ciramAddress(addr));
    return Board::readCHR(mmc1.chrAddress(addr), data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) return ppu.writeCIRAM(mmc1.ciramAddress(addr), data);
    return Board::writeCHR(mmc1.chrAddress(addr), data);
  }

  auto power(bool reset) -> void {
    mmc1.power(reset);
    expLock = 0x00;
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    mmc1.serialize(s);
    if(revision == Revision::SFEXPROM) {
      s.integer(expLock);
    }
  }

  enum class Revision : uint {
    SAROM,
    SBROM,
    SCROM,
    SC1ROM,
    SEROM,
    SFROM,
    SF1ROM,
    SFEXPROM,
    SGROM,
    SHROM,
    SH1ROM,
    SIROM,
    SJROM,
    SKROM,
    SLROM,
    SL1ROM,
    SL2ROM,
    SL3ROM,
    SLRROM,
    SMROM,
    SNROM,
    SNWEPROM,
    SOROM,
    SUROM,
    SXROM,
  } revision;

  MMC1 mmc1;
  uint8 expLock;
};
