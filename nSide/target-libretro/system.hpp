auto adjust_video_resolution(uint& width, uint& height, uint& pitch, float& pixel_aspect_correction) -> uint;
auto video_output(const uint32* data, uint width, uint height, uint pitch) -> void;
auto flush_variables(Emulator::Interface* iface) -> void;
auto check_variables(Emulator::Interface* iface) -> void;
auto create_emulator_interface() -> Emulator::Interface*;
auto get_special_id_from_path(uint default_id, const char* path) -> uint;
auto get_default_id_extension(uint id) -> string;
auto load_special_bios(uint id) -> bool;
auto set_environment_info(retro_environment_t cb) -> void;
auto load_builtin_system_file(string name) -> vfs::shared::file;
auto set_default_controller_ports() -> void;
auto set_controller_ports(unsigned port, unsigned device) -> void;
auto get_memory_data(unsigned id) -> void*;
auto get_memory_size(unsigned id) -> size_t;

namespace BackendSpecific {
  extern const char* extensions;
  extern const char* medium_type;
  extern const char* name;
  extern const uint system_id;
  extern const double audio_rate;
}
