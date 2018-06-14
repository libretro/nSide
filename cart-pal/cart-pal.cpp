#include <nall/nall.hpp>
using namespace nall;

#include <hiro/hiro.hpp>
using namespace hiro;

auto locate(string name) -> string {
  //Use icarus's paths, as cart-pal has no differences in configuration settings
  string location = {Path::program(), name};
  if(inode::exists(location)) return location;

  location = {Path::config(), "icarus/", name};
  if(inode::exists(location)) return location;

  directory::create({Path::local(), "icarus/"});
  return {Path::local(), "icarus/", name};
}

#include "../icarus/settings.cpp"
Settings settings;

#include "../icarus/heuristics/heuristics.hpp"
#include "../icarus/heuristics/heuristics.cpp"
#include "heuristics/atari-2600.cpp"
#include "heuristics/famicom.cpp"
#include "../icarus/heuristics/super-famicom.cpp"
#include "../icarus/heuristics/master-system.cpp"
#include "heuristics/mega-drive.cpp"
#include "../icarus/heuristics/pc-engine.cpp"
#include "../icarus/heuristics/supergrafx.cpp"
#include "../icarus/heuristics/game-boy.cpp"
#include "../icarus/heuristics/game-boy-advance.cpp"
#include "../icarus/heuristics/game-gear.cpp"
#include "../icarus/heuristics/wonderswan.cpp"
#include "heuristics/vs-system.cpp"
#include "../icarus/heuristics/bs-memory.cpp"
#include "../icarus/heuristics/sufami-turbo.cpp"

#include "core/core.hpp"
#include "core/core.cpp"
#include "core/atari-2600.cpp"
#include "core/famicom.cpp"
#include "../icarus/core/super-famicom.cpp"
#include "core/sg-1000.cpp"
#include "../icarus/core/master-system.cpp"
#include "core/mega-drive.cpp"
#include "../icarus/core/pc-engine.cpp"
#include "../icarus/core/supergrafx.cpp"
#include "../icarus/core/game-boy.cpp"
#include "../icarus/core/game-boy-color.cpp"
#include "../icarus/core/game-boy-advance.cpp"
#include "../icarus/core/game-gear.cpp"
#include "../icarus/core/wonderswan.cpp"
#include "../icarus/core/wonderswan-color.cpp"
#include "../icarus/core/pocket-challenge-v2.cpp"
#include "core/vs-system.cpp"
#include "core/playchoice-10.cpp"
#include "core/famicombox.cpp"
#include "../icarus/core/bs-memory.cpp"
#include "../icarus/core/sufami-turbo.cpp"

#if !defined(ICARUS_LIBRARY)

CartPal cart_pal;
#include "../icarus/ui/ui.hpp"
#include "ui/scan-dialog.cpp"
#include "../icarus/ui/settings-dialog.cpp"
#include "ui/import-dialog.cpp"
#include "../icarus/ui/error-dialog.cpp"

#include <nall/main.hpp>
auto nall::main(string_vector args) -> void {
  Application::setName("cart-pal");

  if(args.size() == 2 && args[1] == "--name") {
    return print("cart-pal");
  }

  if(args.size() == 3 && args[1] == "--manifest" && directory::exists(args[2])) {
    return print(cart_pal.manifest(args[2]));
  }

  if(args.size() == 3 && args[1] == "--import" && file::exists(args[2])) {
    if(string target = cart_pal.import(args[2])) {
      return print(target, "\n");
    }
    return;
  }

  if(args.size() == 2 && args[1] == "--import") {
    if(string source = BrowserDialog()
    .setTitle("Load ROM File")
    .setPath(settings["icarus/Path"].text())
    .setFilters("ROM Files|"
      "*.a26:"
      "*.fc:*.nes:"
      "*.sfc:*.smc:"
      "*.sg1000:"
      "*.ms:*.sms:"
      "*.md:*.smd:*.gen:"
      "*.pce:"
      "*.sg:"
      "*.gb:"
      "*.gbc:"
      "*.gba:"
      "*.gg:"
      "*.ws:"
      "*.wsc:"
      "*.vs:"
      "*.pc10:"
      "*.fcb:"
      "*.bs:"
      "*.st:"
      "*.zip"
    ).openFile()) {
      if(string target = cart_pal.import(source)) {
        settings["icarus/Path"].setValue(Location::path(source));
        return print(target, "\n");
      }
    }
    return;
  }

  new ScanDialog;
  new SettingsDialog;
  new ImportDialog;
  new ErrorDialog;
  #if defined(PLATFORM_MACOS)
  Application::Cocoa::onAbout([&] {
    MessageDialog().setTitle("About cart-pal").setText({
      "cart-pal\n\n"
      "Based on icarus by byuu\n"
      "Famicom Additions Author: hex_usr\n"
      "License: GPLv3\n"
      "icarus Website: https://byuu.org/\n"
    }).information();
  });
  Application::Cocoa::onPreferences([&] {
    scanDialog->settingsButton.doActivate();
  });
  Application::Cocoa::onQuit([&] {
    Application::quit();
  });
  #endif
  scanDialog->show();
  Application::run();
  settings.save();
}

#endif
