#if defined(Hiro_ComboEdit)

namespace hiro {

auto pComboEdit::construct() -> void {
  hwnd = CreateWindow(
    L"COMBOBOX", L"",
    WS_CHILD | WS_TABSTOP | CBS_DROPDOWN | CBS_HASSTRINGS,
    0, 0, 0, 0,
    _parentHandle(), nullptr, GetModuleHandle(0), 0
  );
  SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)&reference);
  pWidget::_setState();
  setBackgroundColor(state().backgroundColor);
  setEditable(state().editable);
  setText(state().text);
  for(auto& item : state().items) append(item);
}

auto pComboEdit::destruct() -> void {
  if(backgroundBrush) { DeleteObject(backgroundBrush); backgroundBrush = 0; }
  DestroyWindow(hwnd);
}

auto pComboEdit::append(sComboEditItem item) -> void {
  lock();
  SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)(wchar_t*)utf16_t(item->state.text));
  unlock();
}

auto pComboEdit::minimumSize() const -> Size {
  signed width = 0;
  for(auto& item : state().items) {
    width = max(width, pFont::size(hfont, item->state.text).width());
  }
  return {width + 24, pFont::size(hfont, " ").height() + 10};
}

auto pComboEdit::remove(sComboEditItem item) -> void {
  lock();
  SendMessage(hwnd, CB_DELETESTRING, item->offset(), 0);
  unlock();
}

auto pComboEdit::reset() -> void {
  SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
}

auto pComboEdit::setBackgroundColor(Color color) -> void {
  if(backgroundBrush) { DeleteObject(backgroundBrush); backgroundBrush = 0; }
  backgroundBrush = CreateSolidBrush(color ? CreateRGB(color) : GetSysColor(COLOR_WINDOW));
}

auto pComboEdit::setEditable(bool editable) -> void {
//InvalidateRect(hwnd, 0, false);
}

auto pComboEdit::setForegroundColor(Color color) -> void {
}

auto pComboEdit::setGeometry(Geometry geometry) -> void {
  //height = minimum drop-down list height; use CB_SETITEMHEIGHT to control actual widget height
  pWidget::setGeometry({geometry.x(), geometry.y(), geometry.width(), 1});
  RECT rc;
  GetWindowRect(hwnd, &rc);
  unsigned adjustedHeight = geometry.height() - ((rc.bottom - rc.top) - SendMessage(hwnd, CB_GETITEMHEIGHT, (WPARAM)-1, 0));
  SendMessage(hwnd, CB_SETITEMHEIGHT, (WPARAM)-1, adjustedHeight);
}

auto pComboEdit::setText(const string& text) -> void {
  lock();
  SetWindowText(hwnd, utf16_t(text));
  unlock();
}

auto pComboEdit::onChange() -> void {
  signed offset = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
  if(offset != CB_ERR) {
    if(auto item = self().item(offset)) state().text = item->text();
    self().doChange();
  } else {
    state().text = _text();
    if(!locked()) self().doChange();
  }
}

auto pComboEdit::_text() -> string {
  unsigned length = GetWindowTextLength(hwnd);
  wchar_t text[length + 1];
  GetWindowText(hwnd, text, length + 1);
  text[length] = 0;
  return (const char*)utf8_t(text);
}

}

#endif
