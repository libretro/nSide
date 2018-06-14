auto PPU::readCIRAM(uint12 addr) -> uint8 {
  if(!Model::VSSystem()) addr &= 0x7ff;
  return ciram[addr];
}

auto PPU::writeCIRAM(uint12 addr, uint8 data) -> void {
  if(!Model::VSSystem()) addr &= 0x7ff;
  ciram[addr] = data;
}

auto PPU::readCGRAM(uint5 addr) -> uint8 {
  if((addr & 0x13) == 0x10) addr &= ~0x10;
  uint8 data = cgram[addr];
  if(io.grayscale) data &= 0x30;
  return data;
}

auto PPU::writeCGRAM(uint5 addr, uint8 data) -> void {
  if((addr & 0x13) == 0x10) addr &= ~0x10;
  cgram[addr] = data;
}

auto PPU::readIO(uint16 addr, uint8 data) -> uint8 {
  switch(addr.bits(0,2)) {

  case 0: case 1: case 3:
  case 5: case 6: {
    return io.mdr;
  }

  //PPUSTATUS
  case 2: {
    data = 0x00;
    switch(version) {
    default:
      data |= io.mdr.bits(0,4);
      data |= io.spriteOverflow << 5;
      break;
    case Version::RC2C05_01:
    case Version::RC2C05_04: data |= 0x1b; break;
    case Version::RC2C05_02: data |= 0x3d; break;
    case Version::RC2C05_03: data |= 0x1c; break;
    }
    data |= io.spriteZeroHit << 6;
    data |= io.nmiFlag << 7;
    io.v.latch = 0;
    io.nmiHold = 0;
    cpu.nmiLine(io.nmiFlag = 0);
    return data;
  }

  //OAMDATA
  case 4: {
    switch(version) {
    case Version::RP2C02C:
    case Version::RP2C02E:
      return io.mdr;
    default:
      io.mdr = oam[io.oamAddress];
      for(uint i = 0; i < 8; i++) io.mdrDecay[i] = 3221591;
      break;
    }
    return io.mdr;
  }

  //PPUDATA
  case 7: {
    if(enable() && (vcounter() <= 240 || vcounter() == vlines() - 1)) return 0x00;

    addr = (uint14)io.v.address;
    if(addr <= 0x3eff) {
      io.mdr = io.busData;
    } else if(addr <= 0x3fff) {
      io.mdr = (io.mdr & 0xc0) | readCGRAM(addr);
    }
    io.busData = bus.readCHR(io.chrAddressBus = addr, io.mdr);
    io.v.address += io.vramIncrement;
    io.chrAddressBus = io.v.address;
    return io.mdr;
  }

  }

  return data;
}

auto PPU::writeIO(uint16 addr, uint8 data) -> void {
  io.mdr = data;
  // Decay rate can vary depending on the system and temperature.
  // Value used here is PPU's NTSC clock rate * 0.6 rounded to nearest integer.
  for(uint i = 0; i < 8; i++) io.mdrDecay[i] = 3221591;

  switch(version) {
  case Version::RC2C05_01:
  case Version::RC2C05_02:
  case Version::RC2C05_03:
  case Version::RC2C05_04:
  case Version::RC2C05_05: if(addr.bits(1,2) == 0) addr ^= 1; break;
  }

  switch(addr.bits(0,2)) {

  //PPUCTRL
  case 0: {
    io.t.nametable   = data.bits(0,1);
    io.vramIncrement = data.bit (2) ? 32 : 1;
    io.objAddress    = data.bit (3) ? 0x1000 : 0x0000;
    io.bgAddress     = data.bit (4) ? 0x1000 : 0x0000;
    io.spriteHeight  = data.bit (5) ? 16 : 8;
    io.masterSelect  = data.bit (6);
    io.nmiEnable     = data.bit (7);
    cpu.nmiLine(io.nmiEnable && io.nmiHold && io.nmiFlag);
    return;
  }

  //PPUMASK
  case 1: {
    io.grayscale     = data.bit (0);
    io.bgEdgeEnable  = data.bit (1);
    io.objEdgeEnable = data.bit (2);
    io.bgEnable      = data.bit (3);
    io.objEnable     = data.bit (4);
    io.emphasis      = data.bits(5,7);
    return;
  }

  //PPUSTATUS
  case 2: {
    return;
  }

  //OAMADDR
  case 3: {
    if(ntsc()) {
      //below corruption code only applies for preferred CPU-PPU alignment.
      //on an actual Famicom/NES, waiting a while after writing to OAM will
      //make this corruption happen because the OAM will have decayed at the
      //spot being written to.
      memory::copy(oam + ((addr & 0xf800) >> 8), 8, oam + (io.oamAddress & 0xf8), 8);
    }
    io.oamAddress = data;
    return;
  }

  //OAMDATA
  case 4: {
    if(io.oamAddress.bits(0,1) == 2) data.bits(2,4) = 0;  //clear non-existent bits (always read back as 0)
    oam[io.oamAddress++] = data;
    return;
  }

  //PPUSCROLL
  case 5: {
    if(!io.v.latch) {
      io.v.fineX = data.bits(0,2);
      io.t.tileX = data.bits(3,7);
    } else {
      io.t.fineY = data.bits(0,2);
      io.t.tileY = data.bits(3,7);
    }
    io.v.latch ^= 1;
    return;
  }

  //PPUADDR
  case 6: {
    if(!io.v.latch) {
      io.t.addressHi = data.bits(0,5);
    } else {
      io.t.addressLo = data.bits(0,7);
      io.chrAddressBus = io.v.address = io.t.address;
    }
    io.v.latch ^= 1;
    return;
  }

  //PPUDATA
  case 7: {
    if(enable() && (vcounter() <= 240 || vcounter() == vlines() - 1)) return;

    addr = (uint14)io.v.address;
    if(addr <= 0x3eff) {
      bus.writeCHR(io.chrAddressBus = addr, data);
    } else if(addr <= 0x3fff) {
      writeCGRAM(addr, data);
    }
    io.v.address += io.vramIncrement;
    io.chrAddressBus = io.v.address;
    return;
  }

  }
}
