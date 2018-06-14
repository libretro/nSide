auto Program::stateName(uint slot, bool managed) -> string {
  return {
    mediumPaths(1), "nSide/states/",
    managed ? "managed/" : "quick/",
    "slot-", slot, ".bst"
  };
}

auto Program::loadState(uint slot, bool managed) -> bool {
  if(!emulator) return false;
  string type = locale[{"Status/State/", managed ? "managed" : "quick"}];
  auto location = stateName(slot, managed);
  auto memory = file::read(location);
  if(memory.size() == 0) {
    showMessage(locale["Status/State/DoesNotExist"].replace("%1$s", type).replace("%2$d", slot));
    return false;
  }
  serializer s(memory.data(), memory.size());
  if(emulator->unserialize(s) == false) {
    showMessage(locale["Status/State/Incompatible"].replace("%1$s", type).replace("%2$d", slot));
    return false;
  }
  showMessage(locale["Status/State/Load"].replace("%1$s", type).replace("%2$d", slot));
  return true;
}

auto Program::saveState(uint slot, bool managed) -> bool {
  if(!emulator) return false;
  string type = locale[{"Status/State/", managed ? "managed" : "quick"}];
  auto location = stateName(slot, managed);
  serializer s = emulator->serialize();
  if(s.size() == 0) {
    showMessage(locale["Status/State/Unsupported"].replace("%1$s", type).replace("%2$d", slot));
    return false;
  }
  directory::create(Location::path(location));
  if(file::write(location, s.data(), s.size()) == false) {
    showMessage(locale["Status/State/Unprivileged"].replace("%1$s", type).replace("%2$d", slot));
    return false;
  }
  showMessage(locale["Status/State/Save"].replace("%1$s", type).replace("%2$d", slot));
  return true;
}
