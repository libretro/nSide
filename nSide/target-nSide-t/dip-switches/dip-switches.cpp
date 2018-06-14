#include "../nSide.hpp"
DipSwitches* dipSwitches = nullptr;

DipSwitch::DipSwitch(VerticalLayout* parent, Size size) : HorizontalLayout(parent, size) {
}

DipSwitches::DipSwitches() {
  dipSwitches = this;
  setTitle("DIP Switches");

  layout.setMargin(5);

  setSize({250, layout.minimumSize().height()});

  accept.setText("Accept");

  onClose([&] {
    setModal(false);
    setVisible(false);
  });

  accept.onActivate([&] {
    setModal(false);
    setVisible(false);
  });
}

auto DipSwitches::run(const Markup::Node& node) -> uint {
  for(DipSwitch& dipItem : dip) {
    dipItem.name.setEnabled(false);
    dipItem.name.setText("(empty)");
    dipItem.value.setEnabled(false);
    dipItem.value.reset();
    dipItem.values.reset();
  }

  uint index = 0;
  for(auto& setting : node.find("setting")) {
    dip[index].name.setEnabled();
    dip[index].name.setText(setting["name"].text());
    dip[index].value.setEnabled();
    for(auto& option : setting.find("option")) {
      if(auto result = Eval::integer(option["value"].text())) {
        ComboButtonItem item;
        item.setText(option["name"].text());
        dip[index].value.append(item);
        dip[index].values.append(result());
      }
    }

    if(++index >= Slots) break;
  }

  setVisible();
  accept.setFocused();

  audio->clear();
  setModal();

  uint result = 0;
  for(auto& dipItem : dip) {
    if(dipItem.value.enabled() == false) continue;
    result |= dipItem.values[dipItem.value.selected().offset()];
  }
  return result;
}
