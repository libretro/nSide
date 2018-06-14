PlayChoice10Interface::PlayChoice10Interface() {
  information.manufacturer = "Nintendo";
  information.name         = "PlayChoice-10";
  information.resettable   = false;

  media.append({ID::PlayChoice10, "PlayChoice-10", "pc10"});

  Port controllerPort1{ID::Port::Controller1, "Controller Port 1"};
  Port controllerPort2{ID::Port::Controller2, "Controller Port 2"};
  Port expansionPort{ID::Port::Expansion, "Expansion Port"};
  Port hardware{ID::Port::Hardware, "Hardware"};

  { Device device{ID::Device::Gamepad, "Gamepad"};
    device.inputs.append({0, "Up"   });
    device.inputs.append({0, "Down" });
    device.inputs.append({0, "Left" });
    device.inputs.append({0, "Right"});
    device.inputs.append({0, "B"    });
    device.inputs.append({0, "A"    });
    controllerPort1.devices.append(device);
    controllerPort2.devices.append(device);
  }

  { Device device{ID::Device::None, "None"};
    expansionPort.devices.append(device);
  }

  { Device device{ID::Device::BeamGun, "Zapper"};
    device.inputs.append({1, "X-axis" });
    device.inputs.append({1, "Y-axis" });
    device.inputs.append({0, "Trigger"});
    expansionPort.devices.append(device);
  }

  { Device device{ID::Device::PlayChoice10Controls, "PlayChoice-10 Controls"};
    device.inputs.append({0, "Game Select"   });
    device.inputs.append({0, "Start"         });
    device.inputs.append({0, "Channel Select"});
    device.inputs.append({0, "Enter"         });
    device.inputs.append({0, "Reset"         });
    device.inputs.append({0, "Service Button"});
    device.inputs.append({0, "Coin 1"        });
    device.inputs.append({0, "Coin 2"        });
    hardware.devices.append(device);
  }

  ports.append(move(controllerPort1));
  ports.append(move(controllerPort2));
  ports.append(move(expansionPort));
  ports.append(move(hardware));
}

auto PlayChoice10Interface::videoInformation() -> VideoInformation {
  double squarePixelRate = 135.0 / 22.0 * 1'000'000.0;

  VideoInformation vi;
  vi.width  = 256 / playchoice10.screenConfig;
  vi.height = playchoice10.screenConfig == PlayChoice10::ScreenConfig::Dual ? (240 + 224) / 2 : 240;
  vi.internalWidth  = 256;
  vi.internalHeight = 240 + 224 * (playchoice10.screenConfig - 1);
  vi.aspectCorrection = squarePixelRate / (system.frequency() / ppuM.rate());
  vi.refreshRate = system.frequency() / (ppuM.vlines() * ppuM.rate() * 341.0);
  return vi;
}

auto PlayChoice10Interface::videoColors() -> uint32 {
  return (1 << 9) + (1 << 8);
}

auto PlayChoice10Interface::videoColor(uint32 n) -> uint64 {
  static auto generateRGBColor = [](uint9 color, const uint9* palette) -> uint64 {
    uint3 r = color.bit(6) ? 7 : palette[color.bits(5,0)].bits(6,8);
    uint3 g = color.bit(7) ? 7 : palette[color.bits(5,0)].bits(3,5);
    uint3 b = color.bit(8) ? 7 : palette[color.bits(5,0)].bits(0,2);

    uint64 R = image::normalize(r, 3, 16);
    uint64 G = image::normalize(g, 3, 16);
    uint64 B = image::normalize(b, 3, 16);

    if(settings.colorEmulation) {
      //TODO: check how arcade displays alter the signal.
      //The red, green, and blue channels are connected directly without any
      //conversion to YIQ/YUV/YPbPr/etc. and back.
      static const uint8 gammaRamp[8] = {
        0x00, 0x0a,
        0x2d, 0x5b,
        0x98, 0xb8,
        0xe0, 0xff,
      };
      R = gammaRamp[r] * 0x0101;
      G = gammaRamp[g] * 0x0101;
      B = gammaRamp[b] * 0x0101;
    }

    return R << 32 | G << 16 | B << 0;
  };

  static auto generatePC10Color = [](uint9 color) -> uint64 {
    uint r = 15 - playchoice10.videoCircuit.cgrom[color + 0x000];
    uint g = 15 - playchoice10.videoCircuit.cgrom[color + 0x100];
    uint b = 15 - playchoice10.videoCircuit.cgrom[color + 0x200];

    uint64 R = image::normalize(r, 4, 16);
    uint64 G = image::normalize(g, 4, 16);
    uint64 B = image::normalize(b, 4, 16);

    if(settings.colorEmulation) {
      //TODO: check the menu monitor's gamma ramp
      static const uint8 gammaRamp[16] = {
        0x00, 0x03, 0x0a, 0x15,
        0x24, 0x37, 0x4e, 0x69,
        0x90, 0xa0, 0xb0, 0xc0,
        0xd0, 0xe0, 0xf0, 0xff,
      };
      R = gammaRamp[r] * 0x0101;
      G = gammaRamp[g] * 0x0101;
      B = gammaRamp[b] * 0x0101;
    }

    return R << 32 | G << 16 | B << 0;
  };

  if(n < (1 << 9)) {
    const uint9* palette = nullptr;
    switch(ppuM.version) {
    case PPU::Version::RP2C03B:
    case PPU::Version::RP2C03G:
    case PPU::Version::RC2C03B:
    case PPU::Version::RC2C03C:
    case PPU::Version::RC2C05_01:
    case PPU::Version::RC2C05_02:
    case PPU::Version::RC2C05_03:
    case PPU::Version::RC2C05_04:
    case PPU::Version::RC2C05_05:
      palette = PPU::RP2C03;
      break;
    case PPU::Version::RP2C04_0001:
      palette = PPU::RP2C04_0001;
      break;
    case PPU::Version::RP2C04_0002:
      palette = PPU::RP2C04_0002;
      break;
    case PPU::Version::RP2C04_0003:
      palette = PPU::RP2C04_0003;
      break;
    case PPU::Version::RP2C04_0004:
      palette = PPU::RP2C04_0004;
      break;
    }
    return generateRGBColor(n & 0x1ff, palette);
  } else {
    return generatePC10Color(n - (1 << 9));
  }
}

auto PlayChoice10Interface::load(uint id) -> bool {
  return system.load(this, System::Model::PlayChoice10);
}

auto PlayChoice10Interface::connect(uint port, uint device) -> void {
  if(port == ID::Port::Expansion) expansionPort.connect(settings.expansionPort = device);
}
