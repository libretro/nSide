auto PIA::readIO(uint7 addr, uint8 data) -> uint8 {
  switch(addr & 0x07) {

  case 0x00: {  //SWCHA
    data = (io.swcha & io.swacnt) | (data & ~io.swacnt);
    data |= (controllerPort1.device->direction() << 4) & ~io.swacnt;
    data |= (controllerPort2.device->direction() << 0) & ~io.swacnt;
    break;
  }

  case 0x01: {  //SWACNT
    data = io.swacnt;
    break;
  }

  case 0x02: {  //SWCHB
    data = (io.swchb & io.swbcnt) | (data & ~io.swbcnt);
    //Reset switch/button
    if(!io.swbcnt.bit(0)) data.bit(0) = !platform->inputPoll(ID::Port::Hardware, ID::Device::Controls, 1);
    //Select switch/button
    if(!io.swbcnt.bit(1)) data.bit(1) = !platform->inputPoll(ID::Port::Hardware, ID::Device::Controls, 0);
    if(!io.swbcnt.bit(3)) data.bit(3) = io.tvtype;
    if(!io.swbcnt.bit(6)) data.bit(6) = io.difficulty0;
    if(!io.swbcnt.bit(7)) data.bit(7) = io.difficulty1;
    break;
  }

  case 0x03: {  //SWBCNT
    data = io.swbcnt;
    break;
  }

  case 0x04: case 0x06: {  //INTIM
    data = io.timer.base;
    io.timerIRQEnable = addr.bit(3);
    break;
  }

  case 0x05: case 0x07: {  //INSTAT
    data.bits(0,5) = 0;
    data.bit (  6) = io.timerUnderflowINSTAT;
    data.bit (  7) = io.timerUnderflowTIM_T;
    io.timerUnderflowINSTAT = false;
    break;
  }

  }

  return data;
}

auto PIA::writeIO(uint7 addr, uint8 data) -> void {
  switch(addr & 0x17) {

  case 0x00: case 0x10: {
    return;
  }

  case 0x01: case 0x11: {
    io.swacnt = data;
    return;
  }

  case 0x02: case 0x12: {
    return;
  }

  case 0x03: case 0x13: {
    io.swbcnt = data;
    return;
  }

  case 0x04: case 0x05: case 0x06: case 0x07: {
    //PA7 is connected to the left controller port's Right direction.
    //Because the CPU has no IRQ line, this interrupt is useless.
    io.pa7EdgeDetect = addr.bit(0);
    io.pa7IRQEnable  = addr.bit(1);
    return;
  }

  case 0x14: {  //TIM1T
    io.timer.value         = data << 10;
    io.timerDecrement      = 1024;
    io.timerUnderflowTIM_T = false;
    io.timerIRQEnable      = addr.bit(3);
    runTimer();
    return;
  }

  case 0x15: {  //TIM8T
    io.timer.value         = data << 10;
    io.timerDecrement      =  128;
    io.timerUnderflowTIM_T = false;
    io.timerIRQEnable      = addr.bit(3);
    runTimer();
    return;
  }

  case 0x16: {  //TIM64T
    io.timer.value         = data << 10;
    io.timerDecrement      =   16;
    io.timerUnderflowTIM_T = false;
    io.timerIRQEnable      = addr.bit(3);
    runTimer();
    return;
  }

  case 0x17: {  //T1024T
    io.timer.value         = data << 10;
    io.timerDecrement      =    1;
    io.timerUnderflowTIM_T = false;
    io.timerIRQEnable      = addr.bit(3);
    runTimer();
    return;
  }

  }

  unreachable;
}
