auto Icarus::famicomManifest(string) -> string { return ""; }
auto Icarus::famicomManifest(vector<uint8_t>&, string) -> string { return ""; }
auto Icarus::famicomImport(vector<uint8_t>&, string) -> string { return ""; }

auto CartPal::famicomManifest(string location) -> string {
  vector<uint8_t> buffer;
  concatenate(buffer, {location, "ines.rom"});
  concatenate(buffer, {location, "program.rom"});
  concatenate(buffer, {location, "character.rom"});
  return famicomManifest(buffer, location);
}

auto CartPal::famicomManifest(vector<uint8_t>& buffer, string location) -> string {
  bool ines = true;
  if(buffer[0] != 'N'
  || buffer[1] != 'E'
  || buffer[2] != 'S'
  || buffer[3] !=  26) ines = false;

  auto digest = Hash::SHA256(&buffer[ines ? 16 : 0], buffer.size() - (ines ? 16 : 0)).digest();

  if(settings["icarus/UseDatabase"].boolean()) {
    for(auto game : Database::Famicom.find("game")) {
      if(game["sha256"].text() == digest) return BML::serialize(game);
    }

    //v096-v106 legacy format
    for(auto cartridge : Database::Famicom.find("cartridge")) {
      if(cartridge["sha256"].text() == digest) return string{cartridge.text(), "\n  sha256:   ", digest, "\n"};
    }
  }

  if(settings["icarus/UseHeuristics"].boolean()) {
    Heuristics::Famicom game{buffer, location};
    if(auto manifest = game.manifest()) return manifest;
  }

  return {};
}

auto CartPal::famicomImport(vector<uint8_t>& buffer, string location) -> string {
  bool ines = true;
  if(buffer[0] != 'N'
  || buffer[1] != 'E'
  || buffer[2] != 'S'
  || buffer[3] !=  26) ines = false;

  if(ines) {
    if(buffer.data()[7] & 0x01) return vsSystemImport(buffer, location);
    if((buffer.data()[7] & 0x0c) == 0x08) {  //NES 2.0
      if(buffer.data()[7] & 0x02) return playchoice10Import(buffer, location);
    }
  }

  auto name = Location::prefix(location);
  auto source = Location::path(location);
  string target{settings["Library/Location"].text(), "Famicom/", name, ".fc/"};

  auto manifest = famicomManifest(buffer, location);
  if(!manifest) return failure("failed to parse ROM image");

  if(!create(target)) return failure("library path unwritable");
  if(exists({source, name, ".sav"}) && !exists({target, "save.ram"})) {
    copy({source, name, ".sav"}, {target, "save.ram"});
  }

  if(settings["icarus/CreateManifests"].boolean()) write({target, "manifest.bml"}, manifest);
  auto document = BML::unserialize(manifest);
  uint offset = 0;
  if((buffer.size() & 0x1fff) == 0x10) {
    uint size = 16;
    write({target, "ines.rom"}, &buffer[offset], size);
    offset += size;
  }
  if(auto program = document["game/board/memory(type=ROM,content=Program)"]) {
    uint size = program["size"].natural();
    write({target, "program.rom"}, &buffer[offset], size);
    offset += size;
  }
  if(auto character = document["game/board/memory(type=ROM,content=Character)"]) {
    uint size = character["size"].natural();
    write({target, "character.rom"}, &buffer[offset], size);
    offset += size;
  }
  return success(target);
}
