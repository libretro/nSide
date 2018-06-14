struct Presentation : Window {
  Presentation();
  auto updateEmulator() -> void;
  auto clearViewport() -> void;
  auto resizeViewport(bool resizeWindow = true) -> void;
  auto toggleFullScreen() -> void;
  auto drawMenu() -> void;

  FixedLayout layout{this};
    Viewport viewport{&layout, Geometry{0, 0, 1, 1}};
};

extern unique_pointer<Presentation> presentation;
