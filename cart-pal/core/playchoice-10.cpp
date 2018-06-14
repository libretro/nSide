auto CartPal::playchoice10Manifest(string location) -> string {
  vector<uint8_t> buffer;
  concatenate(buffer, {location, "ines.rom"});
  concatenate(buffer, {location, "program.rom"});
  concatenate(buffer, {location, "character.rom"});
  concatenate(buffer, {location, "instructions.rom"});
  concatenate(buffer, {location, "key.rom"});
  return playchoice10Manifest(buffer, location);
}

auto CartPal::playchoice10Manifest(vector<uint8_t>& buffer, string location) -> string {
  bool ines = true;
  if(buffer[0] != 'N'
  || buffer[1] != 'E'
  || buffer[2] != 'S'
  || buffer[3] !=  26) ines = false;

  if(settings["icarus/UseDatabase"].boolean()) {
    //PlayChoice-10 ROMs in iNES format store 16 bytes for the key,
    //and 16 more bytes for Counter Out.
    //However, only 9 key bytes actually exist in the cartridge.
    //The remaining 7 key bytes, as well as the entire Counter Out,
    //follow a pattern based on hardware behavior.
    //TODO: Detect extraneous bytes and exclude them from SHA256
    auto digest = Hash::SHA256(&buffer[ines ? 16 : 0], buffer.size() - (ines ? 16 : 0)).digest();
    for(auto cartridge : Database::PlayChoice10.find("cartridge")) {
      if(cartridge["sha256"].text() == digest) return string{cartridge.text(), "\n  sha256:   ", digest, "\n"};
    }
  }

  if(settings["icarus/UseHeuristics"].boolean()) {
    Heuristics::Famicom game{buffer, location};
    if(auto manifest = game.manifest()) return manifest;
  }

  return {};
}

auto CartPal::playchoice10Import(vector<uint8_t>& buffer, string location) -> string {
  auto name = Location::prefix(location);
  auto source = Location::path(location);
  string target{settings["Library/Location"].text(), "PlayChoice-10/", name, ".pc10/"};

  auto manifest = playchoice10Manifest(buffer, location);
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
  write({target, "instructions.rom"}, &buffer[16 + prgrom + chrrom], 0x2000);
  write({target, "key.rom"}, &buffer[16 + prgrom + chrrom + 0x2000], 0x9);
  return success(target);
}
