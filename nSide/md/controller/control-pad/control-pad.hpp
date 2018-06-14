struct ControlPad : Controller {
  enum : uint {
    Up, Down, Left, Right, A, B, C, Start,
  };

  ControlPad(uint port);

  auto readData() -> uint7 override;
  auto writeData(uint7 data) -> void override;

  uint1 select = 1;
};
