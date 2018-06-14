//TODO: find a way to split the code into 4 linker objects without having
//icarus's Database variables complain of multiple definitions
//#include "libretro.hpp"

#ifdef PROFILE_BALANCED
#include <sfc-balanced/sfc.hpp>
#else
#error The accuracy profile is prohibited in nSide. Use higan instead.
#endif
#undef platform

#include "resource/sfc.cpp"

enum class VideoMode {
  FullRes,
  MergeScanlines,
  HalfRes,
  FullResCrop,
  MergeScanlinesCrop,
  HalfResCrop
};
static VideoMode video_mode = VideoMode::FullRes;
static string sgb_path;

auto adjust_video_resolution(uint& width, uint& height, uint& pitch, float& pixel_aspect_correction) -> uint {
  uint old_height = height;

  switch(video_mode) {

  case VideoMode::MergeScanlines:
    height >>= 1;
    pitch <<= 1;
    pixel_aspect_correction = 0.5f;
    return 0;

  case VideoMode::MergeScanlinesCrop:
    old_height >>= 1;
    height = 224;
    pitch <<= 1;
    pixel_aspect_correction = 0.5f;
    return pitch * ((old_height - 224) >> 1);

  case VideoMode::HalfRes:
    old_height >>= 1;
    width >>= 1;
    height >>= 1;
    pitch <<= 1;
    pixel_aspect_correction = 1.0f;
    return 0;

  case VideoMode::HalfResCrop:
    old_height >>= 1;
    width >>= 1;
    height = 224;
    pitch <<= 1;
    pixel_aspect_correction = 1.0f;
    return pitch * ((old_height - 224) >> 1);

  case VideoMode::FullRes:
    pixel_aspect_correction = 1.0f;
    return 0;

  case VideoMode::FullResCrop:
    pixel_aspect_correction = 1.0f;
    height = 448;
    return pitch * ((old_height - 448) >> 1);

  }
}

auto video_output(const uint32* data, uint width, uint height, uint pitch) -> void {
  switch(video_mode) {
    case VideoMode::FullRes:
    case VideoMode::FullResCrop:
    case VideoMode::MergeScanlines:
    case VideoMode::MergeScanlinesCrop:
      video_cb(data, width, height, pitch);
      return;

    default:
      break;
  }

  uint word_pitch = pitch >> 2;

  retro_framebuffer fb = {};
  fb.width = width;
  fb.height = height;
  fb.access_flags = RETRO_MEMORY_ACCESS_WRITE;

  uint dst_pitch;
  void* dst_buffer;

  //Use SOFTWARE_FRAMEBUFFER to avoid an extra copy if possible.
  if(environ_cb(RETRO_ENVIRONMENT_GET_CURRENT_SOFTWARE_FRAMEBUFFER, &fb) && fb.format == RETRO_PIXEL_FORMAT_XRGB8888) {
    dst_buffer = fb.data;
    dst_pitch = fb.pitch;
  } else {
    static uint32 scratch[256 * 240];
    dst_buffer = scratch;
    dst_pitch = width * sizeof(uint32);
  }

  auto* dst = static_cast<uint32*>(dst_buffer);
  auto dst_pitch_word = dst_pitch >> 2;
  for(uint y = 0; y < height; y++, dst += dst_pitch_word, data += word_pitch) {
    for(uint x = 0, src_x = 0; x < width; x++, src_x += 2) dst[x] = data[src_x];
  }

  video_cb(dst_buffer, width, height, dst_pitch);
}

auto flush_variables(Emulator::Interface* iface) -> void {
  retro_variable variable = { "higan_sfc_internal_resolution", nullptr };
  if(environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variable) && variable.value) {
         if(strcmp(variable.value, "512x480") == 0) video_mode = VideoMode::FullRes;
    else if(strcmp(variable.value, "512x448") == 0) video_mode = VideoMode::FullResCrop;
    else if(strcmp(variable.value, "512x240") == 0) video_mode = VideoMode::MergeScanlines;
    else if(strcmp(variable.value, "512x224") == 0) video_mode = VideoMode::MergeScanlinesCrop;
    else if(strcmp(variable.value, "256x240") == 0) video_mode = VideoMode::HalfRes;
    else if(strcmp(variable.value, "256x224") == 0) video_mode = VideoMode::HalfResCrop;
  }

  variable = {"higan_sfc_color_emulation", nullptr};
  if(environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variable) && variable.value) {
         if(strcmp(variable.value, "ON" ) == 0) iface->set("Color Emulation", true);
    else if(strcmp(variable.value, "OFF") == 0) iface->set("Color Emulation", false);
  }

  variable = {"higan_sfc_blur_emulation", nullptr};
  if(environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variable) && variable.value) {
         if(strcmp(variable.value, "ON" ) == 0) iface->set("Blur Emulation", true);
    else if(strcmp(variable.value, "OFF") == 0) iface->set("Blur Emulation", false);
  }

  variable = {"higan_sfc_scanline_emulation", nullptr};
  if(environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variable) && variable.value) {
         if(strcmp(variable.value, "ON" ) == 0) iface->set("Scanline Emulation", true);
    else if(strcmp(variable.value, "OFF") == 0) iface->set("Scanline Emulation", false);
  }

  variable = {"higan_sfc_sgb_bios", nullptr};
  if(environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &variable) && variable.value) {
    sgb_path = variable.value;
  }
}

