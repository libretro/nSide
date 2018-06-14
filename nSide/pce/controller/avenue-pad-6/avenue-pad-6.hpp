struct AvenuePad6 : Controller {
  enum : uint {
    Up, Down, Left, Right, Three, Two, One, Four, Five, Six, Select, Run,
  };

  AvenuePad6();

  auto readData() -> uint4 override;
  auto writeData(uint2 data) -> void override;

  bool sel;
  bool clr;
  bool mode;
};
