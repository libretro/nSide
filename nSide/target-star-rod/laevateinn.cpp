#include "laevateinn.hpp"
#include "resource/resource.cpp"
unique_pointer<Video> video;
unique_pointer<Audio> audio;
unique_pointer<Input> input;
Emulator::Interface* emulator = nullptr;

auto locate(string name) -> string {
  string location = {Path::program(), "star-rod-", name};
  if(inode::exists(location)) return location;

  location = {Path::config(), "star-rod/", name};
  if(inode::exists(location)) return location;

  directory::create({Path::local(), "star-rod/"});
  return {Path::local(), "star-rod/", name};
}

auto locateHigan(string name) -> string {
  string location = {Path::program(), name};
  if(inode::exists(location)) return location;

  location = {Path::config(), "nSide/", name};
  if(inode::exists(location)) return location;

  return {Path::local(), "nSide/", name};
}

auto locateSystem(string name) -> string {
  string location = {program->higan_settings["Library/Location"].text(), "systems/", name};
  if(inode::exists(location)) return location;

  return locate(name);
}

#include <nall/main.hpp>
auto nall::main(string_vector args) -> void {
  Application::setName("star-rod");
  new Program(args);
  Application::run();
}
