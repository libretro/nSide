struct PropertiesViewer : Window {
  PropertiesViewer();

  auto updateProperties() -> void;

  enum : uint { PPU, CPU, DMA };

  VerticalLayout layout{this};
    HorizontalLayout controlLayout{&layout, Size{~0, 0}};
      Label sourceLabel{&controlLayout, Size{0, 0}};
      ComboButton sourceSelection{&controlLayout, Size{0, 0}};
      Widget spacer{&controlLayout, Size{~0, 0}};
      CheckLabel autoUpdate{&controlLayout, Size{0, 0}};
      Button update{&controlLayout, Size{0, 0}};
    TextEdit properties{&layout, Size{~0, ~0}};
};

extern unique_pointer<PropertiesViewer> propertiesViewer;
