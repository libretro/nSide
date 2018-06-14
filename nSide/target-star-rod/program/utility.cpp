auto Program::initializeVideoDriver() -> void {
  video = Video::create();
  video->setContext(presentation->viewport.handle());

  video->setBlocking(settings["Video/Synchronize"].boolean());

  video->setSmooth(false);
  if(!video->ready()) {
    debugger->print("Failed to initialize video driver\n");
    video = Video::create("None");
  }
}

auto Program::initializeAudioDriver() -> void {
  audio = Audio::create();
  audio->setContext(presentation->viewport.handle());

  audio->setDevice(audio->availableDevices()(0));
  audio->setFrequency(audio->availableFrequencies()(0));
  audio->setLatency(audio->availableLatencies()(0));

  audio->setChannels(2);
  audio->setExclusive(false);
  audio->setBlocking(settings["Audio/Synchronize"].boolean());

  if(!audio->ready()) {
    debugger->print("Failed to initialize audio driver\n");
    audio = Audio::create("None");
  }

  Emulator::audio.setFrequency(settings["Audio/Frequency"].real());
}

auto Program::initializeInputDriver() -> void {
  input = Input::create();
  input->setContext(presentation->viewport.handle());

  if(!input->ready()) {
    debugger->print("Failed to initialize input driver\n");
    input = Input::create("None");
  }
}

auto Program::message(const string &text) -> void {
  debugger->print(text, "\n");
}

auto Program::updateAudioEffects() -> void {
  auto volume = settings["Audio/Mute"].boolean() ? 0.0 : 1.0;
  Emulator::audio.setVolume(volume);
}
