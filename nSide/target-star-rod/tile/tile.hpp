struct TileViewer : Window {
  TileViewer();

  auto modeChanged() -> void;
  auto updateTiles() -> void;

  VerticalLayout layout{this};
    HorizontalLayout controlLayout{&layout, Size{~0, 0}};
      Label modeLabel{&controlLayout, Size{0, 0}};
      ComboButton modeSelection{&controlLayout, Size{~0, 0}};
      Label paletteLabel{&controlLayout, Size{0, 0}};
      ComboButton paletteSelection{&controlLayout, Size{~0, 0}};
      Widget spacer{&controlLayout, Size{~0, 0}};
      CheckLabel autoUpdate{&controlLayout, Size{0, 0}};
      Button update{&controlLayout, Size{80, 0}};
    HorizontalLayout columnLayout{&layout, Size{~0, 0}};
      RadioLabel columnLinear{&columnLayout, Size{100, 0}};
      RadioLabel column16{&columnLayout, Size{100, 0}};
      Group columnGroup{&columnLinear, &column16};
    HorizontalLayout memoryLayout{&layout, Size{~0, 0}};
      RadioLabel memoryVRAM{&memoryLayout, Size{100, 0}};
      RadioLabel memoryWRAM{&memoryLayout, Size{100, 0}};
      Group memoryGroup{&memoryVRAM, &memoryWRAM};
      LineEdit wramAddress{&memoryLayout, Size{80, 0}};
    HorizontalLayout canvasLayout{&layout, Size{~0, 0}};
      Canvas canvas{&canvasLayout, Size{512, 512}};
      VerticalScrollBar scroll{&canvasLayout, Size{0, ~0}};

  StatusBar statusBar{this};

  uint columnCount;
  uint columnWidth;
  uint columnHeight;
};

extern unique_pointer<TileViewer> tileViewer;
