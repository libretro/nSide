#include "libretro.hpp"

auto locate_libretro(string name) -> string {
  //Try libretro specific paths first ...
  //This is relevant for special chip ROMs/BIOS, etc.
  const char* sys = nullptr;
  if(environ_cb && environ_cb(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &sys) && sys) {
    string location = {sys, "/", name};
    location = location.transform("\\", "/");
    if(inode::exists(location)) return location;
  }

  //If not, fall back to standard higan paths.
  //Prefer higan paths over nSide paths, but still allow the latter.
  string location;

  location = {Path::config(), "higan/", name};
  if(inode::exists(location)) return location;

  location = {Path::local(), "higan/", name};
  if(inode::exists(location)) return location;

  location = {Path::config(), "nSide/", name};
  if(inode::exists(location)) return location;

  location = {Path::local(), "nSide/", name};
  if(inode::exists(location)) return location;

  directory::create({Path::local(), "higan/"});
  return {Path::local(), "higan/", name};
}

//TODO: find a way to split the code into 4 linker objects without having
//icarus's Database variables complain of multiple definitions
#include "program.cpp"
#include "icarus.cpp"

#if defined(libretro_use_sfc)
#error The accuracy profile is prohibited in nSide. Use higan instead.
#elif defined(libretro_use_sfc_balanced)
#include "libretro-sfc.cpp"
#else
#error "Unrecognized higan core."
#endif

RETRO_API void retro_set_environment(retro_environment_t cb) {
  environ_cb = cb;

  retro_log_callback log = {};
  if(environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log) && log.log) libretro_print = log.log;

  set_environment_info(cb);
}

RETRO_API void retro_set_video_refresh(retro_video_refresh_t cb) {
  video_cb = cb;
}

RETRO_API void retro_set_audio_sample(retro_audio_sample_t cb) {
  audio_cb = cb;
}

RETRO_API void retro_set_audio_sample_batch(retro_audio_sample_batch_t) {
}

RETRO_API void retro_set_input_poll(retro_input_poll_t cb) {
  input_poll = cb;
}

RETRO_API void retro_set_input_state(retro_input_state_t cb) {
  input_state = cb;
}

RETRO_API void retro_init() {
  settings["icarus/CreateManifests"].setValue(true);
  program = new Program;
}

RETRO_API void retro_deinit() {
  program.reset();
}

RETRO_API unsigned retro_api_version() {
  return RETRO_API_VERSION;
}

RETRO_API void retro_get_system_info(retro_system_info* info) {
  info->library_name = BackendSpecific::name;
  info->library_version = Emulator::Version;
  info->valid_extensions = BackendSpecific::extensions;
  info->need_fullpath = false;
  info->block_extract = false;
}

RETRO_API void retro_get_system_av_info(struct retro_system_av_info* info) {
  auto res = program->emulator->videoInformation();
  info->geometry.base_width = res.width;
  info->geometry.base_height = res.height;
  info->geometry.max_width = res.internalWidth;
  info->geometry.max_height = res.internalHeight;

  float par;
  uint pitch = 0;
  adjust_video_resolution(info->geometry.base_width, info->geometry.base_height, pitch, par);

  // Adjust for pixel aspect ratio.
  info->geometry.aspect_ratio = res.aspectCorrection * par *
    float(info->geometry.base_width) / float(info->geometry.base_height);

  info->timing.fps = res.refreshRate;

  libretro_print(RETRO_LOG_INFO, "Reported video rate: %.4f Hz.\n", info->timing.fps);

  // We control this.
  info->timing.sample_rate = BackendSpecific::audio_rate;
}

RETRO_API void retro_set_controller_port_device(unsigned port, unsigned device) {
  set_controller_ports(port, device);
}

RETRO_API void retro_reset() {
  program->emulator->reset();
}

RETRO_API void retro_run() {
  check_variables(program->emulator);

  program->polled = false;
  program->has_cached_serialize = false;
  program->emulator->run();
  program->poll_once();  //In case higan did not poll this frame.
}

