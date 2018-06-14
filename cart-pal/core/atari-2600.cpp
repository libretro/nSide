auto CartPal::atari2600Manifest(string location) -> string {
  vector<uint8_t> buffer;
  concatenate(buffer, {location, "program.rom"});
  return atari2600Manifest(buffer, location);
}

auto CartPal::atari2600Manifest(vector<uint8_t>& buffer, string location) -> string {
  auto digest = Hash::SHA256(buffer).digest();

  if(settings["icarus/UseDatabase"].boolean()) {
    for(auto game : Database::Atari2600.find("game")) {
      if(game["sha256"].text() == digest) return BML::serialize(game);
    }
  }

  if(settings["icarus/UseHeuristics"].boolean()) {
    Heuristics::Atari2600 game{buffer, location};
    if(auto manifest = game.manifest()) return manifest;
  }

  return {};
}

auto CartPal::atari2600Import(vector<uint8_t>& buffer, string location) -> string {
  auto name = Location::prefix(location);
  auto source = Location::path(location);
  string target{settings["Library/Location"].text(), "Atari 2600/", name, ".a26/"};

  auto manifest = atari2600Manifest(buffer, location);
  if(!manifest) return failure("failed to parse ROM image");

  if(!create(target)) return failure("library path unwritable");
  if(exists({source, name, ".sav"}) && !exists({target, "save.ram"})) {
    copy({source, name, ".sav"}, {target, "save.ram"});
  }

  if(settings["icarus/CreateManifests"].boolean()) write({target, "manifest.bml"}, manifest);
  write({target, "program.rom"}, buffer);
  return success(target);
}
