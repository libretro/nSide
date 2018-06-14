#include "../shimai.hpp"
#include "sprite.cpp"
unique_pointer<Graphics> graphics;

Graphics::Graphics() {
  graphics = this;
};

auto Graphics::initialize() -> void {
  buffer.allocate(1280, 720);
  scale = buffer.height() / 240.0;
}

auto Graphics::reset() -> void {
  video->setSmooth(settings["Video/Shader"].text() != "None");
  video->setShader("");
  step = 0;

  sprites.reset();
  home->loadSprites(sprites);
}

auto Graphics::run() -> void {
  uint sampleRate = audio->frequency() / 60;

  if(step == 0) {
    home->updateSprites();

    buffer.fill(theme->backgroundColor);

    for(auto sprite : sprites) {
      uint x = max(0, sprite->x - sprite->ox);
      uint y = max(0, sprite->y - sprite->oy);

      if(buffer.alpha().mask() == 255u << 24) {
        uint stride = buffer.stride();
        if(x < buffer.width() && y < buffer.height()) {
          uint rx = x - (sprite->x - sprite->ox);
          uint ry = y - (sprite->y - sprite->oy);
          if(rx < sprite->width() && ry < sprite->height()) {
            uint rw = min(sprite->width () - rx, buffer.width () - x);
            uint rh = min(sprite->height() - ry, buffer.height() - y);
            for(uint row : range(rh)) {
              uint32* bufOffset = (uint32*)buffer.data() + (y + row) * buffer.width() + x;
              uint32* sprOffset = (uint32*)sprite->img->data() + (ry + row) * sprite->width() + rx;
              for(uint column : range(rw)) {
                uint8 alpha = *sprOffset >> 24;
                if(alpha == 0xff) {
                  *bufOffset = *sprOffset;
                } else if(alpha >= 0x01) {
                  uint8* s = (uint8*)sprOffset;
                  uint8* d = (uint8*)bufOffset;
                  for(uint o : range(3)) {
                    *d += ((*s - *d) * alpha) >> 8;
                    s++;
                    d++;
                  }
                }
                bufOffset++;
                sprOffset++;
              }
            }
          }
        }
      } else {
        if(x < buffer.width() && y < buffer.height()) {
          uint rx = x - (sprite->x - sprite->ox);
          uint ry = y - (sprite->y - sprite->oy);
          if(rx < sprite->width() && ry < sprite->height()) {
            uint rw = min(sprite->width () - rx, buffer.width () - x);
            uint rh = min(sprite->height() - ry, buffer.height() - y);
            buffer.impose(image::blend::sourceAlpha, x, y, *sprite->img, rx, ry, rw, rh);
          }
        }
      }
    }

    auto output = (uint32*)buffer.data();
    uint width  = buffer.width();
    uint height = buffer.height();
    program->videoRefresh(output, width * sizeof(uint32), width, height);
  }

  if(step++ >= sampleRate) step = 0;
}

auto Graphics::drawTextCenter(string_view text, image& img) -> void {
  img.fill(0x00000000);

  vector<uint32_t> codepoints;
  uint32_t codepoint;
  uint remainingBytes = 0;
  uint byteIndex = 0;
  while(byteIndex < text.size()) {
    if(remainingBytes) {
      remainingBytes--;
      codepoint |= (text[byteIndex++] & 0x3f) << (remainingBytes * 6);
      if(remainingBytes) continue;
      codepoints.append(codepoint);
      codepoint = 0;
    } else if(text[byteIndex] & 0x80) {
      if((text[byteIndex] & 0xe0) == 0xc0) remainingBytes = 1;
      if((text[byteIndex] & 0xf0) == 0xe0) remainingBytes = 2;
      if((text[byteIndex] & 0xf8) == 0xf0) remainingBytes = 3;
      codepoint = (text[byteIndex++] & (0x3f >> remainingBytes)) << (remainingBytes * 6);
    } else {
      codepoints.append(text[byteIndex++]);
    }
  }

  uint width = 0;
  for(uint index : range(codepoints.size())) {
    uint32_t codepoint = codepoints[index];
    auto glyph = theme->font.u[codepoint];
    if(!glyph) continue;
    width += glyph->width();
  }

  uint x = (img.width() - width) / 2;
  for(uint index : range(codepoints.size())) {
    uint32_t codepoint = codepoints[index];
    auto glyph = theme->font.u[codepoint];
    if(!glyph) continue;
    uint w = glyph->width();
    uint h = glyph->height();
    uint y = (img.height() - h) / 2;
    img.impose(image::blend::sourceAlpha, x, y, *glyph, 0, 0, w, h);
    x += w;
  }
}
