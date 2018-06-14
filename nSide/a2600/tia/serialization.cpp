auto TIA::serialize(serializer& s) -> void {
  Thread::serialize(s);

  s.integer(io.hcounter);
  s.integer(io.vcounter);

  s.integer(io.vsync);
  s.integer(io.vblank);

  s.integer(io.playerMissile0Color);
  s.integer(io.playerMissile1Color);
  s.integer(io.playfieldBallColor);
  s.integer(io.backgroundColor);

  s.integer(io.playfieldBallPriority);

  for(auto& p : player) {
    s.integer(p.numberSize);
    s.integer(p.reflect);
    s.integer(p.graphic);
    s.integer(p.position);
    s.integer(p.motion);
  }

  for(auto& m : missile) {
    s.integer(m.enable);
    s.integer(m.size);
    s.integer(m.position);
    s.integer(m.motion);
  }

  s.integer(ball.enable);
  s.integer(ball.size);
  s.integer(ball.position);
  s.integer(ball.motion);

  s.integer(playfield.reflect);
  s.integer(playfield.score);

  s.integer(playfield.graphic0);
  s.integer(playfield.graphic1);
  s.integer(playfield.graphic2);

  s.integer(collision.m0p1);
  s.integer(collision.m0p0);
  s.integer(collision.m1p0);
  s.integer(collision.m1p1);
  s.integer(collision.p0pf);
  s.integer(collision.p0bl);
  s.integer(collision.p1pf);
  s.integer(collision.p1bl);
  s.integer(collision.m0pf);
  s.integer(collision.m0bl);
  s.integer(collision.m1pf);
  s.integer(collision.m1bl);
  s.integer(collision.blpf);
  s.integer(collision.p0p1);
  s.integer(collision.m0m1);

  for(auto& channel : audio.channel) {
    s.integer(channel.control);
    s.integer(channel.frequency);
    s.integer(channel.volume);
    s.integer(channel.phase);
    s.integer(channel.state1);
    s.integer(channel.state3);
    s.integer(channel.shift4);
    s.integer(channel.shift5);
    s.integer(channel.shift9);
    s.integer(channel.sample);
  }
}
