SG1000Interface::SG1000Interface() {
  information.manufacturer = "Sega";
  information.name         = "SG-1000";
  information.overscan     = true;

  media.append({ID::SG1000, "SG-1000", "sg1000"});

  Port controllerPort1{ID::Port::Controller1, "Controller Port 1"};
  Port controllerPort2{ID::Port::Controller2, "Controller Port 2"};
  Port hardware{ID::Port::Hardware, "Hardware"};

  { Device device{ID::Device::None, "None"};
    controllerPort1.devices.append(device);
    controllerPort2.devices.append(device);
  }

  { Device device{ID::Device::Gamepad, "Gamepad"};
    device.inputs.append({0, "Up"});
    device.inputs.append({0, "Down"});
    device.inputs.append({0, "Left"});
    device.inputs.append({0, "Right"});
    device.inputs.append({0, "1"});
    device.inputs.append({0, "2"});
    controllerPort1.devices.append(device);
    controllerPort2.devices.append(device);
  }

  { Device device{ID::Device::SG1000Controls, "Controls"};
    device.inputs.append({0, "Pause"});
    hardware.devices.append(device);
  }

  ports.append(move(controllerPort1));
  ports.append(move(controllerPort2));
  ports.append(move(hardware));
}

auto SG1000Interface::videoInformation() -> VideoInformation {
  double squarePixelRate = system.region() == System::Region::NTSC
  ? 135.0 / 22.0 * 1'000'000.0
  : 7'375'000.0;
  
  VideoInformation vi;
  vi.width  = 256;
  vi.height = 192;
  vi.internalWidth  = 256;
  vi.internalHeight = 192;
  vi.aspectCorrection = squarePixelRate / (system.colorburst() * 15.0 / 10.0);
  if(Region::NTSC()) vi.refreshRate = (system.colorburst() * 15.0 / 5.0) / (262.0 * 684.0);
  if(Region::PAL())  vi.refreshRate = (system.colorburst() * 15.0 / 5.0) / (313.0 * 684.0);
  return vi;
}

auto SG1000Interface::videoColors() -> uint32 {
  return 1 << 4;
}

auto SG1000Interface::videoColor(uint32 color) -> uint64 {
  double gamma = settings.colorEmulation ? 1.8 : 2.2;

  static double Y[] = {
    0.00, 0.00, 0.53, 0.67,
    0.40, 0.53, 0.47, 0.67,
    0.53, 0.67, 0.73, 0.80,
    0.46, 0.53, 0.80, 1.00,
  };
  static double Saturation[] = {
    0.000, 0.000, 0.267, 0.200,
    0.300, 0.267, 0.233, 0.300,
    0.300, 0.300, 0.233, 0.167,
    0.233, 0.200, 0.000, 0.000,
  };
  static uint Phase[] = {
      0,   0, 237, 235,
    354, 354, 114, 295,
    114, 114, 173, 173,
    235,  53,   0,   0,
  };
  double y = Y[color];
  double i = Saturation[color] * std::sin((Phase[color] - 33) * Math::Pi / 180.0);
  double q = Saturation[color] * std::cos((Phase[color] - 33) * Math::Pi / 180.0);

  auto gammaAdjust = [=](double f) -> double { return f < 0.0 ? 0.0 : std::pow(f, 2.2 / gamma); };
  //This matrix is from FCC's 1953 NTSC standard.
  //The SG-1000, ColecoVision, and MSX are older than the SMPTE C standard that followed in 1987.
  uint64 r = uclamp<16>(65535.0 * gammaAdjust(y +  0.946882 * i +  0.623557 * q));
  uint64 g = uclamp<16>(65535.0 * gammaAdjust(y + -0.274788 * i + -0.635691 * q));
  uint64 b = uclamp<16>(65535.0 * gammaAdjust(y + -1.108545 * i +  1.709007 * q));

  return r << 32 | g << 16 | b << 0;
}

auto SG1000Interface::load(uint id) -> bool {
  if(id == ID::SG1000) return system.load(this, System::Model::SG1000);
  return false;
}

auto SG1000Interface::connect(uint port, uint device) -> void {
  if(port == ID::Port::Controller1) controllerPort1.connect(settings.controllerPort1 = device);
  if(port == ID::Port::Controller2) controllerPort2.connect(settings.controllerPort2 = device);
}
