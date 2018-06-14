struct Interface;

struct System {
  enum class Model : uint {
    Famicom,
    VSSystem,
    PlayChoice10,
    FamicomBox,
  };

  enum class Region : uint { NTSCJ, NTSCU, PAL, Dendy };

  auto loaded() const -> bool { return information.loaded; }
  auto model() const -> Model { return information.model; }
  auto region() const -> Region { return information.region; }
  auto frequency() const -> double { return information.frequency; }

  auto run() -> void;
  auto runToSave() -> void;

  auto load(Emulator::Interface* interface, Model) -> bool;
  auto save() -> void;
  auto unload() -> void;

  auto power(bool reset) -> void;

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
    Model model = Model::Famicom;
    Region region = Region::NTSCJ;
    double frequency = Emulator::Constants::Colorburst::NTSC * 6.0;
  } information;

  uint serializeSize = 0;

  auto serialize(serializer&) -> void;
  auto serializeAll(serializer&) -> void;
  auto serializeInit() -> void;

  friend class Cartridge;
};

extern System system;

auto Model::Famicom() -> bool { return system.model() == System::Model::Famicom; }
auto Model::VSSystem() -> bool { return system.model() == System::Model::VSSystem; }
auto Model::PlayChoice10() -> bool { return system.model() == System::Model::PlayChoice10; }
auto Model::FamicomBox() -> bool { return system.model() == System::Model::FamicomBox; }

auto Region::NTSCJ() -> bool { return system.region() == System::Region::NTSCJ; }
auto Region::NTSCU() -> bool { return system.region() == System::Region::NTSCU; }
auto Region::PAL() -> bool { return system.region() == System::Region::PAL; }
auto Region::Dendy() -> bool { return system.region() == System::Region::Dendy; }
