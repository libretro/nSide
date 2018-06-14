BeamGun::BeamGun() {
  create(Expansion::Enter, system.frequency());
  sprite = Emulator::video.createSprite(16, 16);
  sprite->setPixels(Resource::Sprite::CrosshairGreenSmall);

  latched = 0;
  counter = 0;

  //center cursor onscreen
  x = 256 / 2;
  y = 240 / 2;

  light       = false;
  trigger     = false;
  lighttime   = 0;
  triggertime = 0;
  offscreen   = false;

  triggerlock = false;

  prev = 0;
}

BeamGun::~BeamGun() {
  Emulator::video.removeSprite(sprite);
}

auto BeamGun::main() -> void {
  uint next = ppu.vcounter() * 341 + ppu.hcounter();

  if(lighttime > 0) {
    lighttime -= 1;
  }

  if(!offscreen) {
    uint target = y * 341 + x + 8;
    if(next >= target && prev < target) {
      //CRT raster detected
      //light remains in the gun for 10-25 scanlines
      if(readLight()) lighttime = 341 * 16;
    }
  }

  if(next < prev) {
    if(triggertime > 0) triggertime -= 1;
    //Vcounter wrapped back to zero; update cursor coordinates for start of new frame
    int nx = platform->inputPoll(ID::Port::Expansion, ID::Device::BeamGun, X);
    int ny = platform->inputPoll(ID::Port::Expansion, ID::Device::BeamGun, Y);
    nx += x;
    ny += y;
    x = max(-16, min(256 + 16, nx));
    y = max(-16, min(240 + 16, ny));
    offscreen = (x < 0 || y < 0 || x >= 256 || y >= 240);
    sprite->setPosition(x - 8, y - 8);
    sprite->setVisible(true);
  }

  prev = next;
  step(3);
  synchronize(cpu);
}

auto BeamGun::data1() -> bool {
  return 0;
}

auto BeamGun::data2() -> uint5 {
  if(!Model::VSSystem()) {
    bool newtrigger = platform->inputPoll(ID::Port::Expansion, ID::Device::BeamGun, Trigger);
    if(newtrigger && !triggerlock) {
      triggertime = 3;
      triggerlock = true;
    } else if(!newtrigger) {
      triggerlock = false;
    }
    light = lighttime > 0;
    trigger = triggertime > 0;

    return (trigger << 4) | ((!light) << 3);
  } else {
    if(counter >= 8) return 1;
    if(latched == 1) return 0;

    switch(counter++) {
    case 0: return 0;
    case 1: return 0;
    case 2: return 0;
    case 3: return 0;
    case 4: return 1; // connected
    case 5: return 0;
    case 6: return light;
    case 7: return trigger;
    }
  }
}

auto BeamGun::readLight() -> bool {
  if(offscreen) return false;
  uint32 paletteIndex = ppu.output[y * 256 + x];
  uint color;
  switch(ppu.version) {
  default:
    return ((paletteIndex & 0x20) && ((paletteIndex & 0x0f) < 0x0d));
  case PPU::Version::RP2C04_0001:
    color = PPU::RP2C04_0001[paletteIndex & 63];
    break;
  case PPU::Version::RP2C04_0002:
    color = PPU::RP2C04_0002[paletteIndex & 63];
    break;
  case PPU::Version::RP2C04_0003:
    color = PPU::RP2C04_0003[paletteIndex & 63];
    break;
  case PPU::Version::RP2C04_0004:
    color = PPU::RP2C04_0004[paletteIndex & 63];
    break;
  }
  if((color & 0xf00) > 0x600) return true;
  if((color & 0x0f0) > 0x060) return true;
  if((color & 0x00f) > 0x006) return true;
  return false;
}

auto BeamGun::write(uint3 data) -> void {
  if(latched == data.bit(0)) return;
  latched = data.bit(0);
  if(Model::VSSystem() && latched == 0) {
    counter = 0;
    trigger = platform->inputPoll(ID::Port::Expansion, ID::Device::BeamGun, Trigger);
    light = lighttime > 0;
  }
}
