struct MemoryEditor : Window {
  MemoryEditor();

  auto read(uint addr) -> uint8_t;
  auto write(uint addr, uint8_t data) -> void;
  auto selectSource() -> void;
  auto exportMemoryToDisk() -> void;
  auto updateView() -> void;

  enum : uint { CPU, APU, VRAM, OAM, CGRAM };

  VerticalLayout layout{this};
    HorizontalLayout controlLayout{&layout, Size{~0, 0}};
      Label gotoLabel{&controlLayout, Size{0, 0}};
      LineEdit gotoAddress{&controlLayout, Size{50, 0}};
      ComboButton source{&controlLayout, Size{0, 0}};
      Button exportMemory{&controlLayout, Size{80, 0}};
      Widget spacer{&controlLayout, Size{~0, 0}};
      CheckLabel autoUpdate{&controlLayout, Size{0, 0}};
      Button update{&controlLayout, Size{80, 0}};
    HexEdit editor{&layout, Size{~0, ~0}};
};

extern unique_pointer<MemoryEditor> memoryEditor;
