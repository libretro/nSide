struct InputMapping {
  enum class Logic : uint { AND, OR };
  enum class Qualifier : uint { None, Lo, Hi, Rumble };

  auto bind() -> void;
  auto bind(string mapping, Logic logic) -> void;
  auto bind(shared_pointer<HID::Device> device, uint group, uint input, int16 newValue, Logic logic) -> bool;
  auto poll() -> int16;
  auto rumble(bool enable) -> void;
  auto unbind() -> void;

  auto isDigital() const -> bool { return type == 0; }
  auto isAnalog() const -> bool { return type == 1; }
  auto isRumble() const -> bool { return type == 2; }

  string path;  //configuration file key path
  string name;  //input name (human readable)
  uint type = 0;
  string assignment = "None";

  struct Mapping {
    shared_pointer<HID::Device> device;
    uint group = 0;
    uint input = 0;
    Qualifier qualifier = Qualifier::None;
  };
  vector<vector<Mapping>> mappings;
};

struct InputHotkey : InputMapping {
  function<auto () -> void> press;
  function<auto () -> void> release;

  auto pollIndex() -> maybe<uint>;
};

struct InputDevice {
  uint id;
  string name;
  vector<InputMapping> mappings;
};

struct InputPort {
  uint id;
  string name;
  vector<InputDevice> devices;
};

struct InputEmulator {
  Emulator::Interface* interface = nullptr;
  string name;
  vector<InputPort> ports;
};

struct InputManager {
  InputManager();
  auto bind(Emulator::Interface*) -> void;
  auto bind() -> void;
  auto poll() -> void;
  auto onChange(shared_pointer<HID::Device> device, uint group, uint input, int16_t oldValue, int16_t newValue) -> void;
  auto quit() -> void;

  auto mapping(uint port, uint device, uint input) -> maybe<InputMapping&>;
  auto findMouse() -> shared_pointer<HID::Device>;

  //hotkeys.cpp
  auto appendHotkeys() -> void;
  auto pollHotkeys() -> void;

  vector<shared_pointer<HID::Device>> devices;
  vector<InputEmulator> emulators;
  vector<InputHotkey*> hotkeys;

  InputEmulator* emulator = nullptr;  //points to InputEmulator that represents the currently active emulator
  uint64 lastPoll;   //time in milliseconds since last call to poll()
  uint64 frequency;  //minimum time in milliseconds before poll() can be called again
};

extern unique_pointer<InputManager> inputManager;
