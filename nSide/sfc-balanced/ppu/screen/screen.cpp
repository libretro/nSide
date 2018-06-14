auto PPU::Screen::power() -> void {
  random.array((uint8*)cgram, sizeof(cgram));
  for(auto& word : cgram) word &= 0x7fff;

  io.blendMode = random();
  io.directColor = random();
  io.colorMode = random();
  io.colorHalve = random();
  io.bg1.colorEnable = random();
  io.bg2.colorEnable = random();
  io.bg3.colorEnable = random();
  io.bg4.colorEnable = random();
  io.obj.colorEnable = random();
  io.back.colorEnable = random();
  io.color = random() & 0x7fff;
}

//color addition / subtraction
//thanks go to blargg for the optimized algorithms
auto PPU::Screen::blend(uint x, uint y) const -> uint16 {
  if(!io.colorMode) {
    if(!math.colorHalve) {
      uint sum = x + y;
      uint carry = (sum - ((x ^ y) & 0x0421)) & 0x8420;
      return (sum - carry) | (carry - (carry >> 5));
    } else {
      return (x + y - ((x ^ y) & 0x0421)) >> 1;
    }
  } else {
    uint diff = x - y + 0x8420;
    uint borrow = (diff - ((x ^ y) & 0x8420)) & 0x8420;
    if(!math.colorHalve) {
      return   (diff - borrow) & (borrow - (borrow >> 5));
    } else {
      return (((diff - borrow) & (borrow - (borrow >> 5))) & 0x7bde) >> 1;
    }
  }
}

//Mode 0: ->
//     1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12
//  BG4B, BG3B, OBJ0, BG4A, BG3A, OBJ1, BG2B, BG1B, OBJ2, BG2A, BG1A, OBJ3

//Mode 1 (pri=1): ->
//     1,    2,    3,    4,    5,    6,    7,    8,    9,   10
//  BG3B, OBJ0, OBJ1, BG2B, BG1B, OBJ2, BG2A, BG1A, OBJ3, BG3A
//
//Mode 1 (pri=0): ->
//     1,    2,    3,    4,    5,    6,    7,    8,    9,   10
//  BG3B, OBJ0, BG3A, OBJ1, BG2B, BG1B, OBJ2, BG2A, BG1A, OBJ3

//Mode 2: ->
//     1,    2,    3,    4,    5,    6,    7,    8
//  BG2B, OBJ0, BG1B, OBJ1, BG2A, OBJ2, BG1A, OBJ3

//Mode 3: ->
//     1,    2,    3,    4,    5,    6,    7,    8
//  BG2B, OBJ0, BG1B, OBJ1, BG2A, OBJ2, BG1A, OBJ3

//Mode 4: ->
//     1,    2,    3,    4,    5,    6,    7,    8
//  BG2B, OBJ0, BG1B, OBJ1, BG2A, OBJ2, BG1A, OBJ3

//Mode 5: ->
//     1,    2,    3,    4,    5,    6,    7,    8
//  BG2B, OBJ0, BG1B, OBJ1, BG2A, OBJ2, BG1A, OBJ3

//Mode 6: ->
//     1,    2,    3,    4,    5,    6
//  OBJ0, BG1B, OBJ1, OBJ2, BG1A, OBJ3

//Mode7: ->
//     1,    2,    3,    4,    5
//  OBJ0, BG1n, OBJ1, OBJ2, OBJ3

//Mode 7 EXTBG: ->
//     1,    2,    3,    4,    5,    6,    7
//  BG2B, OBJ0, BG1n, OBJ1, BG2A, OBJ2, OBJ3

auto PPU::Screen::scanline() -> void {
  if(ppu.io.displayDisable || ppu.vcounter() >= ppu.vdisp()) return renderBlack();

  uint16 above = cgram[0];
  uint16 below = (ppu.io.pseudoHires || ppu.io.bgMode == 5 || ppu.io.bgMode == 6) ? above : io.color;

  for(uint x : range(256)) {
    output.above[x].color = above;
    output.above[x].priority = 0;
    output.above[x].source = ID::BACK;
    output.above[x].exemption = false;
    output.below[x].color = below;
    output.below[x].priority = 0;
    output.below[x].source = ID::BACK;
    output.below[x].exemption = false;
  }

  ppu.window.buildTables(Window::ID::COL);
}

auto PPU::Screen::renderBlack() -> void {
  uint32* lineA = ppu.output + ppu.vcounter() * 1024;
  uint32* lineB = lineA + (ppu.interlace() ? 0 : 512);
  if(ppu.interlace() && ppu.field()) lineA += 512, lineB += 512;
  memory::fill(lineA, 512 * sizeof(uint32));
  memory::fill(lineB, 512 * sizeof(uint32));
}

