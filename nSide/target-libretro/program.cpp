//TODO: find a way to split the code into 4 linker objects without having
//icarus's Database variables complain of multiple definitions
//#include "libretro.hpp"

Program::Program() {
  Emulator::platform = this;
  emulator = create_emulator_interface();
  flush_variables(emulator);
}

Program::~Program() {
  delete emulator;
}

auto Program::path(uint id) -> string {
  return medium_paths(id);
}

auto Program::open(uint id, string name, vfs::file::mode mode, bool required) -> vfs::shared::file {
  //For cartridge folders, compatibility with higan proper is paramount.
  //When not using a cartridge folder, block higan's save system so
  //that libretro's save system can be used instead.
  if(!cartridge_folder && id != BackendSpecific::system_id) {
    if(name == "save.ram" || name == "nec.data.ram") return {};
  }

  libretro_print(RETRO_LOG_INFO, "Accessing data from %u: %s (required: %s)\n",
    id, name.data(), required ? "yes" : "no");

  //Load the game manifest.
  if(name == "manifest.bml" && id != BackendSpecific::system_id && loaded_manifest(id)) {
    string manifest = loaded_manifest(id);

    libretro_print(RETRO_LOG_INFO, "Loaded Manifest:\n%s\n", manifest.data());

    return vfs::memory::file::open(manifest.data<uint8_t>(), manifest.size());
  }

  //Try to load static internal files.
  if(id == BackendSpecific::system_id) {
    auto builtin = load_builtin_system_file(name);
    if(builtin) return builtin;
  }

  //If this is file we imported in Icarus, use that.
  if(auto imported = icarus(id).imported_files.find(name)) {
    libretro_print(RETRO_LOG_INFO, "Reading imported file: %s.\n", name.data());

    auto& file = imported.get();
    if(name == "manifest.bml") {
      string manifest(reinterpret_cast<const char*>(file.data()), file.size());
      libretro_print(RETRO_LOG_INFO, "Loaded Manifest:\n%s\n", manifest.data());
    }
    return vfs::memory::file::open(file.data(), file.size());
  }

  //This will be the default save path as chosen during load.
  //If we load from manifest, this will always point to the appropriate directory.
  string p = {path(id), name};

  //If we're trying to load something, and it doesn't exist, try to find it elsewhere.
  if(mode == vfs::file::mode::read && !inode::exists(p)) {
    libretro_print(RETRO_LOG_INFO, "%s does not exist, trying another path.\n", p.data());

    p = locate_libretro(name);
  }

  //Something else, load it from disk.
  libretro_print(RETRO_LOG_INFO, "Trying to %s file %s.\n",
    mode == vfs::file::mode::read ? "read" : "write", p.data());

  if(auto result = vfs::fs::file::open(p, mode)) return result;

  //Fail load if necessary.
  if(required) {
    libretro_print(RETRO_LOG_ERROR, "Failed to open required file %s.\n", p.data());
    failed = true;
  }

  return {};
}

auto Program::load(uint id, string name, string, string_vector options) -> Emulator::Platform::Load {
  //Have to return the first option here to get automatic region detection.
  libretro_print(RETRO_LOG_INFO, "Loading game for ID: %u using option %s.\n",
      id, options(0).data());

  return {id, options(0)};
}

auto Program::videoRefresh(const uint32* data, uint pitch, uint width, uint height) -> void {
  float par;
  data += adjust_video_resolution(width, height, pitch, par) >> 2;

  if(width != program->current_width || height != program->current_height) {
    //If internal resolution changes, notify the frontend if it cares about it.
    retro_game_geometry geom = {};
    geom.base_width = width;
    geom.base_height = height;
    geom.aspect_ratio = emulator->videoInformation().aspectCorrection * par *
      (float(width) / float(height));
    program->current_width = width;
    program->current_height = height;
    environ_cb(RETRO_ENVIRONMENT_SET_GEOMETRY, &geom);
  }

  video_output(data, width, height, pitch);
}

auto Program::audioSample(const double* samples, uint channels) -> void {
  //Double the fun!
  static auto d2i16 = [](double v) -> int16_t {
    v *= 0x8000;
         if(v >  0x7fff) v =  0x7fff;
    else if(v < -0x8000) v = -0x8000;
    return int16_t(floor(v + 0.5));
  };

  int16_t left = d2i16(samples[0]);
  int16_t right = d2i16(samples[1]);
  audio_cb(left, right);
}

auto Program::inputRumble(uint port, uint device, uint input, bool enable) -> void {
  //For Game Boy Advance.
  //nSide's libretro core is specific to the Super Famicom balanced profile,
  //so use higan instead for Game Boy Advance support.
}

auto Program::dipSettings(Markup::Node) -> uint {
  //For Nintendo Super System. Currently not supported.
  return 0x0000;
}

auto Program::notify(string text) -> void {
  libretro_print(RETRO_LOG_INFO, "higan INFO: %s\n", text.data());
}

auto Program::poll_once() -> void {
  //Poll as late as possible, frontend might also do it like this.
  if(!program->polled) {
    input_poll();
    program->polled = true;
  }
}
