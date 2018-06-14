struct KonamiVRC3 : Board {
  KonamiVRC3(Markup::Node& boardNode) : Board(boardNode), vrc3(*this) {
    settings.mirror = boardNode["mirror/mode"].text() == "vertical" ? 1 : 0;
  }

  auto main() -> void {
    vrc3.main();
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if((addr & 0xe000) == 0x6000) return read(prgram, addr & 0x1fff);
    if(addr & 0x8000) return read(prgrom, vrc3.prgAddress(addr));
    return data;
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if((addr & 0xe000) == 0x6000) return write(prgram, addr & 0x1fff, data);
    if(addr & 0x8000) return vrc3.regWrite(addr, data);
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) {
      if(settings.mirror == 0) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.readCIRAM(addr);
    }
    return Board::readCHR(addr, data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) {
      if(settings.mirror == 0) addr = ((addr & 0x0800) >> 1) | (addr & 0x03ff);
      return ppu.writeCIRAM(addr, data);
    }
    return Board::writeCHR(addr, data);
  }

  auto power(bool reset) -> void {
    vrc3.power(reset);
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    vrc3.serialize(s);
  }

  struct Settings {
    bool mirror;  //0 = horizontal, 1 = vertical
  } settings;

  VRC3 vrc3;
};
