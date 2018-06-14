namespace Heuristics {

struct VSSystem {
  VSSystem(vector<uint8_t>& data, string location);
  explicit operator bool() const;
  auto manifest() const -> string;

private:
  vector<uint8_t>& data;
  string location;
};

VSSystem::VSSystem(vector<uint8_t>& data, string location) : data(data), location(location) {
}

VSSystem::operator bool() const {
  if(data.size() < 16) return false;
  if(  data[0] != 'N' ) return false;
  if(  data[1] != 'E' ) return false;
  if(  data[2] != 'S' ) return false;
  if(  data[3] !=  26 ) return false;
  if(!(data[7] & 0x01)) return false;
  return true;
}

auto VSSystem::manifest() const -> string {
  if(!operator bool()) return {};

  uint mapper    = ((data[7] >> 4) << 4) | (data[6] >> 4);
  uint prgrom    = data[4] * 0x4000;
  uint chrrom    = data[5] * 0x2000;
  uint prgram    = 0u;
  uint chrram    = chrrom == 0 ? 8192 : 0;
  bool battery   = data[6] & 0x02;
  bool nes2      = (data[7] & 0x0c) == 0x08;
  uint region    = 0;
  uint submapper = 0;
  uint ppu       = 0;

  if(!nes2) {
    //ignore the last 9 bytes of headers that have "DiskDude!" or other
    //messages written there
    if(data[12] == 0 && data[13] == 0 && data[14] == 0 && data[15] == 0) {
      prgram = data[8] * 0x2000;
      region = data[9] & 0x01;
    } else {
      return {};
    }
  } else {
    mapper |= (data[8] & 0x0f) << 8;
    submapper = data[8] >> 4;
    prgrom += (data[9] & 0x0f) * 0x400000;
    chrrom += (data[9] >>   4) * 0x200000;
    prgram  = ((data[10] & 0x0f) == 0 ? 0 : 64) << (data[10] & 0x0f);  //no battery
    prgram += ((data[10] >>   4) == 0 ? 0 : 64) << (data[10] >>   4);  //battery
    chrram  = ((data[11] & 0x0f) == 0 ? 0 : 64) << (data[11] & 0x0f);  //no battery
    chrram += ((data[11] >>   4) == 0 ? 0 : 64) << (data[11] >>   4);  //battery
    region = data[12] & 0x01;
    ppu = data[13] & 0x0f;
  }

  if(prgram >= 0x800) prgram -= 0x800;  //VS. System built-in RAM

  string ppu_revision = "";
  switch(ppu) {
  case  0: ppu_revision = "RP2C03B"; break;
  case  1: ppu_revision = "RP2C03G"; break;
  case  2: ppu_revision = "RP2C04-0001"; break;
  case  3: ppu_revision = "RP2C04-0002"; break;
  case  4: ppu_revision = "RP2C04-0003"; break;
  case  5: ppu_revision = "RP2C04-0004"; break;
  case  6: ppu_revision = "RC2C03B"; break;
  case  7: ppu_revision = "RC2C03C"; break;
  case  8: ppu_revision = "RC2C05-01"; break;
  case  9: ppu_revision = "RC2C05-02"; break;
  case 10: ppu_revision = "RC2C05-03"; break;
  case 11: ppu_revision = "RC2C05-04"; break;
  case 12: ppu_revision = "RC2C05-05"; break;
  }

  string game;
  game.append("game\n");
  game.append("  sha256: ", Hash::SHA256(&data[16], data.size() - 16).digest(), "\n");
  game.append("  label:  ", Location::prefix(location), "\n");
  game.append("  name:   ", Location::prefix(location), "\n");
  game.append("  region: ", region == 0 ? "NTSC-J" : "PAL", "\n");

  string side;
  side.append("side\n");
  side.append("  ppu revision=", ppu_revision, "\n");
  side.append("  controller port=2 device=gamepad\n");
  side.append("  controller port=1 device=gamepad\n");

  static auto boardName = [&](string name) -> void {
    game.append("  board:  ", name, "\n");
  };

  switch(mapper) {
  default:  //case  99:
    game.append("  board\n");  //No daughterboard
    break;

  case   1:  //SxROM-like
    boardName("MDS-VS1-01");  //Nintendo
    side.append("  chip type=MMC1B2\n");
    break;

  case   2:  //UxROM-like
    boardName("350735");  //Konami
    side.append("  chip type=74LS32\n");
    break;

  case 206:  //NAMCOT-34xx-like
    boardName("8605194200");  //Namco
    side.append("  chip type=108\n");
    break;
  }

  side.append("  prg\n");
  if(prgrom) side.append("    rom name=program.rom size=0x", hex(prgrom), "\n");
  if(prgram) {
    if(battery) side.append("    ram name=save.ram size=0x", hex(prgram), "\n");
    else        side.append("    ram name=work.ram size=0x", hex(prgram), " volatile\n");
  }

  side.append("  chr\n");
  if(chrrom) side.append("    rom name=character.rom size=0x", hex(chrrom), "\n");
  if(chrram) side.append("    ram name=character.ram size=0x", hex(chrram), " volatile\n");

  if(prgrom) game.append(Memory{}.type("ROM").size(prgrom).content("Program").text());
  if(prgram) game.append(battery
  ? Memory{}.type("RAM").size(prgram).content("Save").text()
  : Memory{}.type("RAM").size(prgram).content("Save").isVolatile().text()
  );

  if(chrrom) game.append(Memory{}.type("ROM").size(chrrom).content("Character").text());
  if(chrram) game.append(Memory{}.type("RAM").size(chrram).content("Character").text());

  return {side, "\n", game};
}

}
