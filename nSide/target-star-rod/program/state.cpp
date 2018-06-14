auto Program::stateName(uint slot) -> string {
  return {
    mediumPaths(1), "nSide/states/quick/",
    "slot-", slot, ".bst"
  };
}

auto Program::loadState(uint slot) -> bool {
  if(!emulator) return false;
  auto location = stateName(slot);
  auto memory = file::read(location);
  if(memory.size() == 0) return debugger->print("Slot ", slot, " does not exist\n"), false;
  serializer s(memory.data(), memory.size());
  if(emulator->unserialize(s) == false) {
    return debugger->print("Slot ", slot, " state incompatible", "\n"), false;
  }
  return debugger->print("Loaded state from ", location, "\n"), true;
}

auto Program::saveState(uint slot) -> bool {
  if(!emulator) return false;
  auto location = stateName(slot);
  serializer s = emulator->serialize();
  if(s.size() == 0) return debugger->print("Failed to save state to slot ", slot, "\n"), false;
  directory::create(Location::path(location));
  if(file::write(location, s.data(), s.size()) == false) {
    return debugger->print("Unable to write to slot ", slot, "\n"), false;
  }
  return debugger->print("Saved state to ", location, "\n"), true;
}
