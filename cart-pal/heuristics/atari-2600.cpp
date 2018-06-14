namespace Heuristics {

struct Atari2600 {
  Atari2600(vector<uint8_t>& data, string location);
  explicit operator bool() const;
  auto manifest() const -> string;

private:
  vector<uint8_t>& data;
  string location;
};

Atari2600::Atari2600(vector<uint8_t>& data, string location) : data(data), location(location) {
}

Atari2600::operator bool() const {
  return data.size() >= 0x400;
}

auto Atari2600::manifest() const -> string {
  if(!operator bool()) return {};

  string output;
  output.append("game\n");
  output.append("  sha256: ", Hash::SHA256(data).digest(), "\n");
  output.append("  label:  ", Location::prefix(location), "\n");
  output.append("  name:   ", Location::prefix(location), "\n");
  output.append("  board\n");
  output.append(Memory{}.type("ROM").size(data.size()).content("Program").text());
  return output;
}

}
