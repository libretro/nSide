auto VSSystem::resetButtons() -> void {
  controlLatchedM = 0;
  controlCounterM1 = 0;
  controlCounterM2 = 0;

  controlLatchedS = 0;
  controlCounterS1 = 0;
  controlCounterS2 = 0;

  for(bool& button : buttonsM) button = false;
  for(bool& button : buttonsS) button = false;
}

auto VSSystem::poll(bool side, uint input) -> int16 {
  return platform->inputPoll(ID::Port::Hardware, ID::Device::VSSystemControls, input);
}

auto VSSystem::data1(bool side) -> bool {
  uint counter = side ? controlCounterS1++ : controlCounterM1++;
  bool* buttons = side ? buttonsS : buttonsM;
  bool data;
  data = (side
  ? (swapControllersS ? controllerPortS2 : controllerPortS1)
  : (swapControllersM ? controllerPortM2 : controllerPortM1)
  ).device->data().bit(0);
  if(counter == 2) return buttons[Button1];
  if(counter == 3) return buttons[Button3];
  return data;
}

auto VSSystem::data2(bool side) -> bool {
  uint counter = side ? controlCounterS2++ : controlCounterM2++;
  bool* buttons = side ? buttonsS : buttonsM;
  bool data;
  data = (side
  ? (swapControllersS ? controllerPortS1 : controllerPortS2)
  : (swapControllersM ? controllerPortM1 : controllerPortM2)
  ).device->data().bit(0);
  if(counter == 2) return buttons[Button2];
  if(counter == 3) return buttons[Button4];
  return data;
}

auto VSSystem::latch(bool side, bool data) -> void {
  if(side == 0) {
    if(controlLatchedM == data) return;
    controlLatchedM = data;
    controlCounterM1 = 0;
    controlCounterM2 = 0;

    if(controlLatchedM == 0) {
      for(uint i : {Button1, Button2, Button3, Button4}) buttonsM[i] = poll(0, i);
    }
  } else {
    if(controlLatchedS == data) return;
    controlLatchedS = data;
    controlCounterS1 = 0;
    controlCounterS2 = 0;

    if(controlLatchedS == 0) {
      for(uint i : {Button1, Button2, Button3, Button4}) buttonsS[i] = poll(1, i);
    }
  }
}
