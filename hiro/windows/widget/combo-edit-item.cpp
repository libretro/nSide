#if defined(Hiro_ComboEdit)

namespace hiro {

auto pComboEditItem::construct() -> void {
}

auto pComboEditItem::destruct() -> void {
}

auto pComboEditItem::setIcon(const image& icon) -> void {
  //unsupported
}

auto pComboEditItem::setText(const string& text) -> void {
  if(auto parent = _parent()) {
    parent->lock();
    SendMessage(parent->hwnd, CB_DELETESTRING, self().offset(), 0);
    SendMessage(parent->hwnd, CB_INSERTSTRING, self().offset(), (LPARAM)(wchar_t*)utf16_t(state().text));
    parent->unlock();
  }
}

auto pComboEditItem::_parent() -> maybe<pComboEdit&> {
  if(auto parent = self().parentComboEdit()) {
    if(auto self = parent->self()) return *self;
  }
  return nothing;
}

}

#endif
