auto CartPal::vsSystemManifest(string location) -> string {
  vector<uint8_t> buffer;
  concatenate(buffer, {location, "ines.rom"});
  concatenate(buffer, {location, "program.rom"});
  concatenate(buffer, {location, "character.rom"});
  concatenate(buffer, {location, "program-1.rom"});
  concatenate(buffer, {location, "character-1.rom"});
  concatenate(buffer, {location, "program-2.rom"});
  concatenate(buffer, {location, "character-2.rom"});
  return vsSystemManifest(buffer, location);
}

auto CartPal::vsSystemManifest(vector<uint8_t>& buffer, string location) -> string {
  bool ines = true;
  if(buffer[0] != 'N'
  || buffer[1] != 'E'
  || buffer[2] != 'S'
  || buffer[3] !=  26) ines = false;

  if(settings["icarus/UseDatabase"].boolean()) {
    auto digest = Hash::SHA256(&buffer[ines ? 16 : 0], buffer.size() - (ines ? 16 : 0)).digest();
    for(auto cartridge : Database::VSSystem.find("vs")) {
      if(cartridge["sha256"].text() == digest) return string{cartridge.text(), "\n  sha256:   ", digest, "\n"};
    }
  }

  if(settings["icarus/UseHeuristics"].boolean()) {
    Heuristics::VSSystem game{buffer, location};
    if(auto manifest = game.manifest()) return manifest;
  }

  return {};
}

auto CartPal::vsSystemImport(vector<uint8_t>& buffer, string location) -> string {
  auto name = Location::prefix(location);
  auto source = Location::path(location);
  string target{settings["Library/Location"].text(), "VS. System/", name, ".vs/"};

  auto manifest = vsSystemManifest(buffer, location);
  if(!manifest) return failure("failed to parse ROM image");

  auto document = BML::unserialize(manifest);
  uint prgrom = document["game/memory(name=program.rom)"]["size"].natural();
  uint chrrom = document["game/memory(name=character.rom)"]["size"].natural();

  if(!create(target)) return failure("library path unwritable");
  if(exists({source, name, ".sav"}) && !exists({target, "save.ram"})) {
    copy({source, name, ".sav"}, {target, "save.ram"});
  }

  if(settings["icarus/CreateManifests"].boolean()) write({target, "manifest.bml"}, manifest);
  write({target, "ines.rom"}, &buffer[0], 16);
  write({target, "program.rom"}, &buffer[16], prgrom);
  if(!chrrom) return success(target);
  write({target, "character.rom"}, &buffer[16 + prgrom], chrrom);
  return success(target);
}
