struct Controls {
  Controls();

  auto reset() -> void;
  auto run() -> void;
  auto poll() -> void;

  bool up;
  bool down;
  bool left;
  bool right;
  bool b;
  bool a;
  bool select;
  bool start;
  bool previousUp;
  bool previousDown;
  bool previousLeft;
  bool previousRight;
  bool previousB;
  bool previousA;
  bool previousSelect;
  bool previousStart;

  function<auto () -> void> onPressUp;
  function<auto () -> void> onPressDown;
  function<auto () -> void> onPressLeft;
  function<auto () -> void> onPressRight;
  function<auto () -> void> onPressB;
  function<auto () -> void> onPressA;
  function<auto () -> void> onPressSelect;
  function<auto () -> void> onPressStart;
};

extern unique_pointer<Controls> controls;
