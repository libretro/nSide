#include <a2600/a2600.hpp>

namespace Atari2600 {

Settings settings;

Interface::Interface() {
  information.manufacturer = "Atari";
  information.name         = "Atari 2600";
  information.overscan     = true;

  media.append({ID::Atari2600, "Atari 2600", "a26"});

  Port controllerPort1{ID::Port::Controller1, "Controller Port 1"};
  Port controllerPort2{ID::Port::Controller2, "Controller Port 2"};
  Port hardware{ID::Port::Hardware, "Hardware"};

  { Device device{ID::Device::None, "None"};
    controllerPort1.devices.append(device);
    controllerPort2.devices.append(device);
  }

  { Device device{ID::Device::Joystick, "Joystick"};
    device.inputs.append({0, "Up"   });
    device.inputs.append({0, "Down" });
    device.inputs.append({0, "Left" });
    device.inputs.append({0, "Right"});
    device.inputs.append({0, "Fire" });
    controllerPort1.devices.append(device);
    controllerPort2.devices.append(device);
  }

  { Device device{ID::Device::Controls, "Controls"};
    device.inputs.append({0, "Select"          });
    device.inputs.append({0, "Reset"           });
    device.inputs.append({0, "Color"           });
    device.inputs.append({0, "Left Difficulty" });
    device.inputs.append({0, "Right Difficulty"});
    hardware.devices.append(device);
  }

  ports.append(move(controllerPort1));
  ports.append(move(controllerPort2));
  ports.append(move(hardware));
}

auto Interface::manifest() -> string {
  return cartridge.manifest();
}

auto Interface::title() -> string {
  return cartridge.title();
}

auto Interface::videoInformation() -> VideoInformation {
  double squarePixelRate = Atari2600::Region::NTSC()
  ? 135.0 / 22.0 * 1'000'000.0
  : 7'375'000.0;
  uint assumedVLines = Atari2600::Region::NTSC() ? 262 : 312;

  VideoInformation vi;
  vi.width  = 160;
  vi.height = 228;
  vi.internalWidth  = 160;
  vi.internalHeight = 228;
  vi.aspectCorrection = squarePixelRate / system.frequency();
  vi.refreshRate = system.frequency() / (assumedVLines * 228.0);
  return vi;
}

auto Interface::videoColors() -> uint32 {
  return 1 << 7;
}

auto Interface::videoColor(uint32 n) -> uint64 {
  static auto generateNTSCColor = [](uint7 n, double hue, double gamma) -> uint64 {
    uint4 color = n.bits(3,6);
    uint3 level = n.bits(0,2);

    double y;
    double i;
    double q;

    //TODO: Determine if there is any special circuitry for when both the
    //luminosity and hue are 0 (black).
    if(color == 0 && level == 0) y = 0.0;
    else y = 0.125 + level / 7.0 * 0.875;

    if(color == 0) {
      i = 0.0;
      q = 0.0;
    } else {
      //hue 15 == hue 1:                   (360.0 / 14.0)°
      //hue 15 == (hue 1 + hue 2) / 2:     (360.0 / (14.0 - 1.0 / 2.0))°
      //hue 15 == (hue 1 + hue 2 * 2) / 3: (360.0 / (14.0 - 2.0 / 3.0))°
      static const double delay = (360.0 / (14.0 - 2.0 / 3.0)) * Math::Pi / 180.0;
      //phase shift delay only applies to colors 2-15
      double phase = Math::Pi + hue - (color - 1) * delay;
      i = std::sin(phase - 33.0 * Math::Pi / 180.0) * 0.25;
      q = std::cos(phase - 33.0 * Math::Pi / 180.0) * 0.25;
    }

    auto gammaAdjust = [=](double f) -> double { return f < 0.0 ? 0.0 : std::pow(f, 2.2 / gamma); };
    //This matrix is from FCC's 1953 NTSC standard.
    //The Atari 2600 is older than the SMPTE C standard that followed in 1987.
    uint64 r = uclamp<16>(65535.0 * gammaAdjust(y +  0.946882 * i +  0.623557 * q));
    uint64 g = uclamp<16>(65535.0 * gammaAdjust(y + -0.274788 * i + -0.635691 * q));
    uint64 b = uclamp<16>(65535.0 * gammaAdjust(y + -1.108545 * i +  1.709007 * q));

    return r << 32 | g << 16 | b << 0;
  };

  static auto generatePALColor = [](uint7 n, double gamma) -> uint64 {
    uint4 color = n.bits(3,6);
    uint3 level = n.bits(0,2);

    double y;
    double u;
    double v;

    //TODO: Determine the real formula for generating colors. The below formula
    //is a quick hack-up to match colors with publicly-available palettes.
    if((color.bits(1,3) == 0 || color.bits(1,3) == 7) && level == 0) y = 0.0;
    else y = 0.125 + level / 7.0 * 0.875;

    if(color.bits(1,3) == 0 || color.bits(1,3) == 7) {
      u = 0.0;
      v = 0.0;
    } else if(color.bit(0) == 0) {
      double phase = (180.0 - (color >> 1) * 30.0) * Math::Pi / 180.0;
      u = std::cos(phase) * 0.25;
      v = std::sin(phase) * 0.25;
    } else if(color.bit(0) == 1) {
      double phase = (165.0 + (color >> 1) * 30.0) * Math::Pi / 180.0;
      u = std::cos(phase) * 0.25;
      v = std::sin(phase) * 0.25;
    }

    auto gammaAdjust = [=](double f) -> double { return f < 0.0 ? 0.0 : std::pow(f, 2.2 / gamma); };
    uint64 r = uclamp<16>(65535.0 * gammaAdjust(y                 +  1.139837 * v));
    uint64 g = uclamp<16>(65535.0 * gammaAdjust(y + -0.394652 * u + -0.580599 * v));
    uint64 b = uclamp<16>(65535.0 * gammaAdjust(y +  2.032110 * u                ));

    return r << 32 | g << 16 | b << 0;
  };

  static auto generateSECAMColor = [](uint7 n, double gamma) -> uint64 {
    uint3 level = n.bits(0,2);

    //static const uint32 colors[] = {
    //  0xff000000, 0xff2121ff, 0xfff03c79, 0xffff50ff,
    //  0xff7fff50, 0xff7fffff, 0xffffff3f, 0xffffffff,
    //};
    static const double Y[] = {
       0.0000000000000000000,  0.2286588235294117800,
       0.4736235294117646700,  0.5971568627450980000,
       0.7716784313725490000,  0.8499137254901961000,
       0.9141647058823529000,  1.0000000000000000000,
    };
    static const double Db[] = {
       0.0000000000000000000,  1.1604941176470587000,
       0.0012274509803921196,  0.6059803921568627000,
      -0.6889215686274510000,  0.2258823529411764200,
      -1.0036705882352940000,  0.0000000000000000000,
    };
    static const double Dr[] = {
       0.0000000000000000000,  0.1889176470588235500,
      -0.8890313725490195000, -0.7658823529411765000,
       0.5201921568627452000,  0.6691137254901962000,
      -0.1633882352941175000,  0.0000000000000000000,
    };

    double y  = Y[level];
    double db = Db[level];
    double dr = Dr[level];

    //uint64 r = image::normalize(colors[level].byte(2), 8, 16);
    //uint64 g = image::normalize(colors[level].byte(1), 8, 16);
    //uint64 b = image::normalize(colors[level].byte(0), 8, 16);
    auto gammaAdjust = [=](double f) -> double { return f < 0.0 ? 0.0 : std::pow(f, 2.2 / gamma); };
    uint64 r = uclamp<16>(65535.0 * gammaAdjust(y +  0.000092303716148 * db + -0.525912630661865 * dr));
    uint64 g = uclamp<16>(65535.0 * gammaAdjust(y + -0.129132898890509 * db +  0.267899328207599 * dr));
    uint64 b = uclamp<16>(65535.0 * gammaAdjust(y +  0.664679059978955 * db + -0.000079202543533 * dr));

    return r << 32 | g << 16 | b << 0;
  };

  double gamma = settings.colorEmulation ? 1.8 : 2.2;
  if(system.region() == System::Region::NTSC) {
    return generateNTSCColor(n, 0.0, gamma);
  } else if(system.region() == System::Region::PAL) {
    return generatePALColor(n, gamma);
  } else if(system.region() == System::Region::SECAM) {
    return generateSECAMColor(n, gamma);
  }
}

auto Interface::loaded() -> bool {
  return system.loaded();
}

auto Interface::sha256() -> string {
  return cartridge.sha256();
}

auto Interface::load(uint id) -> bool {
  return system.load(this);
}

auto Interface::save() -> void {
  system.save();
}

auto Interface::unload() -> void {
  save();
  system.unload();
}

auto Interface::connect(uint port, uint device) -> void {
  if(port == ID::Port::Controller1) controllerPort1.connect(settings.controllerPort1 = device);
  if(port == ID::Port::Controller2) controllerPort2.connect(settings.controllerPort2 = device);
}

auto Interface::power() -> void {
  system.power();
}

auto Interface::run() -> void {
  system.run();
}

auto Interface::serialize() -> serializer {
  system.runToSave();
  return system.serialize();
}

auto Interface::unserialize(serializer& s) -> bool {
  return system.unserialize(s);
}

auto Interface::cheatSet(const string_vector& list) -> void {
  cheat.reset();
  cheat.assign(list);
}

auto Interface::cap(const string& name) -> bool {
  if(name == "Color Emulation") return true;
  if(name == "Scanline Emulation") return true;
  return false;
}

auto Interface::get(const string& name) -> any {
  if(name == "Color Emulation") return settings.colorEmulation;
  if(name == "Scanline Emulation") return settings.scanlineEmulation;
  return {};
}

auto Interface::set(const string& name, const any& value) -> bool {
  if(name == "Color Emulation" && value.is<bool>()) {
    settings.colorEmulation = value.get<bool>();
    system.configureVideoPalette();
    return true;
  }
  if(name == "Scanline Emulation" && value.is<bool>()) {
    settings.scanlineEmulation = value.get<bool>();
    system.configureVideoEffects();
    return true;
  }
  return false;
}

}
