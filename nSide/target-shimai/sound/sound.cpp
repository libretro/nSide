#include "../shimai.hpp"
unique_pointer<Sound> sound;

Sound::Sound() {
  sound = this;
}

auto Sound::reset() -> void {
  if(theme->bgm) {
    theme->bgm->seek(4);
    loopOffset = 8 + theme->bgm->readl(4) * 4;
    if(loopOffset > theme->bgm->size()) loopOffset = 8;
    theme->bgm->seek(playOffset);
  }

  Emulator::audio.reset(2, audio->frequency());
  stream = Emulator::audio.createStream(2, 44'100.0);
}

auto Sound::run() -> void {
  double left  = 0;
  double right = 0;

  if(theme->bgm) {
    if(theme->bgm->end()) {
      theme->bgm->seek(playOffset = loopOffset);
    } else {
      playOffset += 4;
      left  = (double)(int16)theme->bgm->readl(2) / 32768.0;
      right = (double)(int16)theme->bgm->readl(2) / 32768.0;
    }
  }
  stream->sample(left, right);
}