RETRO_API size_t retro_serialize_size() {
  //To avoid having to serialize twice to query the size -> serialize.
  if(program->has_cached_serialize) {
    return program->cached_serialize.size();
  } else {
    program->cached_serialize = program->emulator->serialize();
    program->has_cached_serialize = true;
    return program->cached_serialize.size();
  }
}

RETRO_API bool retro_serialize(void* data, size_t size) {
  if(!program->has_cached_serialize) {
    program->cached_serialize = program->emulator->serialize();
    program->has_cached_serialize = true;
  }

  if(program->cached_serialize.size() != size) return false;

  memcpy(data, program->cached_serialize.data(), size);
  return true;
}

RETRO_API bool retro_unserialize(const void* data, size_t size) {
  serializer s(static_cast<const uint8_t*>(data), size);
  program->has_cached_serialize = false;
  return program->emulator->unserialize(s);
}

RETRO_API void retro_cheat_reset() {
  //TODO: v094 implementation seems to have had something here, but this can wait.
  program->has_cached_serialize = false;
}

RETRO_API void retro_cheat_set(unsigned index, bool enabled, const char* code) {
  //TODO: v094 implementation seems to have had something here, but this can wait.
  program->has_cached_serialize = false;
}

RETRO_API bool retro_load_game(const retro_game_info* game) {
  //Need 8-bit (well, apparently 9-bit for SNES to be pedantic, but higan also uses 8-bit).
  retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
  if(!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt)) return false;

  Emulator::audio.reset(2, BackendSpecific::audio_rate);
  const Emulator::Interface::Medium* emulator_medium = nullptr;

  //Each libretro implementation just has a single core, but it might have multiple mediums.
  for(auto& medium : program->emulator->media) {
    if(medium.type == BackendSpecific::medium_type) {
      emulator_medium = &medium;
      break;
    }
  }

  if(!program->emulator || !emulator_medium) return false;

  //For certain game types (like gameboy for SGB, we will actually load the gameboy cartridge here,
  //not the SFC cart.
  uint id = get_special_id_from_path(emulator_medium->id, game->path);

  //Get the folder of the system directory.
  //Generally, this will go unused, but it will be relevant for some backends.
  program->medium_paths(BackendSpecific::system_id) = locate_libretro({ emulator_medium->name, ".sys/" });

  string game_path;
  if(game->path) game_path = string{game->path}.transform("\\", "/");

  //TODO: Can we detect more robustly if we have a BML loaded from memory?
  //If we don't have a path (game loaded from pure VFS for example), we cannot use manifests.
  bool loading_manifest = game_path && game_path.endsWith(".bml");
  bool loading_folder = game_path && game_path.endsWith(".rom");
  program->cartridge_folder = loading_manifest || loading_folder;

  //If we're loading from a foltainer, the ID will depend on the folder extension.
  if(loading_manifest || loading_folder) {
    auto dir = Location::dir(game_path).trimRight("/", 1L);
    id = get_special_id_from_path(emulator_medium->id, dir);
  }

  program->loaded_manifest.reset();

  if(loading_folder) {
    //If we try to load a ROM file, assume this is inside a foltainer (typically called program.rom), and we should create a manifest.
    //This seems to be the preferred way as the manifest format is not *that* stable.
    program->medium_paths(id) = Location::dir(game_path);
    libretro_print(RETRO_LOG_INFO, "Trying to generate manifest for foltainer: %s.\n",
      program->medium_paths(id).data());

    //Have to use plain Icarus here or we have to use the memory-only interface in LibretroIcarus.
    program->loaded_manifest(id) = plain_icarus.manifest(program->medium_paths(id));
    loading_manifest = true;

    if(!program->loaded_manifest) {
      libretro_print(RETRO_LOG_ERROR, "Failed to create manifest from foltainer: %s.\n",
        program->medium_paths(id).data());
      return false;
    }
  } else if(loading_manifest) {
    //Load ROM and RAM from the directory.
    program->medium_paths(id) = Location::dir(game_path);
    program->loaded_manifest(id) = string_view(static_cast<const char*>(game->data), (uint)game->size);
  } else {
    //Try to find appropriate paths for save data.
    if(game_path) {
      auto base_name = game_path;
      string save_path;

      auto suffix = Location::suffix(base_name);
      auto base = Location::base(base_name);

      const char* save = nullptr;
      if(environ_cb && environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &save) && save) {
        save_path = { string(save).transform("\\", "/"), "/", base.trimRight(suffix, 1L), "." };
      } else {
        save_path = { base_name.trimRight(suffix, 1L), "." };
      }

      program->medium_paths(id) = save_path;
    } else {
      //Fallback. No idea, use the game SHA256.
      auto sha = string{Hash::SHA256(static_cast<const uint8_t*>(game->data), game->size).digest(), ".sfc/"};

      const char* save = nullptr;
      if(environ_cb && environ_cb(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &save) && save) {
        string save_path = { save, "/", sha };
        directory::create(save_path);
        program->medium_paths(id) = save_path;
      } else {
        // Use the system data somehow ... This is really deep into fallback territory.
        auto path = locate_libretro(sha);
        directory::create(path);
        program->medium_paths(id) = path;
      }
    }

    //Import the game with Icarus.
    //Create a fake path for Icarus to import.
    //We need a sane extension so Icarus can dispatch to the right importer.
    string ext = game_path ? Location::suffix(game_path) : get_default_id_extension(id);
    string fake_game_path = {"game", ext};
    if(!icarus(id).import_rom(fake_game_path, static_cast<const uint8_t*>(game->data), game->size)) {
      libretro_print(RETRO_LOG_ERROR, "Failed to import game with Icarus.\n");
      return false;
    }
  }

  libretro_print(RETRO_LOG_INFO, "Using base path: %s for game data.\n", program->path(id).data());

  if(!load_special_bios(id)) return false;
  if(!program->emulator->load(emulator_medium->id)) return false;
  if(program->failed) return false;
  if(!program->emulator->loaded()) return false;

  //Setup some defaults.
  //TODO: Might want to use the option interface for these,
  //but most of these seem better suited for shaders tbh ...
  program->emulator->power();
  Emulator::video.setSaturation(1.0);
  Emulator::video.setGamma(1.0);
  Emulator::video.setLuminance(1.0);

  Emulator::video.setPalette();
  Emulator::audio.setFrequency(44100.0);
  Emulator::audio.setVolume(1.0);
  Emulator::audio.setBalance(0.0);
  Emulator::audio.setReverb(false);

  set_default_controller_ports();
  program->has_cached_serialize = false;

  retro_system_av_info av_info;
  retro_get_system_av_info(&av_info);
  program->current_width = av_info.geometry.base_width;
  program->current_height = av_info.geometry.base_height;

  return true;
}

