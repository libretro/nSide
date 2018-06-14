#pragma once

namespace ramus {

using namespace nall;

namespace Hash {

struct SHA1 : nall::Hash::Hash {
  nallHash(SHA1)

  auto reset() -> void override {
    for(auto& n : queue) n = 0;
    for(auto& n : w) n = 0;
    for(auto  n : range(5)) h[n] = square(n);
    queued = length = 0;
  }

  auto input(uint8_t value) -> void override {
    byte(value);
    length++;
  }

  auto output() const -> vector<uint8_t> override {
    SHA1 self(*this);
    self.finish();
    vector<uint8_t> result;
    for(auto h : self.h) {
      for(auto n : rrange(4)) result.append(h >> n * 8);
    }
    return result;
  }

  auto value() const -> uint256_t {
    uint256_t value = 0;
    for(auto byte : output()) value = value << 8 | byte;
    return value;
  }

private:
  auto byte(uint8_t value) -> void {
    uint32_t shift = (3 - (queued & 3)) * 8;
    queue[queued >> 2] &= ~(0xff << shift);
    queue[queued >> 2] |= (value << shift);
    if(++queued == 64) block(), queued = 0;
  }

  auto block() -> void {
    for(auto n : range(16)) w[n] = queue[n];
    for(auto n : range(16, 80)) {
      w[n] = rol(w[n - 3] ^ w[n - 8] ^ w[n - 14] ^ w[n - 16], 1);
    }
    uint32_t t[5];
    for(auto n : range(5)) t[n] = h[n];
    for(auto n : range(80)) {
      uint32_t f;
      uint32_t k;
           if(n < 20) {f = (t[1] & t[2]) | (~t[1] & t[3]);                k = 0x5a827999;}
      else if(n < 40) {f = t[1] ^ t[2] ^ t[3];                            k = 0x6ed9eba1;}
      else if(n < 60) {f = (t[1] & t[2]) | (t[1] & t[3]) | (t[2] & t[3]); k = 0x8f1bbcdc;}
      else if(n < 80) {f = t[1] ^ t[2] ^ t[3];                            k = 0xca62c1d6;}
      uint32_t temp = rol(t[0], 5) + f + t[4] + k + w[n];
      t[4] = t[3];
      t[3] = t[2];
      t[2] = rol(t[1], 30);
      t[1] = t[0];
      t[0] = temp;
    }
    for(auto n : range(5)) h[n] += t[n];
  }

  auto finish() -> void {
    byte(0x80);
    while(queued != 56) byte(0x00);
    for(auto n : range(8)) byte(length * 8 >> (7 - n) * 8);
  }

  auto square(uint n) -> uint32_t {
    static const uint32_t value[8] = {
      0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0,
    };
    return value[n];
  }

  uint32_t queue[16] = {0};
  uint32_t w[80] = {0};
  uint32_t h[5] = {0};
  uint32_t queued = 0;
  uint64_t length = 0;
};

}

}
