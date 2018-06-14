VideoSettings::VideoSettings(TabFrame* parent) : TabFrameItem(parent) {
  setIcon(Icon::Device::Display);

  layout.setMargin(5);

  colorAdjustmentLabel.setFont(Font().setBold());
  saturationValue.setAlignment(0.5);
  saturationSlider.setLength(201).setPosition(settings["Video/Saturation"].natural()).onChange([&] { updateColor(); });
  gammaValue.setAlignment(0.5);
  gammaSlider.setLength(101).setPosition(settings["Video/Gamma"].natural() - 100).onChange([&] { updateColor(); });
  luminanceValue.setAlignment(0.5);
  luminanceSlider.setLength(101).setPosition(settings["Video/Luminance"].natural()).onChange([&] { updateColor(); });

  overscanMaskLabel.setFont(Font().setBold());
  horizontalMaskValue.setAlignment(0.5);
  horizontalMaskSlider.setLength(25).setPosition(settings["Video/Overscan/Horizontal"].natural()).onChange([&] { updateViewport(); });
  verticalMaskValue.setAlignment(0.5);
  verticalMaskSlider.setLength(25).setPosition(settings["Video/Overscan/Vertical"].natural()).onChange([&] { updateViewport(); });

  windowedModeLabel.setFont(Font().setBold());
  windowedModeAspectCorrection.setChecked(settings["Video/Windowed/AspectCorrection"].boolean()).onToggle([&] { updateViewport(); });
  windowedModeIntegralScaling.setChecked(settings["Video/Windowed/IntegralScaling"].boolean()).onToggle([&] { updateViewport(); });
  windowedModeAdaptive.setChecked(settings["Video/Windowed/Adaptive"].boolean()).onToggle([&] { updateViewport(); });

  fullscreenModeLabel.setFont(Font().setBold());
  fullscreenModeAspectCorrection.setChecked(settings["Video/Fullscreen/AspectCorrection"].boolean()).onToggle([&] { updateViewport(); });
  fullscreenModeIntegralScaling.setChecked(settings["Video/Fullscreen/IntegralScaling"].boolean()).onToggle([&] { updateViewport(); });
  fullscreenModeExclusive.setChecked(settings["Video/Fullscreen/Exclusive"].boolean()).onToggle([&] { updateViewport(); });

  updateColor(true);
  updateViewport(true);
}

auto VideoSettings::refreshLocale() -> void {
  setText(locale["Settings/Video"]);

  colorAdjustmentLabel.setText(locale["Settings/Video/ColorAdjustment"]);
  saturationLabel.setText(locale["Settings/Video/ColorAdjustment/Saturation"]);
  gammaLabel.setText(locale["Settings/Video/ColorAdjustment/Gamma"]);
  luminanceLabel.setText(locale["Settings/Video/ColorAdjustment/Luminance"]);

  overscanMaskLabel.setText(locale["Settings/Video/OverscanMask"]);
  horizontalMaskLabel.setText(locale["Settings/Video/OverscanMask/Horizontal"]);
  verticalMaskLabel.setText(locale["Settings/Video/OverscanMask/Vertical"]);

  windowedModeLabel.setText(locale["Settings/Video/WindowedMode"]);
  windowedModeAspectCorrection.setText(locale["Settings/Video/AspectCorrection"]);
  windowedModeIntegralScaling.setText(locale["Settings/Video/IntegralScaling"]);
  windowedModeAdaptive.setText(locale["Settings/Video/AdaptiveSizing"]);

  fullscreenModeLabel.setText(locale["Settings/Video/FullscreenMode"]);
  fullscreenModeAspectCorrection.setText(locale["Settings/Video/AspectCorrection"]);
  fullscreenModeIntegralScaling.setText(locale["Settings/Video/IntegralScaling"]);
  fullscreenModeExclusive.setText(locale["Settings/Video/ExclusiveMode"]);
}

auto VideoSettings::updateColor(bool initializing) -> void {
  settings["Video/Saturation"].setValue(saturationSlider.position());
  settings["Video/Gamma"].setValue(100 + gammaSlider.position());
  settings["Video/Luminance"].setValue(luminanceSlider.position());
  saturationValue.setText({saturationSlider.position(), "%"});
  gammaValue.setText({100 + gammaSlider.position(), "%"});
  luminanceValue.setText({luminanceSlider.position(), "%"});

  if(!initializing) program->updateVideoPalette();
}

auto VideoSettings::updateViewport(bool initializing) -> void {
  bool wasAdaptive = settings["Video/Windowed/Adaptive"].boolean();
  bool isAdaptive = windowedModeAdaptive.checked();

  settings["Video/Overscan/Horizontal"].setValue(horizontalMaskSlider.position());
  settings["Video/Overscan/Vertical"].setValue(verticalMaskSlider.position());
  settings["Video/Windowed/AspectCorrection"].setValue(windowedModeAspectCorrection.checked());
  settings["Video/Windowed/IntegralScaling"].setValue(windowedModeIntegralScaling.checked());
  settings["Video/Windowed/Adaptive"].setValue(windowedModeAdaptive.checked());
  settings["Video/Fullscreen/AspectCorrection"].setValue(fullscreenModeAspectCorrection.checked());
  settings["Video/Fullscreen/IntegralScaling"].setValue(fullscreenModeIntegralScaling.checked());
  settings["Video/Fullscreen/Exclusive"].setValue(fullscreenModeExclusive.checked());
  horizontalMaskValue.setText({horizontalMaskSlider.position()});
  verticalMaskValue.setText({verticalMaskSlider.position()});

  if(!initializing) presentation->resizeViewport(isAdaptive || wasAdaptive != isAdaptive);
}
