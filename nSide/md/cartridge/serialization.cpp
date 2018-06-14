auto Cartridge::serialize(serializer& s) -> void {
  if(ram.size) s.array(ram.data, ram.size);

  if(lockon) lockon->serialize(s);
}
