auto PlayChoice10::VideoCircuit::Enter() -> void {
  while(true) scheduler.synchronize(), playchoice10.videoCircuit.main();
}

auto PlayChoice10::VideoCircuit::main() -> void {
  for(uint y : range(256)) {
    if(y == 240 && playchoice10.z80NMI) playchoice10.pc10cpu.setNMI(1);
    for(uint x : range(327)) {
      run(x, y);
      step(4);
    }
  }
}

auto PlayChoice10::VideoCircuit::step(uint clocks) -> void {
  Thread::step(clocks);
  synchronize(playchoice10.pc10cpu);
}

auto PlayChoice10::VideoCircuit::refresh() -> void {
  if(playchoice10.screenConfig == ScreenConfig::Dual) {
    Emulator::video.refreshRegion(buffer, 256 * sizeof(uint32), 0, 0, 256, 224, 1 << 9);
  } else if(playchoice10.screenConfig == ScreenConfig::Single && playchoice10.display == 0) {
    Emulator::video.refreshRegion(buffer, 256 * sizeof(uint32), 0, 8, 256, 224, 1 << 9);
  }
}

auto PlayChoice10::VideoCircuit::power(bool reset) -> void {
  create(VideoCircuit::Enter, 20'160'000.0);
  if(!reset) memory::fill(&vram, sizeof(vram));
}

auto PlayChoice10::VideoCircuit::writeVRAM(uint16 addr, uint8 data) -> void {
  if(playchoice10.vramAccess) return;
  vram[addr & 0x07ff] = data;
}

auto PlayChoice10::VideoCircuit::run(uint x, uint y) -> void {
  if(y < 16 || y >= 240 || x >= 256) return;
  if(!playchoice10.vramAccess) {
    buffer[(y - 16) * 256 + x] = 0x000;
    return;
  }

  uint5 tileY = y >> 3;
  uint5 tileX = x >> 3;
  uint16 addr = (tileY * 32 + tileX) << 1;

  uint16 data;
  data.byte(0) = vram[addr | 0];
  data.byte(1) = vram[addr | 1];

  uint tileID = data.bits(0,10);
  uint32 color = data.bits(11,15) << 3;

  uint pixelY = y & 7;
  uint pixelX = x & 7;

  uint8 byte;
  for(uint plane : range(3)) {
    byte = chrrom[(plane << 13) + (tileID << 3) + pixelY];
    color.bit(plane) = byte.bit(7 - pixelX);
  }

  buffer[(y - 16) * 256 + x] = color;
}
