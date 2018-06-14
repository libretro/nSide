auto FamicomBox::serialize(serializer& s) -> void {
  s.array(bios_ram);
  s.array(test_ram);

  s.integer(dip);
  s.integer(keyswitch);

  s.integer(exceptionEnable);
  s.integer(exceptionTrap);

  s.integer(ledSelect);
  s.integer(ramProtect);
  s.integer(ledFlash);

  s.integer(zapperGND);
  s.integer(warmboot);
  s.integer(enableControllers);
  s.integer(swapControllers);

  s.integer(attractionTimer);
  s.integer(watchdog);

  s.integer(cartridgeSelect);
  s.integer(cartridgeRowSelect);
  s.integer(registerLock);

  s.integer(coinModule.timer);
  s.integer(coinModule.min10);
  s.integer(coinModule.min20);
}
