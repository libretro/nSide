auto PPUcounter::serialize(serializer& s) -> void {
  s.integer(status.field);
  s.integer(status.hcounter);
  s.integer(status.vcounter);
}

auto PPU::serialize(serializer& s) -> void {
  Thread::serialize(s);
  PPUcounter::serialize(s);

  s.integer(io.chrAddressBus);
  s.integer(io.mdr);
  s.array(io.mdrDecay);

  s.integer(io.busData);

  s.integer(io.v.value);
  s.integer(io.t.value);

  s.integer(io.nmiHold);
  s.integer(io.nmiFlag);

  s.integer(io.vramIncrement);
  s.integer(io.objAddress);
  s.integer(io.bgAddress);
  s.integer(io.spriteHeight);
  s.integer(io.masterSelect);
  s.integer(io.nmiEnable);

  s.integer(io.grayscale);
  s.integer(io.bgEdgeEnable);
  s.integer(io.objEdgeEnable);
  s.integer(io.bgEnable);
  s.integer(io.objEnable);
  s.integer(io.emphasis);

  s.integer(io.spriteOverflow);
  s.integer(io.spriteZeroHit);

  s.integer(io.oamAddress);

  s.integer(l.nametable);
  s.integer(l.attribute);
  s.integer(l.tiledataLo);
  s.integer(l.tiledataHi);

  s.integer(l.oamIterator);
  s.integer(l.oamCounter);

  for(auto& o : l.oam) o.serialize(s);
  for(auto& o : l.soam) o.serialize(s);

  s.array(ciram, !Model::VSSystem() ? 0x0800 : 0x1000);
  s.array(cgram);
  s.array(oam);
}

auto PPU::OAM::serialize(serializer& s) -> void {
  s.integer(id);
  s.integer(y);
  s.integer(tile);
  s.integer(attr);
  s.integer(x);

  s.integer(tiledataLo);
  s.integer(tiledataHi);
}
