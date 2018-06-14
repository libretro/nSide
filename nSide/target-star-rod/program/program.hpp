struct Program : Emulator::Platform {
  //program.cpp
  Program(string_vector args);
  auto main() -> void;
  auto quit() -> void;

  //interface.cpp
  auto path(uint id) -> string override;
  auto open(uint id, string name, vfs::file::mode mode, bool required) -> vfs::shared::file override;
  auto load(uint id, string name, string type, string_vector options = {}) -> Emulator::Platform::Load override;
  auto videoRefresh(const uint32* data, uint pitch, uint width, uint height) -> void override;
  auto audioSample(const double* samples, uint channels) -> void override;
  auto inputPoll(uint port, uint device, uint index) -> int16 override;
  auto dipSettings(Markup::Node node) -> uint override { return 0u; }

  //medium.cpp
  auto loadMedium() -> void;
  auto loadMedium(Emulator::Interface& interface, const Emulator::Interface::Medium& medium) -> void;
  auto unloadMedium() -> void;

  //state.cpp
  auto stateName(uint slot) -> string;
  auto loadState(uint slot) -> bool;
  auto saveState(uint slot) -> bool;

  //utility.cpp
  auto initializeVideoDriver() -> void;
  auto initializeAudioDriver() -> void;
  auto initializeInputDriver() -> void;
  auto message(const string& text) -> void;
  auto updateAudioEffects() -> void;

  vector<string> mediumQueue;  //for command-line and drag-and-drop loading
  vector<string> mediumPaths;  //for keeping track of loaded folder locations

  Markup::Node higan_settings;
};

extern unique_pointer<Program> program;
