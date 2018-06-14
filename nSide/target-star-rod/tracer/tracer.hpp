struct Tracer {
  file fp;
  bool mask;
  uint8 cpuMask[0x200000];
  uint8 smpMask[0x2000];

  auto resetMask() -> void;
  auto maskCPU(uint24 addr) -> bool;
  auto maskSMP(uint16 addr) -> bool;

  auto enabled() -> bool;
  auto enable(bool state) -> void;

  Tracer();

  template<typename... Args> void print(Args&&... args) {
    fp.print(std::forward<Args>(args)...);
  }
};

extern unique_pointer<Tracer> tracer;
