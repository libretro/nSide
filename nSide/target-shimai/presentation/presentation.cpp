#include "../shimai.hpp"
unique_pointer<Presentation> presentation;

Presentation::Presentation() {
  presentation = this;

  viewport.setDroppable().onDrop([&](auto locations) {
    if(!directory::exists(locations(0))) return;
    program->mediumQueue.append(locations(0));
    program->loadMedium();
  });

  onClose([&] {
    program->quit();
  });

  setTitle("shimai");
  setResizable(false);
  setBackgroundColor({0, 0, 0});
  resizeViewport();
  setCentered();

  #if defined(PLATFORM_WINDOWS)
  Application::Windows::onModalChange([](bool modal) { if(modal && audio) audio->clear(); });
  #endif

  #if defined(PLATFORM_MACOS)
  Application::Cocoa::onActivate([&] { setFocused(); });
  Application::Cocoa::onQuit([&] { doClose(); });
  #endif
}

auto Presentation::updateEmulator() -> void {
  if(!emulator) return;

  emulator->set("Blur Emulation", settings["Video/BlurEmulation"].boolean());
  emulator->set("Color Emulation", settings["Video/ColorEmulation"].boolean());
  emulator->set("Scanline Emulation", settings["Video/ScanlineEmulation"].boolean());
}

auto Presentation::clearViewport() -> void {
  if(!video) return;

  uint32_t* output;
  uint length = 0;
  uint width = viewport.geometry().width();
  uint height = viewport.geometry().height();
  if(video->lock(output, length, width, height)) {
    for(uint y : range(height)) {
      auto dp = output + y * (length >> 2);
      for(uint x : range(width)) *dp++ = 0xff000000;
    }

    video->unlock();
    video->output();
  }
}

auto Presentation::resizeViewport(bool resizeWindow) -> void {
  //clear video area before resizing to avoid seeing distorted video momentarily
  clearViewport();

  uint viewportWidth = geometry().width();
  uint viewportHeight = geometry().height();

  double emulatorWidth = 320;
  double emulatorHeight = 240;
  double aspectCorrection = 1.0;
  if(emulator) {
    auto information = emulator->videoInformation();
    emulatorWidth = information.width;
    emulatorHeight = information.height;
    aspectCorrection = information.aspectCorrection;
    if(emulator->information.overscan) {
      uint overscanHorizontal = settings["Video/Overscan/Horizontal"].natural();
      uint overscanVertical = settings["Video/Overscan/Vertical"].natural();
      emulatorWidth -= overscanHorizontal * 2;
      emulatorHeight -= overscanVertical * 2;
    }
  }
  if(!fullScreen()) {
    if(settings["Video/Windowed/AspectCorrection"].boolean()) emulatorWidth *= aspectCorrection;

    if(resizeWindow) {
      string viewportScale = "854x480";
      if(settings["Video/Windowed/Scale"].text() == "Small") viewportScale = "854x480";
      if(settings["Video/Windowed/Scale"].text() == "Medium") viewportScale = "1280x720";
      if(settings["Video/Windowed/Scale"].text() == "Large") viewportScale = "1707x960";
      auto resolution = viewportScale.isplit("x", 1L);
      viewportWidth = resolution(0).natural();
      viewportHeight = resolution(1).natural();
    }

    if(!emulator) {
      setSize({emulatorWidth = viewportWidth, emulatorHeight = viewportHeight});
    } else if(settings["Video/Windowed/AdaptiveSizing"].boolean() && resizeWindow) {
      uint multiplier = min(viewportWidth / emulatorWidth, viewportHeight / emulatorHeight);
      emulatorWidth *= multiplier;
      emulatorHeight *= multiplier;
      setSize({viewportWidth = emulatorWidth, viewportHeight = emulatorHeight});
    } else if(settings["Video/Windowed/IntegralScaling"].boolean()) {
      uint multiplier = min(viewportWidth / emulatorWidth, viewportHeight / emulatorHeight);
      emulatorWidth *= multiplier;
      emulatorHeight *= multiplier;
      if(resizeWindow) setSize({viewportWidth, viewportHeight});
    } else {
      double multiplier = min(viewportWidth / emulatorWidth, viewportHeight / emulatorHeight);
      emulatorWidth *= multiplier;
      emulatorHeight *= multiplier;
      if(resizeWindow) setSize({viewportWidth, viewportHeight});
    }
  } else {
    if(settings["Video/Fullscreen/AspectCorrection"].boolean()) emulatorWidth *= aspectCorrection;

    if(!emulator) {
      emulatorWidth = viewportWidth;
      emulatorHeight = viewportHeight;
    } else if(settings["Video/Fullscreen/IntegralScaling"].boolean()) {
      uint multiplier = min(viewportWidth / emulatorWidth, viewportHeight / emulatorHeight);
      emulatorWidth *= multiplier;
      emulatorHeight *= multiplier;
    } else {
      double multiplier = min(viewportWidth / emulatorWidth, viewportHeight / emulatorHeight);
      emulatorWidth *= multiplier;
      emulatorHeight *= multiplier;
    }
  }

  viewport.setGeometry({
    (viewportWidth - emulatorWidth) / 2, (viewportHeight - emulatorHeight) / 2,
    emulatorWidth, emulatorHeight
  });

  //clear video area again to ensure entire viewport area has been painted in
  clearViewport();
}

auto Presentation::toggleFullScreen() -> void {
  if(!fullScreen()) {
    setFullScreen(true);
    video->setExclusive(settings["Video/Fullscreen/Exclusive"].boolean());
    if(video->exclusive()) setVisible(false);
    if(!input->acquired()) input->acquire();
  } else {
    if(input->acquired()) input->release();
    if(video->exclusive()) setVisible(true);
    video->setExclusive(false);
    setFullScreen(false);
  }
  resizeViewport();
}
