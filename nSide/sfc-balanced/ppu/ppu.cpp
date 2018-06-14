#include <sfc-balanced/sfc.hpp>

namespace SuperFamicom {

PPU ppu;
#include "io.cpp"
#include "cache/cache.cpp"
#include "background/background.cpp"
#include "object/object.cpp"
#include "window/window.cpp"
#include "screen/screen.cpp"
#include "serialization.cpp"

PPU::PPU() :
bg1(Background::ID::BG1),
bg2(Background::ID::BG2),
bg3(Background::ID::BG3),
bg4(Background::ID::BG4) {
  ppu1.version = 1;  //allowed values: 1
  ppu2.version = 3;  //allowed values: 1, 2, 3

  for(uint l : range(16)) {
    for(uint i : range(512)) mosaicTableLo[l][i] = (i / (l + 1)) * (l + 1);
    for(uint i : range(512)) mosaicTableHi[l][i] = (i / ((l << 1) + 2)) * ((l << 1) + 2);
  }
}

PPU::~PPU() {
}

auto PPU::step(uint clocks) -> void {
  tick(clocks);
  Thread::step(clocks);
  synchronize(cpu);
}

auto PPU::Enter() -> void {
  while(true) scheduler.synchronize(), ppu.main();
}

auto PPU::main() -> void {
  //H =    0 (initialize)
  scanline();
  step(10);

  //H =   10 (cache mode7 registers + OAM address reset)
  bg1.m7cache.a = io.m7a;
  bg1.m7cache.b = io.m7b;
  bg1.m7cache.c = io.m7c;
  bg1.m7cache.d = io.m7d;
  bg1.m7cache.x = io.m7x;
  bg1.m7cache.y = io.m7y;
  if(vcounter() == vdisp() && !io.displayDisable) obj.addressReset();
  step(18);

  //H =   28
  bg1.begin();
  bg2.begin();
  bg3.begin();
  bg4.begin();
  step(484);

  //H =  512 (render)
  if(vcounter() == 0) obj.renderRTO();
  if(vcounter() >= 1 && vcounter() < 240) renderScanline();
  step(640);

  //H = 1152 (cache OBSEL)
  obj.cache.baseSize = obj.io.baseSize;
  obj.cache.nameselect = obj.io.nameselect;
  obj.cache.tiledataAddress = obj.io.tiledataAddress;
  step(lineclocks() - 1152);  //seek to start of next scanline
}

auto PPU::renderScanline() -> void {
  bg1.scanline();
  bg2.scanline();
  bg3.scanline();
  bg4.scanline();
  if(io.displayDisable || vcounter() >= vdisp()) return screen.renderBlack();
  screen.scanline();
  bg1.render();
  bg2.render();
  bg3.render();
  bg4.render();
  obj.render();
  screen.render();
  obj.renderRTO();
}

auto PPU::load(Markup::Node node) -> bool {
  ppu1.version = max(1, min(1, node["ppu1/version"].natural()));
  ppu2.version = max(1, min(3, node["ppu2/version"].natural()));
  ppu.vram.mask = node["ppu1/ram/size"].natural() / sizeof(uint16) - 1;
  if(ppu.vram.mask != 0xffff) ppu.vram.mask = 0x7fff;
  return true;
}

auto PPU::power(bool reset) -> void {
  create(Enter, system.cpuFrequency());
  PPUcounter::reset();
  memory::fill(buffer, 512 * 512 * sizeof(uint32));

  output = buffer + 16 * 512;  //overscan offset

  function<auto (uint24, uint8) -> uint8> reader{&PPU::readIO, this};
  function<auto (uint24, uint8) -> void> writer{&PPU::writeIO, this};
  bus.map(reader, writer, "00-3f,80-bf:2100-213f");

  if(!reset) {
    random.array((uint8*)vram.data, sizeof(vram.data));
    cache.flush();
  }

  //open bus support
  ppu1.mdr = random.bias(0xff);
  ppu2.mdr = random.bias(0xff);

  latch.vram = random();
  latch.oam = random();
  latch.cgram = random();
  latch.bgofsPPU1 = random();
  latch.bgofsPPU2 = random();
  latch.mode7 = random();
  latch.counters = false;
  latch.hcounter = 0;
  latch.vcounter = 0;

  latch.oamAddress = 0x0000;
  latch.cgramAddress = 0x00;

  //$2100  INIDISP
  io.displayDisable = true;
  io.displayBrightness = 0;

  //$2101
  obj.cache.baseSize = 0;
  obj.cache.nameselect = 0;
  obj.cache.tiledataAddress = 0x0000;

  //$2102  OAMADDL
  //$2103  OAMADDH
  io.oamBaseAddress = random();
  io.oamAddress = random();
  io.oamPriority = random();

  //$2105  BGMODE
  io.bgPriority = false;
  io.bgMode = 0;

  //$210d  BG1HOFS
  io.hoffsetMode7 = random();

  //$210e  BG1VOFS
  io.voffsetMode7 = random();

  //$2115  VMAIN
  io.vramIncrementMode = random.bias(1);
  io.vramMapping = random();
  io.vramIncrementSize = 1;

  //$2116  VMADDL
  //$2117  VMADDH
  io.vramAddress = random();

  //$211a  M7SEL
  io.repeatMode7 = random();
  io.vflipMode7 = random();
  io.hflipMode7 = random();

  //$211b  M7A
  io.m7a = random();

  //$211c  M7B
  io.m7b = random();

  //$211d  M7C
  io.m7c = random();

  //$211e  M7D
  io.m7d = random();

  //$211f  M7X
  io.m7x = random();

  //$2120  M7Y
  io.m7y = random();

  //$2121  CGADD
  io.cgramAddress = random();
  io.cgramAddressLatch = random();

  //$2133  SETINI
  io.extbg = random();
  io.pseudoHires = random();
  io.overscan = false;
  io.interlace = false;

  //$213c  OPHCT
  io.hcounter = 0;

  //$213d  OPVCT
  io.vcounter = 0;

  bg1.power();
  bg2.power();
  bg3.power();
  bg4.power();
  obj.power();
  window.power();
  screen.power();

  frame();
}

auto PPU::scanline() -> void {
  if(vcounter() == 0) {
    frame();

    //RTO flag reset
    obj.io.timeOver  = false;
    obj.io.rangeOver = false;
  }

  if(vcounter() == 241) {
    scheduler.exit(Scheduler::Event::Frame);
  }
}

auto PPU::frame() -> void {
  if(field() == 0) display.interlace = io.interlace;
}

auto PPU::refresh() -> void {
  auto data = output;
  if(!overscan()) data -= 14 * 512;
  Emulator::video.refresh(data, 512 * sizeof(uint32), 512, 480);
}

}
