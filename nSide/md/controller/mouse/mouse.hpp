struct Mouse : Controller {
  enum : uint {
    X, Y, Left, Middle, Right, Start,
  };

  Mouse(uint port);
  auto main() -> void override;

  auto readData() -> uint7 override;
  auto writeData(uint7 data) -> void override;

  int16 x;         //x-coordinate
  int16 y;         //y-coordinate
  boolean dx;      //x-direction
  boolean dy;      //y-direction
  boolean ox;      //x-overflow
  boolean oy;      //y-overflow
  boolean left;    //left button
  boolean middle;  //middle button
  boolean right;   //right button
  boolean start;   //Start button
  uint wait;       //TR transition latency

  uint1 tr = 1;
  uint1 th = 1;
  uint4 counter;
};
