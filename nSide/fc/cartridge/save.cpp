auto Cartridge::saveCartridge() -> void {
  if(auto node = boardNode["prg/ram"]) saveMemory(board->prgram, node);
  if(auto node = boardNode["chr/ram"]) saveMemory(board->chrram, node);
  if(board->chip) {
    if(auto node = boardNode["chip/ram"]) saveMemory(board->chip->ram, node);
  }
}

//

auto Cartridge::saveMemory(MappedRAM& ram, Emulator::Game::Memory memory, maybe<uint> id) -> void {
  if(!id) id = pathID();
  if(memory.type == "RAM" && !memory.nonVolatile) return;
  if(auto fp = platform->open(id(), memory.name(), File::Write)) {
    fp->write(ram.data(), ram.size());
  }
}

auto Cartridge::saveMemory(MappedRAM& ram, Markup::Node node, maybe<uint> id) -> void {
  if(!id) id = pathID();
  if(auto memory = game.memory(node)) {
    saveMemory(ram, *memory, id);
  } else if(node["name"] && node["size"]) {
    if(node["volatile"]) return;
    if(auto fp = platform->open(id(), node["name"].text(), File::Write)) {
      fp->write(ram.data(), ram.size());
    }
  }
}
