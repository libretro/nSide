//AVE-NINA-01
//AVE-NINA-02
//AVE-NINA-03
//AVE-NINA-06

struct Nina : Board {
  Nina(Markup::Node& boardNode) : Board(boardNode) {
    string type = boardNode["id"].text();
    if(type.match("*NINA-01")) revision =  1;
    if(type.match("*NINA-02")) revision =  2;
    if(type.match("*NINA-03")) revision =  3;
    if(type.match("*NINA-06")) revision =  6;

    if(revision ==  1 || revision ==  2) {
      settings.mirror = 1;
    } else {
      settings.mirror = boardNode["mirror/mode"].text() == "horizontal";
    }
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if(addr & 0x8000) return read(prgrom, (prgBank << 15) | (addr & 0x7fff));
    if((addr & 0xe000) == 0x6000) {
      if(prgram.size() > 0) return read(prgram, addr);
    }
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    switch(revision) {
    case  1:
    case  2:
      switch(addr) {
      case 0x7ffd: prgBank    = data & 0x01; break;
      case 0x7ffe: chrBank[0] = data & 0x0f; break;
      case 0x7fff: chrBank[1] = data & 0x0f; break;
      }
      break;
    case  3:
    case  6:
      if((addr & 0xe100) == 0x4100) {
        prgBank    = (data & 0x08) >> 3;
        chrBank[0] = ((data & 0x07) << 1) | 0;
        chrBank[1] = ((data & 0x07) << 1) | 1;
      }
      break;
    }
    if((addr & 0xe000) == 0x6000 && prgram.size() > 0) write(prgram, addr, data);
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) {
      if(settings.mirror == 1) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.readCIRAM(addr);
    }
    return Board::readCHR((addr & 0x0fff) | (chrBank[(addr & 0x1000) >> 12] << 12), data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) {
      if(settings.mirror == 1) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.writeCIRAM(addr, data);
    }
    return Board::writeCHR((addr & 0x0fff) | (chrBank[(addr & 0x1000) >> 12] << 12), data);
  }

  auto power(bool reset) -> void {
    prgBank = 0;
    chrBank[0] = 0;
    chrBank[1] = 0;
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    s.integer(prgBank);
    s.array(chrBank);
  }

  uint revision;

  struct Settings {
    bool mirror;  //0 = vertical, 1 = horizontal
  } settings;

  bool prgBank;
  uint4 chrBank[2];
};
