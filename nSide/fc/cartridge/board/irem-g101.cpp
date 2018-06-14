struct IremG101 : Board {
  IremG101(Markup::Node& boardNode) : Board(boardNode), g101(*this, boardNode) {
    if(!boardNode["mirror"]) {
      settings.mirror = 0;
    } else {
      string mirror = boardNode["mirror/mode"].text();
      if(mirror == "screen-0") settings.mirror = 1;
      if(mirror == "screen-1") settings.mirror = 2;
    }
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if((addr & 0x8000) == 0x8000) return read(prgrom, g101.prgAddress(addr));
    if((addr & 0xe000) == 0x6000) return read(prgram, addr & 0x1fff);
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if((addr & 0x8000) == 0x8000) return g101.regWrite(addr, data);
    if((addr & 0xe000) == 0x6000) return write(prgram, addr & 0x1fff, data);
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) switch(settings.mirror) {
    case 0: return ppu.readCIRAM(g101.ciramAddress(addr));
    case 1: return ppu.readCIRAM((addr & 0x03ff) | 0x0400);
    case 2: return ppu.readCIRAM((addr & 0x03ff) | 0x0800);
    }
    return Board::readCHR(g101.chrAddress(addr), data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) switch(settings.mirror) {
    case 0: return ppu.writeCIRAM(g101.ciramAddress(addr), data);
    case 1: return ppu.writeCIRAM((addr & 0x03ff) | 0x0400, data);
    case 2: return ppu.writeCIRAM((addr & 0x03ff) | 0x0800, data);
    }
    return Board::writeCHR(g101.chrAddress(addr), data);
  }

  auto power(bool reset) -> void {
    g101.power(reset);
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    g101.serialize(s);
  }

  struct Settings {
    uint2 mirror;  //0 = G101-controlled, 1 = screen 0, 2 = screen 1
  } settings;

  G101 g101;
};
