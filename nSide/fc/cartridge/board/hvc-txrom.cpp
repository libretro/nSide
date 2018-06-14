struct HVC_TxROM : Board {
  HVC_TxROM(Markup::Node& boardNode) : Board(boardNode), mmc3(*this, boardNode) {
    string type = boardNode["id"].text();
    if(type.match("*TBROM"  )) revision = Revision::TBROM;
    if(type.match("*TEROM"  )) revision = Revision::TEROM;
    if(type.match("*TFROM"  )) revision = Revision::TFROM;
    if(type.match("*TGROM"  )) revision = Revision::TGROM;
    if(type.match("*TKROM"  )) revision = Revision::TKROM;
    if(type.match("*TKEPROM")) revision = Revision::TKEPROM;
    if(type.match("*TKSROM" )) revision = Revision::TKSROM;
    if(type.match("*TLROM"  )) revision = Revision::TLROM;
    if(type.match("*TL1ROM" )) revision = Revision::TL1ROM;
    if(type.match("*TL2ROM" )) revision = Revision::TL2ROM;
    if(type.match("*TLSROM" )) revision = Revision::TLSROM;
    if(type.match("*TNROM"  )) revision = Revision::TNROM;
    if(type.match("*TQROM"  )) revision = Revision::TQROM;
    if(type.match("*TR1ROM" )) revision = Revision::TR1ROM;
    if(type.match("*TSROM"  )) revision = Revision::TSROM;
    if(type.match("*TVROM"  )) revision = Revision::TVROM;
    if(type.match("*MC-ACC" )) revision = Revision::MCACC;
  }

  auto main() -> void {
    mmc3.main();
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if((addr & 0xe000) == 0x6000 && prgram.size() > 0) return mmc3.ramRead(addr, data);
    if(addr & 0x8000) return read(prgrom, mmc3.prgAddress(addr));
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if((addr & 0xe000) == 0x6000 && prgram.size() > 0) return mmc3.ramWrite(addr, data);
    if(addr & 0x8000) return mmc3.regWrite(addr, data);
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(revision == Revision::TR1ROM || revision == Revision::TVROM) {
      if(addr & 0x2000) return read(chrram, addr);
      return read(chrrom, mmc3.chrAddress(addr));
    }
    if(addr & 0x2000) return ppu.readCIRAM(ciramAddress(addr));
    if(revision == Revision::TQROM) {
      if(mmc3.chrAddress(addr) & (0x40 << 10)) {
        return read(chrram, mmc3.chrAddress(addr));
      } else {
        return read(chrrom, mmc3.chrAddress(addr));
      }
    }
    return Board::readCHR(mmc3.chrAddress(addr), data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(revision == Revision::TR1ROM || revision == Revision::TVROM) {
      if(addr & 0x2000) write(chrram, addr, data);
      return;
    }
    if(addr & 0x2000) return ppu.writeCIRAM(ciramAddress(addr), data);
    return Board::writeCHR(mmc3.chrAddress(addr), data);
  }

  auto ciramAddress(uint addr) -> uint {
    switch(revision) {
    default:
      return mmc3.ciramAddress(addr);
    case Revision::TKSROM:
    case Revision::TLSROM:
      return ((mmc3.chrAddress(addr & 0xfff) & 0x20000) >> 7) | (addr & 0x3ff);
    }
  }

  auto power(bool reset) -> void {
    mmc3.power(reset);
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    mmc3.serialize(s);
  }

  enum class Revision : uint {
    TBROM,
    TEROM,
    TFROM,
    TGROM,
    TKROM,
    TKEPROM,
    TKSROM,
    TLROM,
    TL1ROM,
    TL2ROM,
    TLSROM,
    TNROM,
    TQROM,
    TR1ROM,
    TSROM,
    TVROM,
    MCACC,
  } revision;

  MMC3 mmc3;
};
