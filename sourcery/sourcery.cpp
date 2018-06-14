#include <nall/nall.hpp>
using namespace nall;

struct Sourcery {
  auto main(string_vector) -> void;
  auto parse(Markup::Node&) -> void;

private:
  string pathname;
  file source;
  file header;
};

auto Sourcery::main(string_vector args) -> void {
  if(args.size() != 4) return print("usage: sourcery resource.bml resource.cpp resource.hpp\n");

  string markupName = args[1];
  string sourceName = args[2];
  string headerName = args[3];
  if(!markupName.endsWith(".bml")) return print("error: arguments in incorrect order\n");
  if(!sourceName.endsWith(".cpp")) return print("error: arguments in incorrect order\n");
  if(!headerName.endsWith(".hpp")) return print("error: arguments in incorrect order\n");

  string markup = string::read(markupName);
  if(!markup) return print("error: unable to read resource manifest\n");

  pathname = Location::path(markupName);
  if(!source.open(sourceName, file::mode::write)) return print("error: unable to write source file\n");
  if(!header.open(headerName, file::mode::write)) return print("error: unable to write header file\n");

  source.print("#include <nall/nall.hpp>\n");
  source.print("#include \"", headerName, "\"\n\n");

  auto document = BML::unserialize(markup);
  parse(document);
}

auto Sourcery::parse(Markup::Node& root) -> void {
  for(auto node : root) {
    if(node.name() == "namespace") {
      header.print("namespace ", node["name"].text(), " {\n");
      source.print("namespace ", node["name"].text(), " {\n");
      parse(node);
      header.print("}\n");
      source.print("}\n");
    } else if(node.name() == "binary") {
      string filename{pathname, node["file"].text()};
      if(!file::exists(filename)) {
        print("warning: binary file ", node["file"].text(), " not found\n");
        continue;
      }
      auto buffer = file::read(filename);
      header.print("extern const nall::vector<uint8_t> ", node["name"].text(), ";\n");
      source.print("const nall::vector<uint8_t> ", node["name"].text(), " = {  //size: ", buffer.size(), "\n");
      for(auto offset : range(buffer)) {
        if((offset & 31) ==  0) source.print("  ");
        source.print(buffer[offset], ",");
        if((offset & 31) == 31) source.print("\n");
      }
      if(buffer.size() & 31) source.print("\n");
      source.print("};\n");
    }
  }
}

#include <nall/main.hpp>
auto nall::main(string_vector args) -> void {
  Sourcery().main(args);
}
