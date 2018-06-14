struct Interface;

struct System {
  enum class Region : uint { NTSC, PAL, SECAM };

  inline auto loaded() const -> bool { return information.loaded; }
  inline auto region() const -> Region { return information.region; }
  inline auto frequency() const -> double { return information.frequency; }

  auto run() -> void;
  auto runToSave() -> void;

  auto load(Emulator::Interface*) -> bool;
  auto save() -> void;
  auto unload() -> void;
  auto power() -> void;

  //video.cpp
  auto configureVideoPalette() -> void;
  auto configureVideoEffects() -> void;

  //serialization.cpp
  auto serialize() -> serializer;
  auto unserialize(serializer&) -> bool;

private:
  Emulator::Interface* interface = nullptr;

  struct Information {
    string manifest;
    bool loaded = false;
    Region region = Region::NTSC;
    double frequency = Emulator::Constants::Colorburst::NTSC;
  } information;

  uint serializeSize = 0;

  auto serialize(serializer&) -> void;
  auto serializeAll(serializer&) -> void;
  auto serializeInit() -> void;

  friend class Cartridge;
};

extern System system;

auto Region::NTSC() -> bool { return system.region() == System::Region::NTSC; }
auto Region::PAL() -> bool { return system.region() == System::Region::PAL; }
auto Region::SECAM() -> bool { return system.region() == System::Region::SECAM; }
