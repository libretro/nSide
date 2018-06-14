namespace MasterSystem {

struct ID {
  enum : uint {
    System,
    SG1000,
    MasterSystem,
    GameGear,
  };

  struct Port { enum : uint {
    Hardware,
    Controller1,
    Controller2,
  };};

  struct Device { enum : uint {
    None,
    SG1000Controls,
    MasterSystemControls,
    GameGearControls,
    Gamepad,
  };};
};

struct Interface : Emulator::Interface {
  Interface();

  auto manifest() -> string override;
  auto title() -> string override;

  auto loaded() -> bool override;
  auto sha256() -> string override;
  auto save() -> void override;
  auto unload() -> void override;

  auto connect(uint port, uint device) -> void override {}
  auto power() -> void override;
  auto run() -> void override;

  auto serialize() -> serializer override;
  auto unserialize(serializer&) -> bool override;

  auto cheatSet(const string_vector&) -> void override;

  auto cap(const string& name) -> bool override;
  auto get(const string& name) -> any override;
  auto set(const string& name, const any& value) -> bool override;
};

struct SG1000Interface : Interface {
  using Emulator::Interface::load;

  SG1000Interface();

  auto videoInformation() -> VideoInformation override;
  auto videoColors() -> uint32 override;
  auto videoColor(uint32 color) -> uint64 override;

  auto load(uint id) -> bool override;

  auto connect(uint port, uint device) -> void override;
};

struct MasterSystemInterface : Interface {
  using Emulator::Interface::load;

  MasterSystemInterface();

  auto videoInformation() -> VideoInformation override;
  auto videoColors() -> uint32 override;
  auto videoColor(uint32 color) -> uint64 override;

  auto load(uint id) -> bool override;

  auto connect(uint port, uint device) -> void override;
};

struct GameGearInterface : Interface {
  using Emulator::Interface::load;

  GameGearInterface();

  auto videoInformation() -> VideoInformation override;
  auto videoColors() -> uint32 override;
  auto videoColor(uint32 color) -> uint64 override;

  auto load(uint id) -> bool override;
};

struct Settings {
  bool colorEmulation = true;
  uint controllerPort1 = ID::Device::None;
  uint controllerPort2 = ID::Device::None;
};

extern Settings settings;

}
