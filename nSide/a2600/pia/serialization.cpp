auto PIA::serialize(serializer& s) -> void {
  Thread::serialize(s);

  s.array(ram);

  s.integer(io.tvtype);
  s.integer(io.difficulty0);
  s.integer(io.difficulty1);

  s.integer(io.swcha);
  s.integer(io.swacnt);
  s.integer(io.swchb);
  s.integer(io.swbcnt);

  s.integer(io.timer.value);
  s.integer(io.timerDecrement);
  s.integer(io.timerUnderflowINSTAT);
  s.integer(io.timerUnderflowTIM_T);

  s.integer(io.timerIRQEnable);
  s.integer(io.pa7IRQEnable);
  s.integer(io.pa7EdgeDetect);
}
