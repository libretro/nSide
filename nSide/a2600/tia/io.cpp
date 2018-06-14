auto TIA::readIO(uint6 addr, uint8 data) -> uint8 {
  switch(addr & 0x0f) {

  //CXM0P
  case 0x0: {
    data.bit(6) = collision.m0p0;
    data.bit(7) = collision.m0p1;
    break;
  }

  //CXM1P
  case 0x1: {
    data.bit(6) = collision.m1p1;
    data.bit(7) = collision.m1p0;
    break;
  }

  //CXP0FB
  case 0x2: {
    data.bit(6) = collision.p0bl;
    data.bit(7) = collision.p0pf;
    break;
  }

  //CXP1FB
  case 0x3: {
    data.bit(6) = collision.p1bl;
    data.bit(7) = collision.p1pf;
    break;
  }

  //CXM0FB
  case 0x4: {
    data.bit(6) = collision.m0bl;
    data.bit(7) = collision.m0pf;
    break;
  }

  //CXM1FB
  case 0x5: {
    data.bit(6) = collision.m1bl;
    data.bit(7) = collision.m1pf;
    break;
  }

  //CXBLPF
  case 0x6: {
    data.bit(7) = collision.blpf;
    break;
  }

  //CXPPMM
  case 0x7: {
    data.bit(6) = collision.m0m1;
    data.bit(7) = collision.p0p1;
    break;
  }

  //INPT0
  case 0x8: {
    data.bit(7) = controllerPort1.device->pot0();
    break;
  }

  //INPT1
  case 0x9: {
    data.bit(7) = controllerPort1.device->pot1();
    break;
  }

  //INPT2
  case 0xa: {
    data.bit(7) = controllerPort2.device->pot0();
    break;
  }

  //INPT3
  case 0xb: {
    data.bit(7) = controllerPort2.device->pot1();
    break;
  }

  //INPT4
  case 0xc: {
    data.bit(7) = controllerPort1.device->fire();
    break;
  }

  //INPT5
  case 0xd: {
    data.bit(7) = controllerPort2.device->fire();
    break;
  }

  }

  return data;
}

