auto PPUcounter::serialize(serializer& s) -> void {
  s.integer(status.interlace);
  s.integer(status.field);
  s.integer(status.vcounter);
  s.integer(status.hcounter);

  s.array(history.field);
  s.array(history.vcounter);
  s.array(history.hcounter);
  s.integer(history.index);
}

auto PPU::serialize(serializer& s) -> void {
  Thread::serialize(s);
  PPUcounter::serialize(s);

  s.integer(vram.mask);
  s.array(vram.data, vram.mask + 1);

  s.integer(ppu1.version);
  s.integer(ppu1.mdr);

  s.integer(ppu2.version);
  s.integer(ppu2.mdr);

  s.integer(display.interlace);
  s.integer(display.overscan);

  s.integer(latch.vram);
  s.integer(latch.oam);
  s.integer(latch.cgram);
  s.integer(latch.bgofsPPU1);
  s.integer(latch.bgofsPPU2);
  s.integer(latch.mode7);
  s.integer(latch.counters);
  s.integer(latch.hcounter);
  s.integer(latch.vcounter);

  s.integer(latch.oamAddress);
  s.integer(latch.cgramAddress);

  s.integer(io.displayDisable);
  s.integer(io.displayBrightness);

  s.integer(io.oamBaseAddress);
  s.integer(io.oamAddress);
  s.integer(io.oamPriority);

  s.integer(io.bgPriority);
  s.integer(io.bgMode);

  s.integer(io.hoffsetMode7);
  s.integer(io.voffsetMode7);

  s.integer(io.vramIncrementMode);
  s.integer(io.vramMapping);
  s.integer(io.vramIncrementSize);

  s.integer(io.vramAddress);

  s.integer(io.repeatMode7);
  s.integer(io.vflipMode7);
  s.integer(io.hflipMode7);

  s.integer(io.m7a);
  s.integer(io.m7b);
  s.integer(io.m7c);
  s.integer(io.m7d);
  s.integer(io.m7x);
  s.integer(io.m7y);

  s.integer(io.cgramAddress);
  s.integer(io.cgramAddressLatch);

  s.integer(io.extbg);
  s.integer(io.pseudoHires);
  s.integer(io.overscan);
  s.integer(io.interlace);

  s.integer(io.hcounter);
  s.integer(io.vcounter);

  //better to just take a small speed hit than store all of the tiledata cache ...
  cache.flush();

  bg1.serialize(s);
  bg2.serialize(s);
  bg3.serialize(s);
  bg4.serialize(s);
  obj.serialize(s);
  window.serialize(s);
  screen.serialize(s);
}

auto PPU::Background::serialize(serializer& s) -> void {
  s.integer(io.tiledataAddress);
  s.integer(io.screenAddress);
  s.integer(io.screenSize);
  s.integer(io.tileSize);
  s.integer(io.mode);
  s.array(io.priority);
  s.integer(io.aboveEnable);
  s.integer(io.belowEnable);
  s.integer(io.hoffset);
  s.integer(io.voffset);

  s.integer(mosaic.size);
  s.integer(mosaic.enable);
  s.integer(mosaic.vcounter);
  s.integer(mosaic.hcounter);
  s.integer(mosaic.voffset);
  s.integer(mosaic.hoffset);

  //s.integer(x);
  s.integer(y);
  //s.integer(tileCounter);
  s.integer(tile);
  s.integer(priority);
  s.integer(paletteNumber);
  s.integer(paletteIndex);
  //s.array(data);
}

