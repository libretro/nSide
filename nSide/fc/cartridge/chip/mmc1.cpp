struct MMC1 : Chip {
  MMC1(Board& board, Markup::Node& boardNode) : Chip(board) {
    string type = boardNode["chip/type"].text();

    if(type == "MMC1"    ) revision = Revision::MMC1;
    if(type == "MMC1A"   ) revision = Revision::MMC1A;
    if(type == "MMC1B1"  ) revision = Revision::MMC1B1;
    if(type == "MMC1B1-H") revision = Revision::MMC1B1;
    if(type == "MMC1B2"  ) revision = Revision::MMC1B2;
    if(type == "MMC1B3"  ) revision = Revision::MMC1B3;
    if(type == "MMC1C"   ) revision = Revision::MMC1C;
  }

  auto main() -> void {
    if(writedelay) writedelay--;
    tick();
  }

  auto prgAddress(uint addr) -> uint {
    bool region = addr & 0x4000;
    uint bank = (prgBank & ~1) + region;

    if(prgSize) {
      bank = (region == 0 ? 0x0 : 0xf);
      if(region != prgMode) bank = prgBank;
    }

    return (bank << 14) | (addr & 0x3fff);
  }

  auto chrAddress(uint addr) -> uint {
    bool region = addr & 0x1000;
    uint bank = chrBank[region];
    if(chrMode == 0) bank = (chrBank[0] & ~1) | region;
    return (bank << 12) | (addr & 0x0fff);
  }

  auto ciramAddress(uint addr) -> uint {
    switch(mirror) {
    case 0: return 0x0000 | (addr & 0x03ff);
    case 1: return 0x0400 | (addr & 0x03ff);
    case 2: return ((addr & 0x0400) >> 0) | (addr & 0x03ff);
    case 3: return ((addr & 0x0800) >> 1) | (addr & 0x03ff);
    }
  }

  auto mmioWrite(uint addr, uint8 data) -> void {
    if(writedelay) return;
    writedelay = 2;

    if(data & 0x80) {
      shiftaddr = 0;
      prgSize = 1;
      prgMode = 1;
    } else {
      shiftdata = ((data & 1) << 4) | (shiftdata >> 1);
      if(++shiftaddr == 5) {
        shiftaddr = 0;
        switch((addr >> 13) & 3) {
        case 0:
          chrMode = (shiftdata & 0x10);
          prgSize = (shiftdata & 0x08);
          prgMode = (shiftdata & 0x04);
          mirror = (shiftdata & 0x03);
          break;

        case 1:
          chrBank[0] = (shiftdata & 0x1f);
          break;

        case 2:
          chrBank[1] = (shiftdata & 0x1f);
          break;

        case 3:
          ramDisable = ((shiftdata & 0x10) && revision != Revision::MMC1 && revision != Revision::MMC1A);
          prgBank = (shiftdata & 0x0f);
          break;
        }
      }
    }
  }

  auto power(bool reset) -> void {
    writedelay = 0;
    shiftaddr = 0;
    shiftdata = 0;

    chrMode = 0;
    prgSize = 1;
    prgMode = 1;
    mirror = 0;
    chrBank[0] = 0;
    chrBank[1] = 1;
    ramDisable = revision == Revision::MMC1C;
    prgBank = 0;
  }

  auto serialize(serializer& s) -> void {
    s.integer(writedelay);
    s.integer(shiftaddr);
    s.integer(shiftdata);

    s.integer(chrMode);
    s.integer(prgSize);
    s.integer(prgMode);
    s.integer(mirror);
    s.array(chrBank);
    s.integer(ramDisable);
    s.integer(prgBank);
  }

  enum class Revision : uint {
    MMC1,
    MMC1A,
    MMC1B1,
    MMC1B2,
    MMC1B3,
    MMC1C,
  } revision;

  uint writedelay;
  uint shiftaddr;
  uint shiftdata;

  bool chrMode;
  bool prgSize;  //0 = 32K, 1 = 16K
  bool prgMode;
  uint2 mirror;  //0 = first, 1 = second, 2 = vertical, 3 = horizontal
  uint5 chrBank[2];
  bool ramDisable;
  uint4 prgBank;
};
