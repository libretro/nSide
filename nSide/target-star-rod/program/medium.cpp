auto Program::loadMedium() -> void {
  if(!mediumQueue) return;

  string location = mediumQueue.left();
  string type = Location::suffix(location).trimLeft(".", 1L);

  for(auto& medium : emulator->media) {
    if(medium.type != type) continue;
    return loadMedium(*emulator, medium);
  }

  mediumQueue.reset();
}

auto Program::loadMedium(Emulator::Interface& interface, const Emulator::Interface::Medium& medium) -> void {
  unloadMedium();

  mediumPaths.append(locateSystem({medium.name, ".sys/"}));
  debugger->print(medium.name, "\n");

  Emulator::audio.reset(2, audio->frequency());
  emulator->connect(SFC::ID::Port::Controller1, SFC::ID::Device::Gamepad);
  emulator->connect(SFC::ID::Port::Controller2, SFC::ID::Device::None);
  emulator->load(medium.id);  //This statement reloads Super Famicom.sys/manifest.bml,
  tileViewer->modeChanged();  //potentially changing the amount of available VRAM.
  emulator->set("Blur Emulation", false);
  emulator->set("Color Emulation", false);
  emulator->set("Scanline Emulation", false);
  emulator->power();

  presentation->setTitle(Location::base(mediumPaths(1)).trimRight("/"));
  debugger->print(emulator->manifest(), "\n");
  debugger->suspend();
}

auto Program::unloadMedium() -> void {
  emulator->unload();
  debugger->print("Cartridge unloaded\n");
  mediumPaths.reset();
}