auto PPU::Object::serialize(serializer& s) -> void {
  for(auto& object : oam.object) {
    s.integer(object.x);
    s.integer(object.y);
    s.integer(object.character);
    s.integer(object.nameselect);
    s.integer(object.vflip);
    s.integer(object.hflip);
    s.integer(object.priority);
    s.integer(object.palette);
    s.integer(object.size);
  }

  s.integer(io.aboveEnable);
  s.integer(io.belowEnable);
  s.integer(io.interlace);

  s.integer(io.baseSize);
  s.integer(io.nameselect);
  s.integer(io.tiledataAddress);
  s.integer(io.firstSprite);

  s.array(io.priority);

  s.integer(io.timeOver);
  s.integer(io.rangeOver);

  s.array(itemList, 32);

  for(uint n : range(34)) {
    s.integer(tileList[n].x);
    s.integer(tileList[n].y);
    s.integer(tileList[n].priority);
    s.integer(tileList[n].palette);
    s.integer(tileList[n].tile);
    s.integer(tileList[n].hflip);
  }

  s.integer(t.itemCount);
  s.integer(t.tileCount);

  s.array(output.palette, 256);
  s.array(output.priority, 256);

  s.integer(cache.baseSize);
  s.integer(cache.nameselect);
  s.integer(cache.tiledataAddress);
}

auto PPU::Window::serialize(serializer& s) -> void {
  s.integer(io.bg1.oneEnable);
  s.integer(io.bg1.oneInvert);
  s.integer(io.bg1.twoEnable);
  s.integer(io.bg1.twoInvert);
  s.integer(io.bg1.mask);
  s.integer(io.bg1.aboveEnable);
  s.integer(io.bg1.belowEnable);
  
  s.integer(io.bg2.oneEnable);
  s.integer(io.bg2.oneInvert);
  s.integer(io.bg2.twoEnable);
  s.integer(io.bg2.twoInvert);
  s.integer(io.bg2.mask);
  s.integer(io.bg2.aboveEnable);
  s.integer(io.bg2.belowEnable);

  s.integer(io.bg3.oneEnable);
  s.integer(io.bg3.oneInvert);
  s.integer(io.bg3.twoEnable);
  s.integer(io.bg3.twoInvert);
  s.integer(io.bg3.mask);
  s.integer(io.bg3.aboveEnable);
  s.integer(io.bg3.belowEnable);

  s.integer(io.bg4.oneEnable);
  s.integer(io.bg4.oneInvert);
  s.integer(io.bg4.twoEnable);
  s.integer(io.bg4.twoInvert);
  s.integer(io.bg4.mask);
  s.integer(io.bg4.aboveEnable);
  s.integer(io.bg4.belowEnable);

  s.integer(io.obj.oneEnable);
  s.integer(io.obj.oneInvert);
  s.integer(io.obj.twoEnable);
  s.integer(io.obj.twoInvert);
  s.integer(io.obj.mask);
  s.integer(io.obj.aboveEnable);
  s.integer(io.obj.belowEnable);

  s.integer(io.col.oneEnable);
  s.integer(io.col.oneInvert);
  s.integer(io.col.twoEnable);
  s.integer(io.col.twoInvert);
  s.integer(io.col.mask);
  s.integer(io.col.aboveMask);
  s.integer(io.col.belowMask);

  s.integer(io.oneLeft);
  s.integer(io.oneRight);
  s.integer(io.twoLeft);
  s.integer(io.twoRight);

  for(uint n : range(6)) {
    s.array(cache[n].above, 256);
    s.array(cache[n].below, 256);
  }
}

auto PPU::Screen::serialize(serializer& s) -> void {
  s.array(cgram);

  s.integer(io.blendMode);
  s.integer(io.directColor);

  s.integer(io.colorMode);
  s.integer(io.colorHalve);
  s.integer(io.bg1.colorEnable);
  s.integer(io.bg2.colorEnable);
  s.integer(io.bg3.colorEnable);
  s.integer(io.bg4.colorEnable);
  s.integer(io.obj.colorEnable);
  s.integer(io.back.colorEnable);

  s.integer(io.color);

  s.integer(math.colorHalve);

  for(uint n : range(256)) {
    s.integer(output.above[n].color);
    s.integer(output.above[n].priority);
    s.integer(output.above[n].source);
    s.integer(output.above[n].exemption);
    s.integer(output.below[n].color);
    s.integer(output.below[n].priority);
    s.integer(output.below[n].source);
    s.integer(output.below[n].exemption);
  }
}