auto PPU::Screen::getPixelAbove(uint x) -> uint16 {
  auto& above = output.above[x];
  auto& below = output.below[x];

  uint16 aboveColor, belowColor;
  uint8 belowLayer;

  aboveColor = above.color;

  if(!io.blendMode) {
    belowLayer = ID::BACK;
    belowColor = io.color;
  } else {
    belowLayer = below.source;
    belowColor = below.color;
  }

  if(!ppu.window.cache[Window::ID::COL].above[x]) {
    if(!ppu.window.cache[Window::ID::COL].below[x]) {
      return 0x0000;
    }
    aboveColor = 0x0000;
  }

  bool colorEnable;
  switch(above.source) {
  case Background::ID::BG1: colorEnable = io.bg1.colorEnable; break;
  case Background::ID::BG2: colorEnable = io.bg2.colorEnable; break;
  case Background::ID::BG3: colorEnable = io.bg3.colorEnable; break;
  case Background::ID::BG4: colorEnable = io.bg4.colorEnable; break;
  case Object::ID::OBJ:     colorEnable = io.obj.colorEnable; break;
  case Screen::ID::BACK:    colorEnable = io.back.colorEnable; break;
  }

  if(!above.exemption && colorEnable && ppu.window.cache[Window::ID::COL].below[x]) {
    math.colorHalve = false;
    if(io.colorHalve && ppu.window.cache[Window::ID::COL].above[x]) {
      if(io.blendMode == 0 || belowLayer != ID::BACK) {
        math.colorHalve = true;
      }
    }
    return blend(aboveColor, belowColor);
  }

  return aboveColor;
}

auto PPU::Screen::getPixelBelow(uint x) -> uint16 {
  auto& above = output.above[x];
  auto& below = output.below[x];

  uint16 aboveColor, belowColor;
  uint8 aboveLayer;

  belowColor = below.color;

  if(!io.blendMode) {
    aboveLayer = ID::BACK;
    aboveColor = io.color;
  } else {
    aboveLayer = above.source;
    aboveColor = above.color;
  }

  if(!ppu.window.cache[Window::ID::COL].above[x]) {
    if(!ppu.window.cache[Window::ID::COL].below[x]) {
      return 0x0000;
    }
    belowColor = 0x0000;
  }

  bool colorEnable;
  switch(below.source) {
  case Background::ID::BG1: colorEnable = io.bg1.colorEnable; break;
  case Background::ID::BG2: colorEnable = io.bg2.colorEnable; break;
  case Background::ID::BG3: colorEnable = io.bg3.colorEnable; break;
  case Background::ID::BG4: colorEnable = io.bg4.colorEnable; break;
  case Object::ID::OBJ:     colorEnable = io.obj.colorEnable; break;
  case Screen::ID::BACK:    colorEnable = io.back.colorEnable; break;
  }

  if(!below.exemption && colorEnable && ppu.window.cache[Window::ID::COL].below[x]) {
    math.colorHalve = false;
    if(io.colorHalve && ppu.window.cache[Window::ID::COL].above[x]) {
      if(io.blendMode == 0 || aboveLayer != ID::BACK) {
        math.colorHalve = true;
      }
    }
    return blend(belowColor, aboveColor);
  }

  return belowColor;
}

auto PPU::Screen::render() -> void {
  uint32* lineA = ppu.output + ppu.vcounter() * 1024;
  uint32* lineB = lineA + (ppu.interlace() ? 0 : 512);
  if(ppu.interlace() && ppu.field()) lineA += 512, lineB += 512;

  if(!ppu.io.pseudoHires && ppu.io.bgMode != 5 && ppu.io.bgMode != 6) {
    for(uint x : range(256)) {
      uint color = (ppu.io.displayBrightness << 15) | getPixelAbove(x);
      *lineA++ = *lineB++ = color;
      *lineA++ = *lineB++ = color;
    }
  } else {
    for(uint x : range(256)) {
      *lineA++ = *lineB++ = (ppu.io.displayBrightness << 15) | getPixelBelow(x);
      *lineA++ = *lineB++ = (ppu.io.displayBrightness << 15) | getPixelAbove(x);
    }
  }
}

auto PPU::Screen::Output::plotAbove(uint x, uint16 color, uint priority, uint source, bool exemption) -> void {
  if(priority > above[x].priority) {
    above[x].color = color;
    above[x].priority = priority;
    above[x].source = source;
    above[x].exemption = exemption;
  }
}

auto PPU::Screen::Output::plotBelow(uint x, uint16 color, uint priority, uint source, bool exemption) -> void {
  if(priority > below[x].priority) {
    below[x].color = color;
    below[x].priority = priority;
    below[x].source = source;
    below[x].exemption = exemption;
  }
}
