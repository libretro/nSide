auto Program::stateName(uint slot) -> string {
  return {
    mediumPaths(1), Emulator::ForkName, "/states/quick/",
    "slot-", slot, ".bst"
  };
}

auto Program::loadState(uint slot) -> bool {
  if(!emulator) return false;
  auto location = stateName(slot);
  auto memory = file::read(location);
  if(memory.size() == 0) return false;
  serializer s(memory.data(), memory.size());
  if(emulator->unserialize(s) == false) return false;
  return true;
}

auto Program::saveState(uint slot) -> bool {
  if(!emulator) return false;
  auto location = stateName(slot);
  serializer s = emulator->serialize();
  if(s.size() == 0) return false;
  directory::create(Location::path(location));
  if(file::write(location, s.data(), s.size()) == false) {
    return false;
  }
  return true;
}
