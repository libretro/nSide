FamilyKeyboard::FamilyKeyboard() {
  column = 0;
  row    = 0;
}

auto FamilyKeyboard::data1() -> bool {
  //Data Recorder
  return 0;
}

auto FamilyKeyboard::data2() -> uint5 {
  static const uint matrix0[] = {
    F8,        RETURN,     LeftBracket, RightBracket,
    F7,        AtSign,     Colon,       Semicolon,
    F6,        O,          L,           K,
    F5,        I,          U,           J,
    F4,        Y,          G,           H,
    F3,        T,          R,           D,
    F2,        W,          S,           A,
    F1,        ESC,        Q,           CTR,
    CLR_HOME,  Up,         Right,       Left,
  };
  static const uint matrix1[] = {
    Kana,      SHIFTRight, En,          STOP,
    KanaN,     Solidus,    HyphenMinus, Circumflex,
    FullStop,  Comma,      P,           Digit0,
    M,         N,          Digit9,      Digit8,
    B,         V,          Digit7,      Digit6,
    F,         C,          Digit5,      Digit4,
    X,         Z,          E,           Digit3,
    SHIFTLeft, GRPH,       Digit1,      Digit2,
    Down,      Space,      DEL,         INS,
  };

  if(row == 9) return 0x1e;
  const uint* matrix = column ? matrix1 : matrix0;
  uint keys[] = {
    matrix[(row << 2) | 0],
    matrix[(row << 2) | 1],
    matrix[(row << 2) | 2],
    matrix[(row << 2) | 3],
  };
  return (
    !platform->inputPoll(ID::Port::Expansion, ID::Device::FamilyKeyboard, keys[0]) << 1
  | !platform->inputPoll(ID::Port::Expansion, ID::Device::FamilyKeyboard, keys[1]) << 2
  | !platform->inputPoll(ID::Port::Expansion, ID::Device::FamilyKeyboard, keys[2]) << 3
  | !platform->inputPoll(ID::Port::Expansion, ID::Device::FamilyKeyboard, keys[3]) << 4
  );
}

auto FamilyKeyboard::write(uint3 data) -> void {
  if(data.bit(2)) {
    if(column && !data.bit(1)) row = (row + 1) % 10;
    column = data.bit(1);
    if(data.bit(0)) row = 0;
  }
}
