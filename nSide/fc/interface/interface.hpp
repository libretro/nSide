namespace Famicom {

struct ID {
  enum : uint {
    System,
    Famicom,
    VSSystem,
    PlayChoice10,
    FamicomBox,
  };

  struct Port { enum : uint {
    Hardware,
    Controller1,
    Controller2,
    Expansion,
  };};

  struct Device { enum : uint {
    None,
    FamicomControls,
    VSSystemControls,
    PlayChoice10Controls,
    FamicomBoxControls,

    Gamepad,
    GamepadMic,
    FourScore,
    Zapper,
    PowerPad,
    Vaus,
    SNESGamepad,
    Mouse,

    GamepadE,
    JoyPair,
    FourPlayers,
    BeamGun,
    FamilyKeyboard,
    FamilyTrainer,
    VausE,
    SFCGamepad,
    MouseE,
  };};
};

struct Interface : Emulator::Interface {
  using Emulator::Interface::load;

  Interface();

  auto manifest() -> string override;
  auto title() -> string override;

  auto loaded() -> bool override;
  auto sha256() -> string override;
  auto save() -> void override;
  auto unload() -> void override;

  auto connect(uint port, uint device) -> void override {}
  auto power() -> void override;
  auto reset() -> void override;
  auto run() -> void override;

  auto serialize() -> serializer override;
  auto unserialize(serializer&) -> bool override;

  auto cheatSet(const string_vector&) -> void override;

  auto cap(const string& name) -> bool override;
  auto get(const string& name) -> any override;
  auto set(const string& name, const any& value) -> bool override;
};

struct FamicomInterface : Interface {
  FamicomInterface();

  auto videoInformation() -> VideoInformation override;
  auto videoColors() -> uint32 override;
  auto videoColor(uint32 color) -> uint64 override;

  auto load(uint id) -> bool override;

  auto connect(uint port, uint device) -> void override;
};

struct VSSystemInterface : Interface {
  VSSystemInterface();

  auto videoInformation() -> VideoInformation override;
  auto videoColors() -> uint32 override;
  auto videoColor(uint32 color) -> uint64 override;

  auto load(uint id) -> bool override;
};

struct PlayChoice10Interface : Interface {
  PlayChoice10Interface();

  auto videoInformation() -> VideoInformation override;
  auto videoColors() -> uint32 override;
  auto videoColor(uint32 color) -> uint64 override;

  auto load(uint id) -> bool override;

  auto connect(uint port, uint device) -> void override;
};

struct FamicomBoxInterface : Interface {
  FamicomBoxInterface();

  auto videoInformation() -> VideoInformation override;
  auto videoColors() -> uint32 override;
  auto videoColor(uint32 color) -> uint64 override;

  auto load(uint id) -> bool override;

  auto connect(uint port, uint device) -> void override;
};

struct Settings {
  bool colorEmulation = true;
  bool scanlineEmulation = true;

  uint controllerPort1 = ID::Device::None;
  uint controllerPort2 = ID::Device::None;
  uint expansionPort = ID::Device::None;
  bool random = true;
};

extern Settings settings;

}
