GameNotes::GameNotes(TabFrame* parent) : TabFrameItem(parent) {
  setIcon(Icon::Emblem::Text);

  layout.setMargin(5);
  notes.setWordWrap(false).setFont(Font().setFamily(Font::Mono));
}

auto GameNotes::refreshLocale() -> void {
  setText(locale["Tools/GameNotes"]);
}

//Use the same notes as higan; use "higan" instead of "nSide"
auto GameNotes::loadNotes() -> void {
  auto contents = string::read({program->mediumPaths(1), "higan/notes.txt"});
  notes.setText(contents);
}

auto GameNotes::saveNotes() -> void {
  auto contents = notes.text();
  if(contents) {
    directory::create({program->mediumPaths(1), "higan/"});
    file::write({program->mediumPaths(1), "higan/notes.txt"}, contents);
  } else {
    file::remove({program->mediumPaths(1), "higan/notes.txt"});
  }
}