auto check_variables(Emulator::Interface* iface) -> void {
  bool updated = false;
  if(environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated) flush_variables(iface);
}

auto create_emulator_interface() -> Emulator::Interface* {
  auto* iface = new SuperFamicom::Interface;
  iface->set("Blur Emulation", false);
  return iface;
}

auto get_special_id_from_path(uint default_id, const char* path) -> uint {
  if(!path) return default_id;

  string p = path;
  if(p.endsWith(".gb") || p.endsWith(".gbc")) return SuperFamicom::ID::GameBoy;

  return default_id;
}

auto get_default_id_extension(uint id) -> string {
  //Super Game Boy can take .gbc as well if the game is dual-mode.
  //This path is only relevant as a fallback
  //in the rare case that we don't know the path.
  switch(id) {
  case SuperFamicom::ID::SuperFamicom: return ".sfc";
  case SuperFamicom::ID::GameBoy:      return ".gb";
  default: return "";
  }
}

auto load_special_bios(uint id) -> bool {
  if(id == SuperFamicom::ID::GameBoy) {
    //Find the Super Game Boy ROM here.
    string config_path = {sgb_path, "program.rom"};
    libretro_print(RETRO_LOG_INFO, "Trying to find %s in system directories.\n", config_path.data());
    string path = locate_libretro({sgb_path, "/program.rom"});

    if(!file::exists(path)) {
      libretro_print(RETRO_LOG_INFO, "Trying to find SGB.sfc/program.rom in system directories.\n");
      path = locate_libretro("SGB.sfc/program.rom");
    }

    if(!file::exists(path)) {
      libretro_print(RETRO_LOG_INFO, "Trying to find Super GameBoy.sfc/program.rom in system directories.\n");
      path = locate_libretro("Super GameBoy.sfc/program.rom");
    }

    if(!file::exists(path)) {
      libretro_print(RETRO_LOG_ERROR, "Need to find Super GameBoy BIOS, but could not find it.\n");
      return false;
    }

    program->medium_paths(SuperFamicom::ID::SuperFamicom) = Location::dir(path);
    program->loaded_manifest(SuperFamicom::ID::SuperFamicom) =
      plain_icarus.manifest(program->medium_paths(SuperFamicom::ID::SuperFamicom));

    if(!program->loaded_manifest(SuperFamicom::ID::SuperFamicom)) {
      libretro_print(RETRO_LOG_ERROR, "Could not create manifest for Super GameBoy BIOS.\n");
      return false;
    }

    return true;
  } else {
    return true;
  }
  unreachable;
}

#define RETRO_DEVICE_JOYPAD_MULTITAP       RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 0)
#define RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE  RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_LIGHTGUN, 0)
#define RETRO_DEVICE_LIGHTGUN_JUSTIFIER    RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_LIGHTGUN, 1)
#define RETRO_DEVICE_LIGHTGUN_JUSTIFIERS   RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_LIGHTGUN, 2)

