ManifestViewer::ManifestViewer(TabFrame* parent) : TabFrameItem(parent) {
  setIcon(Icon::Emblem::Text);

  layout.setMargin(5);
  manifestView.setEditable(false).setWordWrap(false).setFont(Font().setFamily(Font::Mono));
}

auto ManifestViewer::refreshLocale() -> void {
  setText(locale["Tools/ManifestViewer"]);
}

auto ManifestViewer::doRefresh() -> void {
  manifestView.setText("");
  if(emulator) manifestView.setText(emulator->manifest());
}
