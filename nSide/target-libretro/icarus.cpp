//TODO: find a way to split the code into 4 linker objects without having
//icarus's Database variables complain of multiple definitions
//#include "libretro.hpp"

auto locate(string name) -> string {
  string location = {Path::program(), name};
  if(inode::exists(location)) return location;

  directory::create({Path::local(), "icarus/"});
  return {Path::local(), "icarus/", name};
}

#include "../../icarus/heuristics/heuristics.cpp"
#include "../../icarus/heuristics/famicom.cpp"
#include "../../icarus/heuristics/super-famicom.cpp"
#include "../../icarus/heuristics/master-system.cpp"
#include "../../icarus/heuristics/mega-drive.cpp"
#include "../../icarus/heuristics/pc-engine.cpp"
#include "../../icarus/heuristics/supergrafx.cpp"
#include "../../icarus/heuristics/game-boy.cpp"
#include "../../icarus/heuristics/game-boy-advance.cpp"
#include "../../icarus/heuristics/game-gear.cpp"
#include "../../icarus/heuristics/wonderswan.cpp"
#include "../../icarus/heuristics/bs-memory.cpp"
#include "../../icarus/heuristics/sufami-turbo.cpp"

#include "../../icarus/core/core.cpp"
#include "../../icarus/core/famicom.cpp"
#include "../../icarus/core/super-famicom.cpp"
#include "../../icarus/core/master-system.cpp"
#include "../../icarus/core/mega-drive.cpp"
#include "../../icarus/core/pc-engine.cpp"
#include "../../icarus/core/supergrafx.cpp"
#include "../../icarus/core/game-boy.cpp"
#include "../../icarus/core/game-boy-color.cpp"
#include "../../icarus/core/game-boy-advance.cpp"
#include "../../icarus/core/game-gear.cpp"
#include "../../icarus/core/wonderswan.cpp"
#include "../../icarus/core/wonderswan-color.cpp"
#include "../../icarus/core/pocket-challenge-v2.cpp"
#include "../../icarus/core/bs-memory.cpp"
#include "../../icarus/core/sufami-turbo.cpp"

Settings::Settings() {
  Markup::Node::operator=(BML::unserialize(string::read(locate("settings.bml"))));

  auto set = [&](const string& name, const string& value) {
    //create node and set to default value only if it does not already exist
    if(!operator[](name)) operator()(name).setValue(value);
  };

  set("Library/Location", {Path::user(), "Emulation/"});

  set("icarus/Path", Path::user());
  set("icarus/CreateManifests", true);
  set("icarus/UseDatabase", true);
  set("icarus/UseHeuristics", true);
}

auto Settings::save() -> void {
}

//Stub out most of this. libretro uses a memory-only interface.
auto LibretroIcarus::create(const string&) -> bool {
  return true;
}

auto LibretroIcarus::copy(const string&, const string&) -> bool {
  return true;
}

auto LibretroIcarus::exists(const string&) -> bool {
  return true;
}

auto LibretroIcarus::directory_exists(const string&) -> bool {
  return true;
}

auto LibretroIcarus::readable(const string&) -> bool {
  return true;
}

auto LibretroIcarus::write(const string& filename, const uint8_t* data, uint size) -> bool {
  //To avoid any directories in the mix in case icarus wants to write to ~/Emulation or similar.
  string imported_path = Location::file(filename);
  vector<uint8_t> imported_data;

  imported_data.resize(size);
  memory::copy(imported_data.data(), data, size);
  imported_files.insert(imported_path, std::move(imported_data));
  libretro_print(RETRO_LOG_INFO, "Icarus writes file: %s.\n", imported_path.data());
  return true;
}

auto LibretroIcarus::read(const string& pathname) -> vector<uint8_t> {
  auto result = imported_files.find(pathname);
  if(result) {
    libretro_print(RETRO_LOG_INFO, "Icarus successfully read: %s.\n", pathname.data());
    return result.get();
  } else {
    libretro_print(RETRO_LOG_ERROR, "Icarus failed to read: %s.\n", pathname.data());
    return {};
  }
}

auto LibretroIcarus::reset() -> void {
  imported_files.reset();
}

auto LibretroIcarus::import_rom(const string& fake_path, const uint8_t* rom_data, size_t rom_size) -> bool {
  reset();
  write(fake_path, rom_data, rom_size);

  if(import(fake_path)) return true;

  auto missing_files = missing();
  if(!missing_files) return false;

  //If there are missing files, try to load them from disk, and append them in order to the ROM data in question.
  //If that fails, bail out.
  for(auto& rom_name : missing_files) {
    libretro_print(RETRO_LOG_INFO, "ROM did not include missing file: %s.\n", rom_name.data());

    auto& file = imported_files.find(fake_path).get();
    string path = locate_libretro(rom_name);
    auto data = file::read(path);
    if(!data) {
      libretro_print(RETRO_LOG_ERROR, "Could not find missing file: %s.\n", rom_name.data());
      return false;
    }

    libretro_print(RETRO_LOG_INFO, "Found missing ROM in: %s.\n", path.data());

    size_t offset = file.size();
    file.resize(offset + data.size());
    memory::copy(file.data() + offset, data.data(), data.size());
  }

  //Try again.
  return import(fake_path);
}
