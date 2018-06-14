auto System::serialize() -> serializer {
  serializer s(serializeSize);

  uint signature = 0x31545342;
  char version[16] = {};
  char hash[64] = {};
  char description[512] = {};
  memory::copy(&version, (const char*)Emulator::SerializerVersion, Emulator::SerializerVersion.size());
  memory::copy(&hash, (const char*)cartridgeSlot[model() == Model::VSSystem].sha256(), 64);

  s.integer(signature);
  s.array(version);
  s.array(hash);
  s.array(description);

  serializeAll(s);
  return s;
}

auto System::unserialize(serializer& s) -> bool {
  uint signature = 0;
  char version[16] = {};
  char hash[64] = {};
  char description[512] = {};

  s.integer(signature);
  s.array(version);
  s.array(hash);
  s.array(description);

  if(signature != 0x31545342) return false;
  if(string{version} != Emulator::SerializerVersion) return false;

  power(/* reset = */ false);
  serializeAll(s);
  return true;
}

//internal

auto System::serialize(serializer& s) -> void {
}

auto System::serializeAll(serializer& s) -> void {
  random.serialize(s);
  for(auto& cartridge : cartridgeSlot) cartridge.serialize(s);
  system.serialize(s);

  cpuM.serialize(s);
  apuM.serialize(s);
  ppuM.serialize(s);

  if(model() == Model::VSSystem) {
    cpuS.serialize(s);
    apuS.serialize(s);
    ppuS.serialize(s);
    vssystem.serialize(s);
  }

  if(model() == Model::PlayChoice10) playchoice10.serialize(s);
  if(model() == Model::FamicomBox)   famicombox.serialize(s);

  controllerPortM1.serialize(s);
  controllerPortM2.serialize(s);
  if(model() != Model::VSSystem) expansionPort.serialize(s);
  if(model() == Model::VSSystem) controllerPortS1.serialize(s);
  if(model() == Model::VSSystem) controllerPortS2.serialize(s);
}

//perform dry-run state save:
//determines exactly how many bytes are needed to save state for this cartridge,
//as amount varies per game (eg different RAM sizes, etc.)
auto System::serializeInit() -> void {
  serializer s;

  uint signature = 0;
  char version[16] = {};
  char hash[64] = {};
  char description[512] = {};

  s.integer(signature);
  s.array(version);
  s.array(hash);
  s.array(description);

  serializeAll(s);
  serializeSize = s.size();
}
