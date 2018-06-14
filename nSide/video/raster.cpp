Raster::Raster() {
  originX = 0;
  originY = 0;
  width  = video.width;
  height = video.height;
  reset();
}

Raster::Raster(uint _x, uint _y, uint _width, uint _height) {
  assert(_x <= video.width  - _width);
  assert(_y <= video.height - _height);
  originX = _x;
  originY = _y;
  width  = _width;
  height = _height;
  reset();
}

auto Raster::reset() -> void {
  position = video.buffer + (originY * video.width + originX);
}

auto Raster::pixel(uint32 color) -> void {
  if(!video.effects.interframeBlending) {
    *position++ = video.palette[color];
  } else {
    auto a = *position;
    auto b = video.palette[color];
    *position++ = (a + b - ((a ^ b) & 0x01010101)) >> 1;
  }
}

auto Raster::scanline() -> void {
  assert((position - video.buffer) % video.width == originX);
  uint y = ((position - video.buffer - originX + video.width - 1) / video.width * video.width);
  position = video.buffer + y + originX;
}
