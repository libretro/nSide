struct BGViewer : Window {
  BGViewer();

  auto bgChanged() -> void;
  auto updateTiles() -> void;

  VerticalLayout layout{this};
    HorizontalLayout controlLayout{&layout, Size{~0, 0}};
      Label bgLabel{&controlLayout, Size{0, 0}};
      ComboButton bgSelection{&controlLayout, Size{~0, 0}};
      Widget spacer{&controlLayout, Size{~0, 0}};
      CheckLabel autoUpdate{&controlLayout, Size{0, 0}};
      Button update{&controlLayout, Size{80, 0}};
    HorizontalLayout canvasLayout{&layout, Size{~0, 0}};
      Canvas canvas{&canvasLayout, Size{1024, 512}};
      VerticalScrollBar scroll{&canvasLayout, Size{0, ~0}};

  StatusBar statusBar{this};
};

extern unique_pointer<BGViewer> bgViewer;
