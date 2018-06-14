AvenuePad6::AvenuePad6() {
  //create(Controller::Enter, 60.0);
  mode = 0;
}

//auto AvenuePad6::main() -> void {
//  step(1);
//  mode = !mode;
//  synchronize(cpu);
//}

auto AvenuePad6::readData() -> uint4 {
  if(clr) return 0;

  uint4 data;

  if(sel) {
    if(mode == 0) {
      bool up    = platform->inputPoll(ID::Port::Controller, ID::Device::AvenuePad6, Up);
      bool right = platform->inputPoll(ID::Port::Controller, ID::Device::AvenuePad6, Right);
      bool down  = platform->inputPoll(ID::Port::Controller, ID::Device::AvenuePad6, Down);
      bool left  = platform->inputPoll(ID::Port::Controller, ID::Device::AvenuePad6, Left);
      data.bit(0) = !(up & !down);
      data.bit(1) = !(right & !left);
      data.bit(2) = !(down & !up);
      data.bit(3) = !(left & !right);
    } else {
      data = 0;  //all 4 directions appear pressed
    }
    mode = !mode;
  } else {
    if(mode == 0) {
      bool one    = platform->inputPoll(ID::Port::Controller, ID::Device::AvenuePad6, One);
      bool two    = platform->inputPoll(ID::Port::Controller, ID::Device::AvenuePad6, Two);
      bool select = platform->inputPoll(ID::Port::Controller, ID::Device::AvenuePad6, Select);
      bool run    = platform->inputPoll(ID::Port::Controller, ID::Device::AvenuePad6, Run);
      data.bit(0) = !one;
      data.bit(1) = !two;
      data.bit(2) = !select;
      data.bit(3) = !run;
    } else {
      bool six    = platform->inputPoll(ID::Port::Controller, ID::Device::AvenuePad6, Six);
      bool five   = platform->inputPoll(ID::Port::Controller, ID::Device::AvenuePad6, Five);
      bool four   = platform->inputPoll(ID::Port::Controller, ID::Device::AvenuePad6, Four);
      bool three  = platform->inputPoll(ID::Port::Controller, ID::Device::AvenuePad6, Three);
      data.bit(0) = !six;
      data.bit(1) = !five;
      data.bit(2) = !four;
      data.bit(3) = !three;
    }
  }

  return data;
}

auto AvenuePad6::writeData(uint2 data) -> void {
  if(!sel && data.bit(0)) mode = !mode;  //Mednafen method
  sel = data.bit(0);
  clr = data.bit(1);
}
