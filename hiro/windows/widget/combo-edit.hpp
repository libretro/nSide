#if defined(Hiro_ComboEdit)

namespace hiro {

struct pComboEdit : pWidget {
  Declare(ComboEdit, Widget)

  auto append(sComboEditItem item) -> void;
  auto minimumSize() const -> Size override;
  auto remove(sComboEditItem item) -> void;
  auto reset() -> void;
  auto setBackgroundColor(Color color) -> void;
  auto setEditable(bool editable) -> void;
  auto setForegroundColor(Color color) -> void;
  auto setGeometry(Geometry geometry) -> void override;
  auto setText(const string& text) -> void;

  auto onChange() -> void;

  auto _text() -> string;

  HBRUSH backgroundBrush = nullptr;
};

}

#endif