RETRO_API bool retro_load_game_special(unsigned game_type, const struct retro_game_info* info, size_t num_info) {
  //TODO: Sufami and other shenanigans?
  return false;
}

RETRO_API void retro_unload_game() {
  program->emulator->unload();
}

RETRO_API unsigned retro_get_region() {
  //This function isn't all that important,
  //but less than 59 FPS would mean PAL to account for possible variations in the implementation.
  if(program && program->emulator) {
    return program->emulator->videoInformation().refreshRate < 59.0f ? RETRO_REGION_PAL : RETRO_REGION_NTSC;
  } else {
    //Shouldn't happen, but provide some fallback.
    return RETRO_REGION_NTSC;
  }
}

//When using a cartridge folder, rely on higan to load and save SRAM.
//When using a standalone ROM, block higan from loading and saving SRAM so that libretro does it instead.
RETRO_API void* retro_get_memory_data(unsigned id) {
  if(!program->emulator || !program->emulator->loaded()) return nullptr;
  if(program->cartridge_folder && id == RETRO_MEMORY_SAVE_RAM) return nullptr;
  return get_memory_data(id);
}

RETRO_API size_t retro_get_memory_size(unsigned id) {
  if(!program->emulator || !program->emulator->loaded()) return 0;
  if(program->cartridge_folder && id == RETRO_MEMORY_SAVE_RAM) return 0;
  return get_memory_size(id);
}
