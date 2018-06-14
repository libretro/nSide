#define ICARUS_LIBRARY
#include "../../icarus/heuristics/heuristics.hpp"
#include "../../icarus/core/core.hpp"

struct Settings : Markup::Node {
  Settings();
  auto save() -> void;
};

struct LibretroIcarus : Icarus {
	auto create(const string&) -> bool override;
	auto copy(const string&, const string&) -> bool override;
	auto exists(const string&) -> bool override;
	auto directory_exists(const string&) -> bool override;
	auto readable(const string&) -> bool override;
	auto write(const string& filename, const uint8_t* data, uint size) -> bool override;
	auto read(const string& pathname) -> vector<uint8_t> override;

	auto reset() -> void;
	auto import_rom(const string& fake_path, const uint8_t* data, size_t size) -> bool;

	map<string, vector<uint8_t>> imported_files;
};

static vector<LibretroIcarus> icarus;
static Icarus plain_icarus;
static Settings settings;
