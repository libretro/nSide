struct Program : Emulator::Platform {
  Program();
  ~Program();

  Emulator::Interface* emulator = nullptr;

  auto path(uint id) -> string override;
  auto open(uint id, string name, vfs::file::mode mode, bool required) -> vfs::shared::file override;
  auto load(uint id, string name, string type, string_vector options) -> Load override;
  auto videoRefresh(const uint32* data, uint pitch, uint width, uint height) -> void override;
  auto audioSample(const double* samples, uint channels) -> void override;
  auto inputPoll(uint port, uint device, uint input) -> int16 override;
  auto inputRumble(uint port, uint device, uint input, bool enable) -> void override;
  auto dipSettings(Markup::Node node) -> uint override;
  auto notify(string text) -> void override;

  auto poll_once() -> void;

  string_vector medium_paths;
  string_vector loaded_manifest;

  serializer cached_serialize;
  bool has_cached_serialize = false;

  bool failed = false;
  bool polled = false;

  uint current_width = 0;
  uint current_height = 0;

  bool cartridge_folder = false;
};

static unique_pointer<Program> program;
