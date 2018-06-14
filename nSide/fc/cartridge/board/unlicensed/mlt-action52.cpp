//MLT-ACTION52

struct MLT_Action52 : Board {
  MLT_Action52(Markup::Node& boardNode) : Board(boardNode) {
    settings.connectedChips = boardNode["connected-chips"].natural();
    uint2 chipBlock = 0;
    for(uint chipID = 0; chipID < 4; chipID++) {
      if(settings.connectedChips & (0x1 << chipID)) settings.chipMap[chipID] = chipBlock++;
    }
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if(addr & 0x8000 && settings.connectedChips & (1 << prgChip)) {
      uint bank = prgBank;
      bank |= settings.chipMap[prgChip] << 5;
      if(prgMode) return read(prgrom, (bank << 14) | (addr & 0x3fff));
      else        return read(prgrom, (bank << 14) | (addr & 0x7fff));
    }
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if(addr & 0x8000) {
      mirror  = (addr & 0x2000) >> 13;
      prgChip = (addr & 0x1800) >> 11;
      prgBank = (addr & 0x07c0) >> 6;
      prgMode = (addr & 0x0020) >> 5;
      chrBank = ((addr & 0x000f) << 2) + (data & 0x03);
    }
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) {
      if(mirror) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.readCIRAM(addr);
    }
    addr = (chrBank * 0x2000) + (addr & 0x1fff);
    return Board::readCHR(addr, data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) {
      if(mirror) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.writeCIRAM(addr, data);
    }
    addr = (chrBank * 0x2000) + (addr & 0x1fff);
    Board::writeCHR(addr, data);
  }

  auto power(bool reset) -> void {
    mirror = 0;
    prgChip = 0;
    prgBank = 0;
    prgMode = 0;
    chrBank = 0;
  }

  auto serialize(serializer &s) -> void {
    Board::serialize(s);
    s.integer(mirror);
    s.integer(prgChip);
    s.integer(prgBank);
    s.integer(prgMode);
    s.integer(chrBank);
  }

  struct Settings {
    uint4 connectedChips;
    uint2 chipMap[4];
  } settings;

  bool mirror;
  uint2 prgChip;
  uint5 prgBank;
  bool prgMode;
  uint6 chrBank;
};