auto TIA::writeIO(uint6 addr, uint8 data) -> void {
  switch(addr) {

  //VSYNC
  case 0x00: {
    if(io.vsync && !data.bit(1)) io.vcounter = 0;
    io.vsync = data.bit(1);
    return;
  }

  //VBLANK
  case 0x01: {
    io.vblank = data.bit(1);
    return;
  }

  //WSYNC
  case 0x02: {
    cpu.rdyLine(0);
    return;
  }

  //RSYNC
  case 0x03: {
    io.hcounter = 0;
    return;
  }

  //NUSIZ0
  case 0x04: {
    player[0].numberSize = data.bits(0,2);
    //missile size is 2 ^ data, or {1, 2, 4, 8}[data].
    missile[0].size      = data.bits(4,5);
    return;
  }

  //NUSIZ1
  case 0x05: {
    player[1].numberSize = data.bits(0,2);
    missile[1].size      = data.bits(4,5);
    return;
  }

  //COLUP0
  case 0x06: {
    io.playerMissile0Color = data >> 1;
    return;
  }

  //COLUP1
  case 0x07: {
    io.playerMissile1Color = data >> 1;
    return;
  }

  //COLUPF
  case 0x08: {
    io.playfieldBallColor = data >> 1;
    return;
  }

  //COLUBK
  case 0x09: {
    io.backgroundColor = data >> 1;
    return;
  }

  //CTRLPF
  case 0x0a: {
    playfield.reflect        = data.bit (  0);
    playfield.score          = data.bit (  1);
    io.playfieldBallPriority = data.bit (  2);
    ball.size                = data.bits(4,5);
    return;
  }

  //REFP0
  case 0x0b: {
    player[0].reflect = data.bit(3);
    return;
  }

  //REFP1
  case 0x0c: {
    player[1].reflect = data.bit(3);
    return;
  }

  //PF0
  case 0x0d: {
    playfield.graphic0 = data.bits(4,7);
    return;
  }

  //PF1
  case 0x0e: {
    playfield.graphic1 = data.bits(0,7);
    return;
  }

  //PF2
  case 0x0f: {
    playfield.graphic2 = data.bits(0,7);
    return;
  }

  //RESP0
  case 0x10: {
    player[0].position = (hblank() ? 2 : io.hcounter - 68 + 4) % 160;
    return;
  }

  //RESP1
  case 0x11: {
    player[1].position = (hblank() ? 2 : io.hcounter - 68 + 4) % 160;
    return;
  }

  //RESM0
  case 0x12: {
    missile[0].position = (hblank() ? 1 : io.hcounter - 68 + 3) % 160;
    return;
  }

  //RESM1
  case 0x13: {
    missile[1].position = (hblank() ? 1 : io.hcounter - 68 + 3) % 160;
    return;
  }

  //RESBL
  case 0x14: {
    ball.position = (hblank() ? 1 : io.hcounter - 68 + 3) % 160;
    return;
  }

  //AUDC0
  case 0x15: {
    audio.channel[0].control = data.bits(0,3);
    return;
  }

  //AUDC1
  case 0x16: {
    audio.channel[1].control = data.bits(0,3);
    return;
  }

  //AUDF0
  case 0x17: {
    audio.channel[0].frequency = data.bits(0,4);
    return;
  }

  //AUDF1
  case 0x18: {
    audio.channel[1].frequency = data.bits(0,4);
    return;
  }

  //AUDV0
  case 0x19: {
    audio.channel[0].volume = data.bits(0,3);
    return;
  }

  //AUDV1
  case 0x1a: {
    audio.channel[1].volume = data.bits(0,3);
    return;
  }

  //GRP0
  case 0x1b: {
    player[0].graphic = data;
    return;
  }

  //GRP1
  case 0x1c: {
    player[1].graphic = data;
    return;
  }

  //ENAM0
  case 0x1d: {
    missile[0].enable = data.bit(1);
    return;
  }

  //ENAM1
  case 0x1e: {
    missile[1].enable = data.bit(1);
    return;
  }

  //ENABL
  case 0x1f: {
    ball.enable = data.bit(1);
    return;
  }

  //HMP0
  case 0x20: {
    player[0].motion = data.bits(4,7);
    return;
  }

  //HMP1
  case 0x21: {
    player[1].motion = data.bits(4,7);
    return;
  }

  //HMM0
  case 0x22: {
    missile[0].motion = data.bits(4,7);
    return;
  }

  //HMM1
  case 0x23: {
    missile[1].motion = data.bits(4,7);
    return;
  }

  //HMBL
  case 0x24: {
    ball.motion = data.bits(4,7);
    return;
  }

  //VDELP0
  case 0x25: {
    return;
  }

  //VDELP1
  case 0x26: {
    return;
  }

  //VDELBL
  case 0x27: {
    return;
  }

  //RESMP0
  case 0x28: {
    missile[0].reset = data.bit(1);
    return;
  }

  //RESMP1
  case 0x29: {
    missile[1].reset = data.bit(1);
    return;
  }

  //HMOVE
  case 0x2a: {
    for(auto& p : player)  p.position = (p.position - p.motion + 160) % 160;
    for(auto& m : missile) m.position = (m.position - m.motion + 160) % 160;
    ball.position = (ball.position - ball.motion + 160) % 160;
    return;
  }

  //HMCLR
  case 0x2b: {
    player[0].motion = 0;
    player[1].motion = 0;
    missile[0].motion = 0;
    missile[1].motion = 0;
    ball.motion = 0;
    return;
  }

  //CXCLR
  case 0x2c: {
    collision.m0p1 = false;
    collision.m0p0 = false;
    collision.m1p0 = false;
    collision.m1p1 = false;
    collision.p0pf = false;
    collision.p0bl = false;
    collision.p1pf = false;
    collision.p1bl = false;
    collision.m0pf = false;
    collision.m0bl = false;
    collision.m1pf = false;
    collision.m1bl = false;
    collision.blpf = false;
    collision.p0p1 = false;
    collision.m0m1 = false;
    return;
  }

  }

}
