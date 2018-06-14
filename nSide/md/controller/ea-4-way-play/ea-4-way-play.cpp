uint2 EA4WayPlay::player = 3;
boolean EA4WayPlay::signature = 1;
boolean EA4WayPlay::select = 1;
//boolean EA4WayPlay::localSelect[4] = {1, 1, 1, 1};
//uint3 EA4WayPlay::counter[4] = {0, 0, 0, 0};
//uint EA4WayPlay::timeout[4] = {0, 0, 0, 0};

EA4WayPlay::EA4WayPlay(uint port) : Controller(port) {
  if(port == ID::Port::Controller1) {
  //create(Controller::Enter, 1'000'000.0);
    player = 3;
    signature = 1;
    select = 1;
  //for(boolean& n : localSelect) n = 1;
  //for(uint3& n : counter) n = 0;
  //for(uint& n : timeout) n = 0;
  }
}

/*
auto EA4WayPlay::main() -> void {
  if(port == ID::Port::Controller1) {
    for(uint player : range(4)) {
      if(timeout) timeout[player] -= 1;
      else        counter[player]  = 0;
    }
    step(1);
    synchronize(cpu);
  }
}
*/

auto EA4WayPlay::readData() -> uint7 {
  if(port == ID::Port::Controller1) {
    if(signature) {
      return select << 6 | 0b110000;  //bits 0 and 1 must be low
    }

    uint6 data;

    if(select == 0) {
      data.bit(0) = platform->inputPoll(port, ID::Device::EA4WayPlay, Up    + player * 8);
      data.bit(1) = platform->inputPoll(port, ID::Device::EA4WayPlay, Down  + player * 8);
      data.bits(2,3) = ~0;
      data.bit(4) = platform->inputPoll(port, ID::Device::EA4WayPlay, A     + player * 8);
      data.bit(5) = platform->inputPoll(port, ID::Device::EA4WayPlay, Start + player * 8);
    } else {
      data.bit(0) = platform->inputPoll(port, ID::Device::EA4WayPlay, Up    + player * 8);
      data.bit(1) = platform->inputPoll(port, ID::Device::EA4WayPlay, Down  + player * 8);
      data.bit(2) = platform->inputPoll(port, ID::Device::EA4WayPlay, Left  + player * 8);
      data.bit(3) = platform->inputPoll(port, ID::Device::EA4WayPlay, Right + player * 8);
      data.bit(4) = platform->inputPoll(port, ID::Device::EA4WayPlay, B     + player * 8);
      data.bit(5) = platform->inputPoll(port, ID::Device::EA4WayPlay, C     + player * 8);
    }

    /*
    if(select == 0) {
      if(counter[player] != 2 && counter[player] != 3) {
        data.bit(0) = platform->inputPoll(port, ID::Device::EA4WayPlay, Up   + player * 12);
        data.bit(1) = platform->inputPoll(port, ID::Device::EA4WayPlay, Down + player * 12);
        data.bits(2,3) = ~0;
      } else if(counter[player] == 2) {
        data.bits(0,3) = ~0;  //needed for controller detection
      } else if(counter[player] == 3) {
        data.bits(0,3) =  0;
      }
      data.bit(4) = platform->inputPoll(port, ID::Device::EA4WayPlay, A     + player * 12);
      data.bit(5) = platform->inputPoll(port, ID::Device::EA4WayPlay, Start + player * 12);
    } else {
      if(counter[player] != 3) {
        data.bit(0) = platform->inputPoll(port, ID::Device::EA4WayPlay, Up    + player * 12);
        data.bit(1) = platform->inputPoll(port, ID::Device::EA4WayPlay, Down  + player * 12);
        data.bit(2) = platform->inputPoll(port, ID::Device::EA4WayPlay, Left  + player * 12);
        data.bit(3) = platform->inputPoll(port, ID::Device::EA4WayPlay, Right + player * 12);
      } else {
        data.bit(0) = platform->inputPoll(port, ID::Device::EA4WayPlay, Z     + player * 12);
        data.bit(1) = platform->inputPoll(port, ID::Device::EA4WayPlay, Y     + player * 12);
        data.bit(2) = platform->inputPoll(port, ID::Device::EA4WayPlay, X     + player * 12);
        data.bit(3) = platform->inputPoll(port, ID::Device::EA4WayPlay, Mode  + player * 12);
      }
      data.bit(4) = platform->inputPoll(port, ID::Device::EA4WayPlay, B     + player * 12);
      data.bit(5) = platform->inputPoll(port, ID::Device::EA4WayPlay, C     + player * 12);
    }
    */

    data = ~data;
    return select << 6 | data;
  } else if(port == ID::Port::Controller2) {
    return signature << 6 | player << 4 | 0b1111;
  }
}

auto EA4WayPlay::writeData(uint7 data) -> void {
  if(port == ID::Port::Controller1) {
    select = data.bit(6);
  } else if(port == ID::Port::Controller2) {
    if(data.bits(0,3) == 0x0c) {
      player = data.bits(4,5);
      signature = data.bit(6);
    }
  }
//if(!player.bit(2)) {
//  if(!localSelect[player] && select) counter[player] = (counter[player] + 1) % 5;
//  localSelect[player] = select;
//}
}
