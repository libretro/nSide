namespace Heuristics {

struct Famicom {
  Famicom(vector<uint8_t>& data, string location);
  explicit operator bool() const;
  auto manifest() const -> string;

private:
  vector<uint8_t>& data;
  string location;
};

Famicom::Famicom(vector<uint8_t>& data, string location) : data(data), location(location) {
}

Famicom::operator bool() const {
  if(data.size() < 16) return false;
  if(data[0] != 'N') return false;
  if(data[1] != 'E') return false;
  if(data[2] != 'S') return false;
  if(data[3] !=  26) return false;
  if(data[7] & 0x01) return false;
  return true;
}

auto Famicom::manifest() const -> string {
  if(!operator bool()) return {};

  uint   mapper        = ((data[7] >> 4) << 4) | (data[6] >> 4);
  uint   mirror        = ((data[6] & 0x08) >> 2) | (data[6] & 0x01);
  uint   prgrom        = data[4] * 0x4000;
  uint   chrrom        = data[5] * 0x2000;
  uint   prgram        = 0;
  uint   chrram        = chrrom == 0 ? 8192 : 0;
  bool   iram_battery  = false;
  uint   iram_size     = 0;
  string iram_category = "";
  uint   iram          = 0;
  bool   battery       = data[6] & 0x02;
  bool   pc10          = false;
  bool   nes2          = (data[7] & 0x0c) == 0x08;
  uint   region        = 0;
  uint   submapper     = 0;

  if(!nes2) {
    //ignore the last 9 bytes of headers that have "DiskDude!" or other
    //messages written there
    if(data[12] == 0 && data[13] == 0 && data[14] == 0 && data[15] == 0) {
      prgram = data[8] * 0x2000;
      region = data[9] & 0x01;
    } else {
      mapper &= 0x0f;
    }
    pc10 = false;
  } else {
    pc10 = data[7] & 0x02;
    mapper |= (data[8] & 0x0f) << 8;
    submapper = data[8] >> 4;
    prgrom += (data[9] & 0x0f) * 0x400000;
    chrrom += (data[9] >>   4) * 0x200000;
    prgram  = ((data[10] & 0x0f) == 0 ? 0 : 64) << (data[10] & 0x0f);  //no battery
    prgram += ((data[10] >>   4) == 0 ? 0 : 64) << (data[10] >>   4);  //battery
    chrram  = ((data[11] & 0x0f) == 0 ? 0 : 64) << (data[11] & 0x0f);  //no battery
    chrram += ((data[11] >>   4) == 0 ? 0 : 64) << (data[11] >>   4);  //battery
    region = data[12] & 0x01;
  }

  string game;
  game.append("game\n");
  game.append("  sha256: ", Hash::SHA256(&data[16], data.size() - 16).digest(), "\n");
  game.append("  label:  ", Location::prefix(location), "\n");
  game.append("  name:   ", Location::prefix(location), "\n");
  game.append("  region: ", region == 0 ? "NTSC-J" : "PAL", "\n");

  string board;
  board.append("board region=", region == 0 ? "ntsc-j" : "pal", " ");

  static auto boardName = [&](string name) -> void {
    game.append("  board:  ", name, "\n");
    board.append("id:", name, "\n");
  };

  static auto boardLeaf = [&](string data) -> void {
    game.append("    ", data, "\n");
    board.append("  ", data, "\n");
  };

  static auto boardIRAM = [&](bool battery, uint size, string category) -> void {
    iram_battery = battery;
    iram_size = size;
    iram_category = category;
    board.append("    ram name=", category.downcase(), ".ram size=0x", hex(size), battery ? "\n" : "volatile\n");
  };

  switch(mapper) {
  default:
    if(prgram) {
      boardName("HVC-FAMILYBASIC");
    } else if(prgrom <= 0x2000) {
      boardName("NAMCOT-3301");
    } else if(prgrom <= 0x4000) {
      boardName("NES-NROM-128");
    } else {
      boardName("NES-NROM-256");
    }
    boardLeaf({"mirror mode=", mirror == 0 ? "horizontal" : "vertical"});
    break;

  case   1:
  case 155:
    if(prgram <= 0x2000) {
      if(submapper != 5 || prgrom >= 0x8000) {
        boardName(prgrom <= 0x40000 ? "NES-SNROM" : "NES-SUROM");
      } else {
        boardName("NES-SHROM");
      }
    } else {
      boardName(prgrom <= 0x40000 ? "NES-SOROM" : "HVC-SXROM");
    }
    boardLeaf({"chip type=MMC1", mapper != 155 ? "B2" : "A"});
    if(!nes2 && !prgram) prgram = 0x2000;
    break;

  case   2:
  case  94:
  case 180:
         if(mapper == 94)     boardName("HVC-UN1ROM");
    else if(prgrom <= 131072) boardName("NES-UNROM");
    else                      boardName("NES-UOROM");
    boardLeaf({"chip type=74HC", mapper != 180 ? "32" : "08"});
    boardLeaf({"mirror mode=", mirror == 0 ? "horizontal" : "vertical"});
    break;

  case   3:
  case 185:
    boardName("NES-CNROM");
    if(mapper == 185) {
      boardLeaf({"security pass=0x", hex(submapper & 3)});
    }
    boardLeaf({"mirror mode=", mirror == 0 ? "horizontal" : "vertical"});
    break;

  case   4:
  case 118:
  case 119:
    if(prgram != 0x400) {
      if(submapper != 3) {
        switch(mapper) {
        case   4:
          if(mirror & 2) boardName("NES-TR1ROM");
          else           boardName("NES-TLROM");
          break;
        case 118: boardName("NES-TLSROM"); break;
        case 119: boardName("NES-TQROM"); break;
        }
        boardLeaf({"chip type=MMC3B"});
      } else {
        boardLeaf({"chip type=ACCLAIM-MC-ACC"});
        boardLeaf({"chip type=MC-ACC"});
      }
    } else {
      boardName("NES-HKROM");
      boardLeaf({"chip type=MMC6n"});
    }
    if(!nes2 && !prgram) prgram = 0x2000;
    if(mapper == 119 && !nes2 && !chrram) chrram = 0x2000;
    break;

  case   5:
         if(prgram ==      0) boardName("NES-ELROM");
    else if(prgram <= 0x2000) boardName("NES-EKROM");
    else if(prgram == 0x8000) boardName("NES-EWROM");
    else if(prgram == 0x4000) boardName("NES-ETROM");
    boardLeaf("chip type=MMC5");
    boardIRAM(false, 0x400, "Internal");
    if(!nes2 && !prgram) prgram = 32768;
    break;

  case   7:
    boardName("NES-AOROM");
    break;

  case   9:
    boardName("NES-PNROM");
    boardLeaf("chip type=MMC2");
    if(!nes2 && !prgram) prgram = 0x2000;
    break;

  case  10:
    boardName("HVC-FKROM");
    boardLeaf("chip type=MMC4");
    if(!nes2 && !prgram) prgram = 0x2000;
    break;

  case  13:
    boardName("NES-CPROM");
    boardLeaf({"mirror mode=", mirror == 0 ? "horizontal" : "vertical"});
    break;

  case  16:
  case 153:
  case 159:
    switch(mapper) {
    case  16:
      boardName("BANDAI-FCG");
      boardLeaf("chip type=LZ93D50");
      boardLeaf("chip type=24C02");
      break;
    case 153:
      boardName("BANDAI-JUMP2");
      boardLeaf("chip type=LZ93D50");
      if(!nes2 && !prgram) prgram = 0x2000;
      break;
    case 159:
      boardName("BANDAI-LZ93D50");
      boardLeaf("chip type=LZ93D50");
      boardLeaf("chip type=24C01");
      break;
    }
    break;

  case  18:
    boardName("JALECO-JF-24");
    boardLeaf("chip type=SS88006");
    break;

  case  19:
  case 210:
    {
      string chip_type;
      switch(mapper) {
      case  19: chip_type = "163"; break;
      case 210: chip_type = submapper == 1 ? "175" : "340"; break;
      }
      boardName({"NAMCOT-", chip_type});
      boardLeaf({"chip type=", chip_type});
    }
    if(prgram == 0x80) {
      boardIRAM(true,  0x80, "Sound");
      prgram = 0;
    } else {
      boardIRAM(false, 0x80, "Sound");
    }
    if(mapper == 210 && submapper == 1) {
      boardLeaf({"mirror mode=", mirror == 0 ? "horizontal" : "vertical"});
    }
    break;

  case  21:  //VRC4a,VRC4c
  case  23:  //VRC4e,VRC4f,VRC2b
  case  25:  //VRC4b,VRC4d,VRC2c
    //VRC4f is not confirmed to exist.
    boardName("KONAMI-VRC-4");
    boardLeaf({"chip type=", submapper == 15 ? "VRC2" : "VRC4"});
    if(submapper == 0) {
      switch(mapper) {
      case 21: boardLeaf("  pinout a0=1 a1=2"); break;
      case 23: boardLeaf("  pinout a0=0 a1=1"); break;
      case 25: boardLeaf("  pinout a0=1 a1=0"); break;
      }
    } else if(submapper == 15) {
      boardLeaf({"  pinout",
        " a0=", submapper == 25,
        " a1=", submapper == 23,
      });
    } else {
      boardLeaf({"  pinout",
        " a0=", submapper & 7,
        " a1=", (submapper & 7) + (((submapper & 8) >> 2) - 1),
      });
    }
    if(!nes2 && !prgram) prgram = 0x2000;
    break;

  case  22:
    //VRC2a
    boardName("KONAMI-VRC-2");
    boardLeaf("chip type=VRC2");
    boardLeaf("  pinout a0=1 a1=0");
    break;

  case  24:
  case  26:
    boardName("KONAMI-VRC-6");
    boardLeaf("chip type=VRC6");
    switch(mapper) {
    case 24: boardLeaf("  pinout a0=0 a1=1"); break;
    case 26: boardLeaf("  pinout a0=1 a1=0"); break;
    }
    if(!nes2 && !prgram) prgram = 0x2000;
    break;

  case  32:
    boardName("IREM-G101");
    boardLeaf("chip type=G-101");
    if(submapper == 1) boardLeaf("mirror mode=screen-1");
    break;

  case  34:
    boardName("NES-BNROM");
    boardLeaf({"mirror mode=", mirror == 0 ? "horizontal" : "vertical"});
    break;

  case  65:
    boardName("IREM-H3001");
    boardLeaf("chip type=IF-H3001");
    break;

  case  66:
    boardName("NES-GNROM");
    boardLeaf({"mirror mode=", mirror == 0 ? "horizontal" : "vertical"});
    break;

  case  68:
    boardName("SUNSOFT-4");
    if(!nes2 && !prgram) prgram = 0x2000;
    break;

  case  69:
    boardName("SUNSOFT-5B");
    boardLeaf("chip type=5B");
    if(!nes2 && !prgram) prgram = 0x2000;
    break;

  case  70:
    boardName("BANDAI-74*161/161/32");
    boardLeaf({"mirror mode=", mirror == 0 ? "horizontal" : "vertical"});
    break;

  case  73:
    boardName("KONAMI-VRC-3");
    boardLeaf("chip type=VRC3");
    boardLeaf({"mirror mode=", mirror == 0 ? "horizontal" : "vertical"});
    if(!nes2 && !prgram) prgram = 0x2000;
    break;

  case  75:
    boardName("KONAMI-VRC-1");
    boardLeaf("chip type=VRC1");
    break;

  case  77:
    boardName("IREM-74*161/161/21/138");
    break;

  case  78:
    boardName(submapper != 3 ? "JALECO-JF-16" : "IREM-HOLYDIVER");
    break;

  case  85:
    boardName("KONAMI-VRC-7");
    boardLeaf("chip type=VRC7");
    boardLeaf("  pinout a0=4 a1=5");
    if(!nes2 && !prgram) prgram = 0x2000;
    break;

  case  86:
    boardName("JALECO-JF-13");
    boardLeaf("chip type=uPD7756C");
    boardLeaf({"mirror mode=", mirror == 0 ? "horizontal" : "vertical"});
    break;

  case  87:
    boardName("JALECO-JF-09");
    boardLeaf({"mirror mode=", mirror == 0 ? "horizontal" : "vertical"});
    break;

  case  97:
    boardName("IREM-TAM-S1");
    boardLeaf("chip type=TAM-S1");
    break;

  case 140:
    boardName("JALECO-JF-14");
    boardLeaf({"mirror mode=", mirror == 0 ? "horizontal" : "vertical"});
    break;

  case  76:
  case  88:
  case  95:
  case 154:
  case 206:
    switch(mapper) {
    case  76: boardName("NAMCOT-3446"); break;
    case  88: boardName("NAMCOT-3443"); break;
    case  95: boardName("NAMCOT-3425"); break;
    case 154: boardName("NAMCOT-3453"); break;
    case 206: boardName("NAMCOT-3401"); break;
    //Normally 3416, but バベルの塔 (Babel no Tou) uses bankswitching despite
    //fitting entirely in the memory map, and other 32KB programs are not
    //broken by having it enabled.
    }
    boardLeaf("chip type=Namcot108");
    boardLeaf({"mirror mode=", mirror == 0 ? "horizontal" : "vertical"});
    break;
  }

  #undef boardName

  board.append("  prg\n");
  if(prgrom) board.append("    rom name=program.rom size=0x", hex(prgrom), "\n");
  if(prgram) {
    if(battery) board.append("    ram name=save.ram size=0x", hex(prgram), "\n");
    else        board.append("    ram name=work.ram size=0x", hex(prgram), " volatile\n");
  }

  board.append("  chr\n");
  if(chrrom) board.append("    rom name=character.rom size=0x", hex(chrrom), "\n");
  if(chrram) board.append("    ram name=character.ram size=0x", hex(chrram), " volatile\n");

  if(pc10) {
    board.append("  pc10\n");
    board.append("    rom name=instructions.rom size=0x2000\n");
    board.append("    rom name=key.rom size=0x9\n");
  }

  if(prgrom) game.append(Memory{}.type("ROM").size(prgrom).content("Program").text());
  if(prgram) game.append(battery
  ? Memory{}.type("RAM").size(prgram).content("Save").text()
  : Memory{}.type("RAM").size(prgram).content("Save").isVolatile().text()
  );

  if(chrrom) game.append(Memory{}.type("ROM").size(chrrom).content("Character").text());
  if(chrram) game.append(Memory{}.type("RAM").size(chrram).content("Character").text());

  if(iram_size) game.append(iram_battery
  ? Memory{}.type("RAM").size(iram_size).content(iram_category).text()
  : Memory{}.type("RAM").size(iram_size).content(iram_category).isVolatile().text());

  if(pc10) {
    game.append(Memory{}.type("ROM").size(0x2000).content("Instructions").text());
    game.append(Memory{}.type("ROM").size(   0x9).content("Key").text());
  }

  return {board, "\n", game};
}

}
