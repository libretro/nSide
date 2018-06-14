struct Multitap : Controller {
  enum : uint {
    Up, Down, Left, Right, Two, One, Select, Run,
  };

  Multitap();

  auto readData() -> uint4 override;
  auto writeData(uint2 data) -> void override;

private:
  bool sel;
  bool clr;

  uint id;
};
