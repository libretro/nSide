auto PPUcounter::tick(uint clocks) -> void {
  status.hcounter += clocks;
  if(status.hcounter == 337) status.skip = 341 - lineclocks();
  if(status.hcounter == 341 - status.skip) {
    status.hcounter = 0;
    status.skip = false;
    vcounterTick();
  }
}

//internal
auto PPUcounter::vcounterTick() -> void {
  status.vcounter++;

  if(((Region::NTSCJ() || Region::NTSCU()) && status.vcounter == 262)
  || ((Region::PAL  () || Region::Dendy()) && status.vcounter == 312)
  ) {
    status.vcounter = 0;
    status.field = !status.field;
  }
  if(scanline) scanline();
}

auto PPUcounter::field() const -> bool { return status.field; }
auto PPUcounter::vcounter() const -> uint16 { return status.vcounter; }
auto PPUcounter::hcounter() const -> uint16 { return status.hcounter; }

//one PPU dot = 4 CPU clocks (1/3 CPU cycle)

auto PPUcounter::lineclocks() const -> uint16 {
  if(ppuM.ntsc() && ppuM.enable() && vcounter() == 261 && field() == 1) return 340;
  return 341;
}

auto PPUcounter::reset() -> void {
  status.field    = 0;
  status.vcounter = 0;
  status.hcounter = 0;
  status.skip     = false;
}
