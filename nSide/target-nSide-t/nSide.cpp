#include "nSide.hpp"
unique_pointer<Video> video;
unique_pointer<Audio> audio;
unique_pointer<Input> input;
Emulator::Interface* emulator = nullptr;

auto locate(string name) -> string {
  string location = {Path::program(), name};
  if(inode::exists(location)) return location;

  location = {Path::config(), "nSide/", name};
  if(inode::exists(location)) return location;

  location = {Path::local(), "nSide/", name};
  if(inode::exists(location)) return location;

  location = {Path::config(), "higan/", name};
  if(inode::exists(location)) {
    if(Location::suffix(location) == ".bml" && Location::suffix(Location::dir(location)) != ".sys") {
      directory::create({Path::config(), "nSide/"});
      file::copy(location, {Path::config(), "nSide/", name});
      return {Path::config(), "nSide/", name};
    }
    return location;
  }

  location = {Path::local(), "higan/", name};
  if(inode::exists(location)) {
    if(Location::suffix(location) == ".bml" && Location::suffix(Location::dir(location)) != ".sys") {
      directory::create({Path::local(), "nSide/"});
      file::copy(location, {Path::local(), "nSide/", name});
      return {Path::local(), "nSide/", name};
    }
    return location;
  }

  directory::create({Path::local(), "nSide/"});
  return {Path::local(), "nSide/", name};
}

auto locateSystem(string name) -> string {
  string location = {settings["Library/Location"].text(), "systems/", name};
  if(inode::exists(location)) return location;

  return locate({"systems/", name});
}

#include <nall/main.hpp>
auto nall::main(string_vector args) -> void {
  Application::setName("nSide");
  new Program(args);
  Application::run();
}
