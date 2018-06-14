struct DipSwitch : HorizontalLayout {
  DipSwitch(VerticalLayout*, Size);

  Label name{this, Size{100, 0}, 5};
  ComboButton value{this, Size{~0, 0}};

  vector<uint> values;
};

struct DipSwitches : Window {
  enum : uint { Slots = 8 };

  VerticalLayout layout{this};
    DipSwitch dip[Slots] = {
      DipSwitch{&layout, Size{~0, 0}},
      DipSwitch{&layout, Size{~0, 0}},
      DipSwitch{&layout, Size{~0, 0}},
      DipSwitch{&layout, Size{~0, 0}},
      DipSwitch{&layout, Size{~0, 0}},
      DipSwitch{&layout, Size{~0, 0}},
      DipSwitch{&layout, Size{~0, 0}},
      DipSwitch{&layout, Size{~0, 0}}
    };
    HorizontalLayout controlLayout{&layout, Size{~0, 0}};
      Widget spacer{&controlLayout, Size{~0, 0}};
      Button accept{&controlLayout, Size{80, 0}};

  auto run(const Markup::Node& node) -> uint;
  DipSwitches();
};

extern DipSwitches* dipSwitches;