auto set_environment_info(retro_environment_t cb) -> void {
  //TODO: Hook up RETRO_ENVIRONMENT_SET_SUBSYSTEM_INFO for Sufami/BSX/SGB?
  //IIRC, no known frontend actually hooks it up properly, so doubt there is any
  //real need for now.

  static const retro_controller_description port_1[] = {
    { "SNES Joypad", RETRO_DEVICE_JOYPAD },
    { "SNES Mouse", RETRO_DEVICE_MOUSE },
  };

  static const retro_controller_description port_2[] = {
    { "SNES Joypad", RETRO_DEVICE_JOYPAD },
    { "SNES Mouse", RETRO_DEVICE_MOUSE },
    { "Multitap", RETRO_DEVICE_JOYPAD_MULTITAP },
    { "SuperScope", RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE },
    { "Justifier", RETRO_DEVICE_LIGHTGUN_JUSTIFIER },
    { "Justifiers", RETRO_DEVICE_LIGHTGUN_JUSTIFIERS },
  };

  static const retro_controller_info ports[] = {
    { port_1, 2 },
    { port_2, 6 },
    { 0 },
  };

  cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, const_cast<retro_controller_info*>(ports));

  static const retro_input_descriptor desc[] = {
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,     "X" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,     "Y" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,     "L" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,     "R" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
    { 0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,     "X" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,     "Y" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,     "L" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,     "R" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
    { 1, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,     "X" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,     "Y" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,     "L" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,     "R" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
    { 2, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,     "X" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,     "Y" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,     "L" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,     "R" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
    { 3, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

    { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT,  "D-Pad Left" },
    { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP,    "D-Pad Up" },
    { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN,  "D-Pad Down" },
    { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right" },
    { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B,     "B" },
    { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A,     "A" },
    { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_X,     "X" },
    { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_Y,     "Y" },
    { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_L,     "L" },
    { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_R,     "R" },
    { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT,   "Select" },
    { 4, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START,    "Start" },

    { 0 },
  };

  cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, const_cast<retro_input_descriptor*>(desc));

  //Use "higan" instead of "nSide" in order to share settings with higan's own libretro core
  static const retro_variable vars[] = {
    { "higan_sfc_internal_resolution", "Internal resolution; 512x480|512x448|512x240|512x224|256x240|256x224" },
    { "higan_sfc_color_emulation", "Color emulation; OFF|ON" },
    { "higan_sfc_blur_emulation", "Blur emulation; OFF|ON" },
    { "higan_sfc_scanline_emulation", "Scanline emulation; OFF|ON" },
    { "higan_sfc_sgb_bios", "Preferred Super GameBoy BIOS (restart); SGB1.sfc/|SGB2.sfc/" },
    { nullptr },
  };
  cb(RETRO_ENVIRONMENT_SET_VARIABLES, const_cast<retro_variable*>(vars));
}

//Normally, these files are loaded from an external folder after higan is installed,
//but a libretro core shouldn't require that for anything but the Game Boy Advance BIOS.
//If any of these files are changed, please run sourcery inside the resources/ folder.
auto load_builtin_system_file(string name) -> vfs::shared::file {
  const vector<uint8_t>* resource;
       if(name == "manifest.bml") resource = &Resource::manifest;
  else if(name == "ipl.rom")      resource = &Resource::iplrom;
  else if(name == "boards.bml")   resource = &Resource::boards;
  return vfs::memory::file::open(resource->data(), resource->size());
}

auto Program::inputPoll(uint port, uint device, uint input) -> int16 {
  poll_once();

  //TODO: This will need to be remapped on a per-system basis.
  uint libretro_port;
  uint libretro_id;
  uint libretro_device;
  uint libretro_index = 0;

  static const uint joypad_mapping[] = {
    RETRO_DEVICE_ID_JOYPAD_UP,
    RETRO_DEVICE_ID_JOYPAD_DOWN,
    RETRO_DEVICE_ID_JOYPAD_LEFT,
    RETRO_DEVICE_ID_JOYPAD_RIGHT,
    RETRO_DEVICE_ID_JOYPAD_B,
    RETRO_DEVICE_ID_JOYPAD_A,
    RETRO_DEVICE_ID_JOYPAD_Y,
    RETRO_DEVICE_ID_JOYPAD_X,
    RETRO_DEVICE_ID_JOYPAD_L,
    RETRO_DEVICE_ID_JOYPAD_R,
    RETRO_DEVICE_ID_JOYPAD_SELECT,
    RETRO_DEVICE_ID_JOYPAD_START,
  };

  static const uint mouse_mapping[] = {
    RETRO_DEVICE_ID_MOUSE_X,
    RETRO_DEVICE_ID_MOUSE_Y,
    RETRO_DEVICE_ID_MOUSE_LEFT,
    RETRO_DEVICE_ID_MOUSE_RIGHT,
  };

  switch(port) {
  case SuperFamicom::ID::Port::Controller1: libretro_port = 0; break;
  case SuperFamicom::ID::Port::Controller2: libretro_port = 1; break;

  default:
    return 0;
  }

  switch(device) {
  case SuperFamicom::ID::Device::Gamepad:
    libretro_device = RETRO_DEVICE_JOYPAD;
    libretro_id = joypad_mapping[input];
    break;

  case SuperFamicom::ID::Device::Mouse:
    libretro_device = RETRO_DEVICE_MOUSE;
    libretro_id = mouse_mapping[input];
    break;

  case SuperFamicom::ID::Device::SuperMultitap:
    libretro_device = RETRO_DEVICE_JOYPAD;  //Maps to player [2, 5].
    libretro_port += input / 12;
    libretro_id = joypad_mapping[input % 12];
    break;

  //TODO: SuperScope/Justifiers.
  //Do we care? The v94 port hasn't hooked them up. :)

  default:
    return 0;
  }

  return input_state(libretro_port, libretro_device, libretro_index, libretro_id);
}

auto set_default_controller_ports() -> void {
  program->emulator->connect(SuperFamicom::ID::Port::Controller1, SuperFamicom::ID::Device::Gamepad);
  program->emulator->connect(SuperFamicom::ID::Port::Controller2, SuperFamicom::ID::Device::Gamepad);
}

auto set_controller_ports(unsigned port, unsigned device) -> void {
  switch(device) {
  default:
  case RETRO_DEVICE_NONE:                 device = SuperFamicom::ID::Device::None; break;
  case RETRO_DEVICE_JOYPAD:               device = SuperFamicom::ID::Device::Gamepad; break;
  case RETRO_DEVICE_ANALOG:               device = SuperFamicom::ID::Device::Gamepad; break;
  case RETRO_DEVICE_JOYPAD_MULTITAP:      device = SuperFamicom::ID::Device::SuperMultitap; break;
  case RETRO_DEVICE_MOUSE:                device = SuperFamicom::ID::Device::Mouse;
  case RETRO_DEVICE_LIGHTGUN_SUPER_SCOPE: device = SuperFamicom::ID::Device::SuperScope; break;
  case RETRO_DEVICE_LIGHTGUN_JUSTIFIER:   device = SuperFamicom::ID::Device::Justifier; break;
  case RETRO_DEVICE_LIGHTGUN_JUSTIFIERS:  device = SuperFamicom::ID::Device::Justifiers; break;
  }
  if(port < 2) program->emulator->connect(port, device);
}

auto get_memory_data(unsigned id) -> void* {
  switch(id) {

  case RETRO_MEMORY_SAVE_RAM: {
         if(SuperFamicom::cartridge.has.SA1)        return SuperFamicom::sa1.bwram.data();
    else if(SuperFamicom::cartridge.has.SuperFX)    return SuperFamicom::superfx.ram.data();
    else if(SuperFamicom::cartridge.has.HitachiDSP) return SuperFamicom::hitachidsp.ram.data();
    else if(SuperFamicom::cartridge.has.SPC7110)    return SuperFamicom::spc7110.ram.data();
    else if(SuperFamicom::cartridge.has.OBC1)       return SuperFamicom::obc1.ram.data();

    if(SuperFamicom::cartridge.ram.size()) {
      return SuperFamicom::cartridge.ram.data();
    } else if(SuperFamicom::cartridge.has.NECDSP
    && SuperFamicom::necdsp.revision == SuperFamicom::NECDSP::Revision::uPD96050) {
      return SuperFamicom::necdsp.dataRAM;
    }
  }

  case RETRO_MEMORY_SYSTEM_RAM: {
    return SuperFamicom::cpu.wram;
  }

  }

  return nullptr;
}

auto get_memory_size(unsigned id) -> size_t {
  switch(id) {

  case RETRO_MEMORY_SAVE_RAM: {
         if(SuperFamicom::cartridge.has.SA1)        return SuperFamicom::sa1.bwram.size();
    else if(SuperFamicom::cartridge.has.SuperFX)    return SuperFamicom::superfx.ram.size();
    else if(SuperFamicom::cartridge.has.HitachiDSP) return SuperFamicom::hitachidsp.ram.size();
    else if(SuperFamicom::cartridge.has.SPC7110)    return SuperFamicom::spc7110.ram.size();
    else if(SuperFamicom::cartridge.has.OBC1)       return SuperFamicom::obc1.ram.size();

    if(SuperFamicom::cartridge.ram.size()) {
      return SuperFamicom::cartridge.ram.size();
    } else if(SuperFamicom::cartridge.has.NECDSP
    && SuperFamicom::necdsp.revision == SuperFamicom::NECDSP::Revision::uPD96050) {
      return 2048 * sizeof(uint16);
    }
  }

  case RETRO_MEMORY_SYSTEM_RAM: {
    return 128 * 1024;
  }

  }

  return 0;
}

const char* BackendSpecific::extensions = "sfc|smc|gb|gbc|bml|rom";  //icarus supports headered ROMs as well.
const char* BackendSpecific::medium_type = "sfc";
#ifdef PROFILE_BALANCED
const char* BackendSpecific::name = "higan (Super Famicom Balanced)";
#else
const char* BackendSpecific::name = "higan (Super Famicom)";
#endif
const uint BackendSpecific::system_id = SuperFamicom::ID::System;
const double BackendSpecific::audio_rate = 44100.0;  //MSU1 is 44.1k CD, so use that.
