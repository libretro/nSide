struct Presentation : Window {
  Presentation();
  auto drawSplashScreen() -> void;

  FixedLayout layout{this};
    Viewport viewport{&layout, Geometry{0, 0, 512, 480}};

  StatusBar statusBar{this};

  nall::image splash;
};

extern unique_pointer<Presentation> presentation;
