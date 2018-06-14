struct Sound {
  Sound();

  auto reset() -> void;
  auto run() -> void;

  shared_pointer<Emulator::Stream> stream;
  uint playOffset = 0;
  uint loopOffset;
};

extern unique_pointer<Sound> sound;
