auto Cartridge::loadBoard(string board) -> Markup::Node {
  if(board.beginsWith("NES-")) board.replace("NES-", "HVC-", 1L);
  if(board.beginsWith("PAL-")) board.replace("PAL-", "HVC-", 1L);

  if(auto fp = platform->open(ID::System, "boards.bml", File::Read, File::Required)) {
    auto document = BML::unserialize(fp->reads());
    for(auto leaf : document.find("board")) {
      auto id = leaf.text();
      bool matched = id == board;
      if(!matched && id.match("*(*)*")) {
        auto part = id.transform("()", "||").split("|");
        for(auto& revision : part(1).split(",")) {
          if(string{part(0), revision, part(2)} == board) matched = true;
        }
      }
      if(matched) return leaf;
    }
  }

  return {};
}

auto Cartridge::loadCartridge() -> void {
  if(Model::VSSystem()) {
    vssystem.gameCount = 0;
    auto sides = game.document.find("side");
    if(sides.size() == 2) cartridgeSlot[0].setupVS(sides(0), pathID());
    cartridgeSlot[1].setupVS(sides(sides.size() - 1), pathID());
    return;
  }

  if(game.document["board"]) {
    boardNode = game.document["board"];

    if(region() == "Auto") {
      if(boardNode["region"].text() == "ntsc-j") information.region = "NTSC-J";
      if(boardNode["region"].text() == "ntsc-u") information.region = "NTSC-U";
      if(boardNode["region"].text() == "pal") information.region = "PAL";
      if(boardNode["region"].text() == "dendy") information.region = "Dendy";
    }

    static auto convertMemory = [&](string query,
      string type,
      string content,
      string manufacturer = "",
      string architecture = ""
    ) -> void {
      if(auto memory = boardNode[query]) {
        string prefix = {query.split("/").left(), "/"};
        string compare = {prefix, "memory(type=", type, ",content=", content, ")"};
        if(architecture) compare.replace(")", {",architecture=", architecture, ")"});
        if(boardNode[compare]) return;
        memory.setName("memory");
        memory.append(Markup::Node("type",    type));
        memory.append(Markup::Node("content", content));
        if(manufacturer) memory.append(Markup::Node("manufacturer", manufacturer));
        if(architecture) memory.append(Markup::Node("architecture", architecture));
      }
    };

    convertMemory("prg/rom", "ROM", "Program");
    convertMemory("prg/ram", "RAM", boardNode["prg/ram/volatile"] ? "Work" : "Save");
    convertMemory("chr/rom", "ROM", "Character");
    convertMemory("chr/ram", "RAM", "Character");
    convertMemory("chip/ram", "RAM", "Internal", "", boardNode["chip/type"].text().upcase());

    if(Model::PlayChoice10()) {
      //These 2 lines must run in order
      convertMemory("pc10/rom", "ROM", "Instructions");
      convertMemory("pc10/rom", "ROM", "Key");
    }
  } else {
    boardNode = loadBoard(game.board);

    if(region() == "Auto") {
      auto region = game.region;
      if(region.endsWith("CAN")
      || region.endsWith("USA")
      || region == "NTSC-U") {
        information.region = "NTSC-U";
      } else
      if(region.endsWith("EEC")
      || region.endsWith("ESP")
      || region.endsWith("FRA")
      || region.endsWith("FRG")
      || region.endsWith("GBR")
      || region.endsWith("NOE")
      || region.endsWith("SCN")
      || region.endsWith("UKV")
      || region == "PAL") {
        information.region = "PAL";
      } else if(region == "Dendy") {
        information.region = "Dendy";
      } else {
        information.region = "NTSC-J";
      }
    }
  }

  if(boardNode["mirror"] && !boardNode["mirror/mode"]) {
    boardNode["mirror"].append(game.document["game/board/mirror/mode"]);
  }

  board = Board::load(boardNode);
  if(!board) return;
  board->slot = slot;
  if(auto node = boardNode["prg/memory(type=ROM)"]) loadMemory(board->prgrom, node, File::Required, pathID());
  if(auto node = boardNode["prg/memory(type=RAM)"]) loadMemory(board->prgram, node, File::Optional, pathID());
  if(auto node = boardNode["chr/memory(type=ROM)"]) loadMemory(board->chrrom, node, File::Required, pathID());
  if(auto node = boardNode["chr/memory(type=RAM)"]) loadMemory(board->chrram, node, File::Optional, pathID());
  if(board->chip) {
    if(auto node = boardNode["chip/memory(type=RAM)"]) loadMemory(board->chip->ram, node, File::Optional, pathID());
  }

  if(Model::PlayChoice10()) {
    auto instrom = boardNode["pc10/memory(type=ROM,content=Instructions)"];
    auto keyrom  = boardNode["pc10/memory(type=ROM,content=Key)"];
    if(instrom) loadMemory(board->instrom, instrom, File::Required, pathID());
    if(keyrom)  loadMemory(board->keyrom,  keyrom,  File::Required, pathID());
  }
}

