struct Namco163 : Board {
  Namco163(Markup::Node& boardNode) : Board(boardNode), n163(*this, boardNode) {
    chip = &n163;
  }

  auto main() -> void {
    n163.main();
  }

  auto readPRG(uint addr, uint8 data) -> uint8 {
    n163.readPRG(addr, data);
  }

  auto writePRG(uint addr, uint8 data) -> void {
    n163.writePRG(addr, data);
  }

  auto readCHR(uint addr, uint8 data) -> uint8 {
    n163.readCHR(addr, data);
  }

  auto writeCHR(uint addr, uint8 data) -> void {
    n163.writeCHR(addr, data);
  }

  auto serialize(serializer& s) -> void {
    Board::serialize(s);
    n163.serialize(s);
  }

  auto power(bool reset) -> void {
    n163.power(reset);
  }

  N163 n163;
};
