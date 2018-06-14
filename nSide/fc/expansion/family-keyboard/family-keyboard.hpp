struct FamilyKeyboard : Expansion {
  enum : uint {
    F1, F2, F3, F4, F5, F6, F7, F8,
    Digit1, Digit2, Digit3, Digit4, Digit5, Digit6, Digit7, Digit8, Digit9, Digit0,
    HyphenMinus, Circumflex, En, STOP,
    ESC,     Q, W, E, R, T, Y, U, I, O, P, AtSign, LeftBracket, RETURN,
    CTR,      A, S, D, F, G, H, J, K, L, Semicolon, Colon, RightBracket, Kana,
    SHIFTLeft, Z, X, C, V, B, N, M, Comma, FullStop, Solidus, KanaN, SHIFTRight,
    GRPH, Space,
    CLR_HOME, INS, DEL,
    Up, Left, Right, Down,
  };

  FamilyKeyboard();

  auto data1() -> bool;
  auto data2() -> uint5;
  auto write(uint3 data) -> void;

private:
  bool column;
  uint row;
};
