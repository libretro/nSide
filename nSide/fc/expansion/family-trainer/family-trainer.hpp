struct FamilyTrainer : Expansion {
  FamilyTrainer();

  auto data1() -> bool;
  auto data2() -> uint5;
  auto write(uint3 data) -> void;

private:
  uint3 mask;
};
