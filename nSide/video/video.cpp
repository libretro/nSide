#include <emulator/emulator.hpp>
#include <ramus/encode/png.hpp>

namespace Emulator {

//#include "raster.cpp"
#include "sprite.cpp"
Video video;

Video::~Video() {
  reset();
}

auto Video::reset() -> void {
  interface = nullptr;
  sprites.reset();
  delete buffer;
  buffer = nullptr;
  delete rotate;
  rotate = nullptr;
  delete palette;
  palette = nullptr;
  width = 0;
  height = 0;
  effects.colorBleed = false;
  effects.interframeBlending = false;
  effects.rotateLeft = false;
}

auto Video::setInterface(Interface* interface) -> void {
  this->interface = interface;
//if(interface) {
//  uint width  = interface->videoResolution().width;
//  uint height = interface->videoResolution().height;
//  resize(width, height);
//}
}

auto Video::setPalette() -> void {
  if(!interface) return;

  delete palette;
  colors = interface->videoColors();
  palette = new uint32[colors];
  for(auto index : range(colors)) {
    uint64 color = interface->videoColor(index);
    uint16 b = color.bits( 0,15);
    uint16 g = color.bits(16,31);
    uint16 r = color.bits(32,47);
    uint16 a = 0xffff;

    if(saturation != 1.0) {
      uint16 grayscale = uclamp<16>((r + g + b) / 3);
      double inverse = max(0.0, 1.0 - saturation);
      r = uclamp<16>(r * saturation + grayscale * inverse);
      g = uclamp<16>(g * saturation + grayscale * inverse);
      b = uclamp<16>(b * saturation + grayscale * inverse);
    }

    if(gamma != 1.0) {
      double reciprocal = 1.0 / 32767.0;
      r = r > 32767 ? r : uint16(32767 * pow(r * reciprocal, gamma));
      g = g > 32767 ? g : uint16(32767 * pow(g * reciprocal, gamma));
      b = b > 32767 ? b : uint16(32767 * pow(b * reciprocal, gamma));
    }

    if(luminance != 1.0) {
      r = uclamp<16>(r * luminance);
      g = uclamp<16>(g * luminance);
      b = uclamp<16>(b * luminance);
    }

    //convert color from 16-bits/channel to 8-bits/channel; force alpha to 1.0
    palette[index] = a.byte(1) << 24 | r.byte(1) << 16 | g.byte(1) << 8 | b.byte(1) << 0;
  }
}

auto Video::setSaturation(double saturation) -> void {
  this->saturation = saturation;
}

auto Video::setGamma(double gamma) -> void {
  this->gamma = gamma;
}

auto Video::setLuminance(double luminance) -> void {
  this->luminance = luminance;
}

auto Video::setEffect(Effect effect, const any& value) -> void {
  if(effect == Effect::ColorBleed && value.is<bool>()) {
    effects.colorBleed = value.get<bool>();
  }

  if(effect == Effect::InterframeBlending && value.is<bool>()) {
    effects.interframeBlending = value.get<bool>();
  }

  if(effect == Effect::Scanlines && value.is<bool>()) {
    effects.scanlines = value.get<bool>();
    resize(this->width, this->height);
  }

  if(effect == Effect::RotateLeft && value.is<bool>()) {
    effects.rotateLeft = value.get<bool>();
  }
}

auto Video::createSprite(uint width, uint height) -> shared_pointer<Sprite> {
  shared_pointer<Sprite> sprite = new Sprite{width, height};
  sprites.append(sprite);
  return sprite;
}

auto Video::removeSprite(shared_pointer<Sprite> sprite) -> bool {
  for(uint n : range(sprites)) {
    if(sprite == sprites[n]) {
      sprites.remove(n);
      return true;
    }
  }
  return false;
}

auto Video::refresh(uint32* input, uint pitch, uint width, uint height) -> void {
  resize(width, height);
  refreshRegion(input, pitch, 0, 0, width, height);
  refreshFinal();
}

auto Video::resize(uint width, uint height) -> void {
  if(this->width != width || this->height != height) {
    delete buffer;
    delete rotate;
    buffer = new uint32[width * (height << effects.scanlines)]();
    rotate = new uint32[(height << effects.scanlines) * width]();
    this->width = width;
    this->height = height;
  }
}

auto Video::refreshRegion(uint32* input, uint pitch, uint origin_x, uint origin_y, uint width, uint height, uint paletteOffset) -> void {
  auto output = buffer;
  pitch >>= 2;  //bytes to words

  palette += paletteOffset;
  if(!effects.scanlines) {
    int increment = 1;
    for(uint y : range(height)) {
      uint32* source = input + y * pitch;
      uint32* target = output + (origin_y + y) * this->width + origin_x;

      if(!effects.interframeBlending) {
        for(uint x : range(width)) {
          auto color = palette[*source++];
          *target++ = color;
        }
      } else {
        for(uint x : range(width)) {
          auto a = *target;
          auto b = palette[*source++];
          *target++ = (a + b - ((a ^ b) & 0x01010101)) >> 1;
        }
      }
    }
  } else {
    for(uint y : range(height)) {
      auto source = input + y * pitch;
      auto targetLo = output + (origin_y + y) * this->width * 2 + origin_x;
      auto targetHi = output + (origin_y + y) * this->width * 2 + origin_x + this->width;

      if(!effects.interframeBlending) {
        for(uint x : range(width)) {
          auto color = palette[*source++];
          *targetLo++ = color;
          *targetHi++ = (255 << 24) | ((color & 0xfefefe) >> 1);
        }
      } else {
        for(uint x : range(width)) {
          auto a = *targetLo;
          auto b = palette[*source++];
          *targetLo++ = (a + b - ((a ^ b) & 0x01010101)) >> 1;
          auto c = *targetHi;
          auto d = (255 << 24) | ((b & 0xfefefe) >> 1);
          *targetHi++ = (c + d - ((c ^ d) & 0x01010101)) >> 1;
        }
      }
    }
  }
  palette -= paletteOffset;
}

auto Video::refreshFinal() -> void {
  auto output = buffer;
  uint width  = this->width;
  uint height = this->height << effects.scanlines;

  if(effects.colorBleed) {
    for(uint y : range(height)) {
      auto target = output + y * width;
      for(uint x : range(width)) {
        auto a = target[x];
        auto b = target[x + (x != width - 1)];
        target[x] = (a + b - ((a ^ b) & 0x01010101)) >> 1;
      }
    }
  }

  if(effects.rotateLeft) {
    for(uint y : range(height)) {
      auto source = buffer + y * width;
      for(uint x : range(width)) {
        auto target = rotate + (width - 1 - x) * height + y;
        *target = *source++;
      }
    }
    output = rotate;
    swap(width, height);
  }

  for(auto& sprite : sprites) {
    if(!sprite->visible) continue;

    for(int y : range(sprite->height)) {
      for(int x : range(sprite->width)) {
        int pixelY = sprite->y + y;
        if(pixelY < 0 || pixelY >= height) continue;

        int pixelX = sprite->x + x;
        if(pixelX < 0 || pixelX >= width) continue;

        auto pixel = sprite->pixels[y * sprite->width + x];
        if(pixel) output[pixelY * width + pixelX] = 0xff000000 | pixel;
      }
    }
  }

  platform->videoRefresh(output, width * sizeof(uint32), width, height << effects.scanlines);
}

auto Video::clear() -> void {
  memory::fill(buffer, width * (height << effects.scanlines) * sizeof(uint32));
}

auto Video::screenshot(string path) -> void {
  uint width = this->width;
  uint height = this->height << effects.scanlines;
  if(!effects.rotateLeft) {
    ramus::Encode::PNG::create(path, (uint32_t*)buffer, width, height, false);
  } else {
    ramus::Encode::PNG::create(path, (uint32_t*)rotate, height, width, false);
  }
}

}
