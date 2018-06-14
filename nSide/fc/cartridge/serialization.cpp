auto Cartridge::serialize(serializer& s) -> void {
  Thread::serialize(s);
  if(board) return board->serialize(s);
}
