struct VRC1 : Chip {
  VRC1(Board& board) : Chip(board) {
  }

  auto prgAddress(uint addr) const -> uint {
    uint bank = 0x0f;
    if((addr & 0xe000) == 0x8000) bank = prgBank[0];
    if((addr & 0xe000) == 0xa000) bank = prgBank[1];
    if((addr & 0xe000) == 0xc000) bank = prgBank[2];
    return (bank * 0x2000) + (addr & 0x1fff);
  }

  auto chrAddress(uint addr) const -> uint {
    uint bank = chrBanklo[(bool)(addr & 0x1000)];
    bank |= chrBankhi[(bool)(addr & 0x1000)] << 4;
    return (bank * 0x1000) + (addr & 0x0fff);
  }

  auto ciramAddress(uint addr) const -> uint {
    switch(mirror) {
    case 0: return ((addr & 0x0400) >> 0) | (addr & 0x03ff);  //vertical mirroring
    case 1: return ((addr & 0x0800) >> 1) | (addr & 0x03ff);  //horizontal mirroring
    }
    throw;
  }

  auto regWrite(uint addr, uint8 data) -> void {
    switch(addr & 0xf000) {
    case 0x8000:
      prgBank[0] = data & 0x0f;
      break;

    case 0x9000:
      chrBankhi[1] = data & 0x04;
      chrBankhi[0] = data & 0x02;
      mirror = data & 0x01;
      break;

    case 0xa000:
      prgBank[1] = data & 0x0f;
      break;

    case 0xc000:
      prgBank[2] = data & 0x0f;
      break;

    case 0xe000:
      chrBanklo[0] = data & 0x0f;
      break;

    case 0xf000:
      chrBanklo[1] = data & 0x0f;
      break;
    }
  }

  auto power(bool reset) -> void {
    for(auto& n : prgBank) n = 0;
    for(auto& n : chrBanklo) n = 0;
    for(auto& n : chrBankhi) n = 0;
    mirror = 0;
  }

  auto serialize(serializer& s) -> void {
    for(auto& n : prgBank) s.integer(n);
    for(auto& n : chrBanklo) s.integer(n);
    for(auto& n : chrBankhi) s.integer(n);
    s.integer(mirror);
  }

  uint4 prgBank[3];
  uint4 chrBanklo[2];
  bool chrBankhi[2];
  bool mirror;
};
