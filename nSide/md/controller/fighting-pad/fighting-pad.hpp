struct FightingPad : Controller {
  enum : uint {
    Up, Down, Left, Right, A, B, C, X, Y, Z, Mode, Start,
  };

  FightingPad(uint port);
  auto main() -> void override;

  auto readData() -> uint7 override;
  auto writeData(uint7 data) -> void override;

  uint1 select = 1;
  uint3 counter;
  uint32 timeout;
};
