auto PPU::Background::beginMode7() -> void {
  latch.hoffset = ppu.io.hoffsetMode7;
  latch.voffset = ppu.io.voffsetMode7;
}

//higan mode7 renderer
//
//base algorithm written by anomie
//higan implementation written by byuu
//
//supports mode 7 + extbg + rotate + zoom + direct color + scrolling + m7sel + windowing + mosaic
//interlace and pseudo-hires support are automatic via main rendering routine

auto PPU::Background::renderMode7() -> void {
  int a = (int16)m7cache.a;
  int b = (int16)m7cache.b;
  int c = (int16)m7cache.c;
  int d = (int16)m7cache.d;

  int cx = (int13)m7cache.x;
  int cy = (int13)m7cache.y;
  int hoffset = (int13)latch.hoffset;
  int voffset = (int13)latch.voffset;

  ppu.window.buildTables(id);
  uint8* wt_above = ppu.window.cache[id].above;
  uint8* wt_below = ppu.window.cache[id].below;

  uint y = ppu.bg1.mosaic.voffset;  //BG2 vertical mosaic uses BG1 mosaic size

  if(ppu.io.vflipMode7) y = 255 - y;

  uint16* mtable = (uint16*)ppu.mosaicTableLo[mosaic.enable ? mosaic.size : uint4(0)];

  //13-bit sign extend
  //--s---vvvvvvvvvv -> ssssssvvvvvvvvvv
  #define clip(n) (n & 0x2000 ? n | ~1023 : n & 1023)
  int psx = ((a * clip(hoffset - cx)) & ~63) + ((b * clip(voffset - cy)) & ~63) + ((b * y) & ~63) + (cx << 8);
  int psy = ((c * clip(hoffset - cx)) & ~63) + ((d * clip(voffset - cy)) & ~63) + ((d * y) & ~63) + (cy << 8);
  #undef clip

  int px;
  int py;

  uint tile;
  uint palette;
  uint priority;
  int _x;
  for(int x : range(256)) {
    px = psx + (a * mtable[x]);
    py = psy + (c * mtable[x]);

    //mask floating-point bits (low 8 bits)
    px >>= 8;
    py >>= 8;

    switch(ppu.io.repeatMode7) {
    //screen repetition outside of screen area
    case 0:
    case 1:
      px &= 1023;
      py &= 1023;
      tile = ppu.vram[(py >> 3) * 128 + (px >> 3)].byte(0);
      palette = ppu.vram[(tile << 6) + ((py & 7) << 3) + (px & 7)].byte(1);
      break;

    //palette color 0 outside of screen area
    case 2: 
      if((px | py) & ~1023) {
        palette = 0;
      } else {
        px &= 1023;
        py &= 1023;
        tile = ppu.vram[(py >> 3) * 128 + (px >> 3)].byte(0);
        palette = ppu.vram[(tile << 6) + ((py & 7) << 3) + (px & 7)].byte(1);
      }
      break;

    //character 0 repetition outside of screen area
    case 3:
      if((px | py) & ~1023) {
        tile = 0;
      } else {
        px &= 1023;
        py &= 1023;
        tile = ppu.vram[(py >> 3) * 128 + (px >> 3)].byte(0);
      }
      palette = ppu.vram[(tile << 6) + ((py & 7) << 3) + (px & 7)].byte(1);
      break;

    }

    if(id == ID::BG1) {
      priority = io.priority[0];
    } else {
      priority = io.priority[(palette >> 7) ? 1 : 0];
      palette &= 0x7f;
    }

    if(!palette) continue;

    _x = !ppu.io.hflipMode7 ? (uint)x : 255 - x;

    uint16 color;
    if(ppu.screen.io.directColor && id == ID::BG1) {
      //direct color mode does not apply to bg2, as it is only 128 colors...
      color = directColor(palette, 0);
    } else {
      color = ppu.screen.cgram[palette];
    }

    if(io.aboveEnable && !wt_above[_x]) {
      ppu.screen.output.plotAbove(_x, color, priority, id, false);
    }
    if(io.belowEnable && !wt_below[_x]) {
      ppu.screen.output.plotBelow(_x, color, priority, id, false);
    }
  }
}
