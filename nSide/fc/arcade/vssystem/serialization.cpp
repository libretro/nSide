auto VSSystem::serialize(serializer& s) -> void {
  s.array(ram);
  s.integer(ramSide);

  s.integer(dipM);
  s.integer(dipS);
  s.integer(watchdog);
}