auto Cartridge::setupVS(Markup::Node& side, uint _pathID) -> void {
  if(slot == 0) vssystem.forceSubRAM = side["cpu/version"].text() == "RP2A04";

  if(!side["ppu"]) return;

  vssystem.gameCount++;
  if(side["setting"]) vssystem.setDip(slot, platform->dipSettings(side));

  information.pathID = _pathID;
  information.region = "NTSC-U";

  auto& ppu = slot ? ppuS : ppuM;
  string ppuVersion = side["ppu/version"].text();
  if(ppuVersion == "RP2C02C")     ppu.version = PPU::Version::RP2C02C;
  if(ppuVersion == "RP2C02G")     ppu.version = PPU::Version::RP2C02G;
  if(ppuVersion == "RP2C03B")     ppu.version = PPU::Version::RP2C03B;
  if(ppuVersion == "RP2C03G")     ppu.version = PPU::Version::RP2C03G;
  if(ppuVersion == "RP2C04-0001") ppu.version = PPU::Version::RP2C04_0001;
  if(ppuVersion == "RP2C04-0002") ppu.version = PPU::Version::RP2C04_0002;
  if(ppuVersion == "RP2C04-0003") ppu.version = PPU::Version::RP2C04_0003;
  if(ppuVersion == "RP2C04-0004") ppu.version = PPU::Version::RP2C04_0004;
  if(ppuVersion == "RC2C03B")     ppu.version = PPU::Version::RC2C03B;
  if(ppuVersion == "RC2C03C")     ppu.version = PPU::Version::RC2C03C;
  if(ppuVersion == "RC2C05-01")   ppu.version = PPU::Version::RC2C05_01;
  if(ppuVersion == "RC2C05-02")   ppu.version = PPU::Version::RC2C05_02;
  if(ppuVersion == "RC2C05-03")   ppu.version = PPU::Version::RC2C05_03;
  if(ppuVersion == "RC2C05-04")   ppu.version = PPU::Version::RC2C05_04;
  if(ppuVersion == "RC2C05-05")   ppu.version = PPU::Version::RC2C05_05;

  board = Board::load(side);
  if(!board) return;
  board->slot = slot;
  if(auto node = side["prg/rom"]) loadMemory(board->prgrom, node, File::Required, pathID());
  if(auto node = side["prg/ram"]) loadMemory(board->prgram, node, File::Optional, pathID());
  if(auto node = side["chr/rom"]) loadMemory(board->chrrom, node, File::Required, pathID());
  if(auto node = side["chr/ram"]) loadMemory(board->chrram, node, File::Optional, pathID());
  if(board->chip) {
    if(auto node = side["chip/ram"]) loadMemory(board->chip->ram, node, File::Optional, pathID());
  }
}

//

auto Cartridge::loadMemory(MappedRAM& ram, Emulator::Game::Memory memory, bool required, maybe<uint> id) -> void {
  if(!id) id = pathID();
  ram.allocate(memory.size);
  if(memory.type == "RAM" && !memory.nonVolatile) return;
  if(auto fp = platform->open(id(), memory.name(), File::Read, required)) {
    fp->read(ram.data(), ram.size());
  }
}

auto Cartridge::loadMemory(MappedRAM& ram, Markup::Node node, bool required, maybe<uint> id) -> void {
  if(auto memory = game.memory(node)) {
    loadMemory(ram, *memory, required, id);
  } else if(node["name"] && node["size"]) {
    ram.allocate(node["size"].natural());
    if(auto fp = platform->open(id(), node["name"].text(), File::Read, required)) {
      fp->read(ram.data(), ram.size());
    }
  }
}
