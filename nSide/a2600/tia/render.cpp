auto TIA::frame() -> void {
}

auto TIA::scanline() -> void {
  if(io.vcounter == 0) frame();

  if(io.vcounter == 228 + (Region::NTSC() ? 19 : 44)) {
    //dirty hack to prevent controls for hardware switches from being polled
    //19912 (262 * 228 / 3) times as fast as joystick/paddle controls and other emulators' controls
    pia.updateIO();

    scheduler.exit(Scheduler::Event::Frame);
  }
}

auto TIA::runVideo() -> void {
  for(bool i : range(2)) {
    if(!missile[i].reset) continue;
    uint offset = player[i].numberSize == 5 ? 6 : player[i].numberSize == 7 ? 10 : 3;
    missile[i].position = (player[i].position + offset) % 160;
  }

  uint offsetY = Region::NTSC() ? 19 : 37;
  if(hblank() || io.vcounter < offsetY) return;

  uint x = io.hcounter - 68;
  uint y = io.vcounter - offsetY;
  if(y >= 228) return;

  uint7 pixel = 0;

  if(!io.vblank) {
    pixel = io.backgroundColor;
    uint size;

    bool p0 = false;
    bool p1 = false;
    bool m0 = false;
    bool m1 = false;
    bool bl = false;
    bool pf = false;

    for(bool i : range(2)) {
      uint scale;
      if(player[i].numberSize != 5 && player[i].numberSize != 7) {
        scale = 0;
      } else if(player[i].numberSize == 5) {
        scale = 1;
      } else if(player[i].numberSize == 7) {
        scale = 2;
      }
      size = 8 << scale;
      if(player[i].position >= x - (size - 1) && player[i].position <= x) {
        uint bit = (x - player[i].position) >> scale;
        if(!player[i].reflect) bit = 7 - bit;
        if(i == 0) p0 = player[i].graphic.bit(bit);
        if(i == 1) p1 = player[i].graphic.bit(bit);
      }
    }

    for(bool i : range(2)) {
      if(missile[i].enable && !missile[i].reset) {
        size = 1 << missile[i].size;
        if(missile[i].position >= x - (size - 1) && missile[i].position <= x) {
          if(i == 0) m0 = true;
          if(i == 1) m1 = true;
        }
      }
    }

    if(ball.enable) {
      size = 1 << ball.size;
      if(ball.position >= x - (size - 1) && ball.position <= x) bl = true;
    }

    uint playfieldX = x >> 2;
    if(playfieldX >= 20) playfieldX = !playfield.reflect ? playfieldX - 20 : 39 - playfieldX;
    if(playfieldX >=  0 && playfieldX <  4) pf = playfield.graphic0.bit(playfieldX -  0);
    if(playfieldX >=  4 && playfieldX < 12) pf = playfield.graphic1.bit(11 - playfieldX);
    if(playfieldX >= 12 && playfieldX < 20) pf = playfield.graphic2.bit(playfieldX - 12);

    if(m0 && p1) collision.m0p1 = true;
    if(m0 && p0) collision.m0p0 = true;
    if(m1 && p0) collision.m1p0 = true;
    if(m1 && p1) collision.m1p1 = true;
    if(p0 && pf) collision.p0pf = true;
    if(p0 && bl) collision.p0bl = true;
    if(p1 && pf) collision.p1pf = true;
    if(p1 && bl) collision.p1bl = true;
    if(m0 && pf) collision.m0pf = true;
    if(m0 && bl) collision.m0bl = true;
    if(m1 && pf) collision.m1pf = true;
    if(m1 && bl) collision.m1bl = true;
    if(bl && pf) collision.blpf = true;
    if(p0 && p1) collision.p0p1 = true;
    if(m0 && m1) collision.m0m1 = true;

    if(playfield.score && pf) {
      if(x <  80) p0 = m0 = true;
      if(x >= 80) p1 = m1 = true;
    }
    if(io.playfieldBallPriority && (bl || pf)) pixel = io.playfieldBallColor;
    else if(p0 || m0) pixel = io.playerMissile0Color;
    else if(p1 || m1) pixel = io.playerMissile1Color;
    else if(bl || pf) pixel = io.playfieldBallColor;
  }

  output[y * 160 + x] = pixel;
}

