struct KonamiVRC4 : Board {
  KonamiVRC4(Markup::Node& boardNode) : Board(boardNode), vrc4(*this) {
    settings.pinout.a0 = 1 << boardNode["chip/pinout/a0"].natural();
    settings.pinout.a1 = 1 << boardNode["chip/pinout/a1"].natural();
  }

  auto main() -> void {
    return vrc4.main();
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    if(addr < 0x6000) return data;
    if(addr < 0x8000) return read(prgram, addr);
    return read(prgrom, vrc4.prgAddress(addr));
  }

  auto writePRG(uint addr, uint8 data) -> void {
    if(addr < 0x6000) return;
    if(addr < 0x8000) return write(prgram, addr, data);

    bool a0 = (addr & settings.pinout.a0);
    bool a1 = (addr & settings.pinout.a1);
    addr &= 0xff00;
    addr |= (a1 << 1) | (a0 << 0);
    return vrc4.regWrite(addr, data);
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    if(addr & 0x2000) return ppu.readCIRAM(vrc4.ciramAddress(addr));
    return Board::readCHR(vrc4.chrAddress(addr), data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    if(addr & 0x2000) return ppu.writeCIRAM(vrc4.ciramAddress(addr), data);
    return Board::writeCHR(vrc4.chrAddress(addr), data);
  }

  auto power(bool reset) -> void {
    vrc4.power(reset);
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    vrc4.serialize(s);
  }

  struct Settings {
    struct Pinout {
      uint a0;
      uint a1;
    } pinout;
  } settings;

  VRC4 vrc4;
};
