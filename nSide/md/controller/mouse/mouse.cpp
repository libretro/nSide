Mouse::Mouse(uint port) : Controller(port) {
  create(Controller::Enter, system.frequency() / 7.0);
}

auto Mouse::main() -> void {
  step(4);
  if(wait) wait--;
  synchronize(cpu);
}

auto Mouse::readData() -> uint7 {
  uint4 data;

  switch(counter) {
  case  0: data = 0b0000; break;  //Signature
  case  1: data = 0b1011; break;  //Signature
  case  2: data = 0b1111; break;  //Signature
  case  3: data = 0b1111; break;  //Signature
  case  4: data = oy << 3 | ox << 2 | dy << 1 | dx << 0; break;
  case  5: data = start << 3 | middle << 2 | right << 1 | left << 0; break;
  case  6: data = x.bits(4,7); break;
  case  7: data = x.bits(0,3); break;
  case  8: data = y.bits(4,7); break;
  case  9:
  default: data = y.bits(0,3); break;
  }

  uint1 tl = tr ^ (wait > 0);

  return th << 6 | tr << 5 | tl << 4 | data;
}

auto Mouse::writeData(uint7 data) -> void {
  if(th != data.bit(6)) {
    counter = 1 - data.bit(6);

    x      =  platform->inputPoll(port, ID::Device::Mouse, X);  //-n = left, 0 = center, +n = right
    y      = -platform->inputPoll(port, ID::Device::Mouse, Y);  //-n = down, 0 = center, +n = up
    left   =  platform->inputPoll(port, ID::Device::Mouse, Left);
    middle =  platform->inputPoll(port, ID::Device::Mouse, Middle);
    right  =  platform->inputPoll(port, ID::Device::Mouse, Right);
    start  =  platform->inputPoll(port, ID::Device::Mouse, Start);

    dx = x < 0;  //0 = right, 1 = left
    dy = y < 0;  //0 = down,  1 = up

    ox = x > 255 || x < -255;
    oy = y > 255 || y < -255;
  }
  if(tr != data.bit(5)) {
    if(counter > 0 && counter < 9) {
      counter++;
      wait = 16;
    }
  }
  tr     = data.bit(5);
  th     = data.bit(6);
}
