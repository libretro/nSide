auto MOS6502::instructionAbsoluteModify(fp alu) -> void {
  uint16 absolute = operand();
  absolute |= operand() << 8;
  auto data = read(absolute);
  write(absolute, data);
L write(absolute, ALU(data));
}

auto MOS6502::instructionAbsoluteModify(fp alu, uint8 index) -> void {
  uint16 absolute = operand();
  absolute |= operand() << 8;
  idlePageAlways(absolute, absolute + index);
  auto data = read(absolute + index);
  write(absolute + index, data);
L write(absolute + index, ALU(data));
}

auto MOS6502::instructionAbsoluteRead(fp alu, uint8& data) -> void {
  uint16 absolute = operand();
  absolute |= operand() << 8;
L data = ALU(read(absolute));
}

auto MOS6502::instructionAbsoluteRead(fp alu, uint8& data, uint8 index) -> void {
  uint16 absolute = operand();
  absolute |= operand() << 8;
  idlePageCrossed(absolute, absolute + index);
L data = ALU(read(absolute + index));
}

auto MOS6502::instructionAbsoluteWrite(uint8& data) -> void {
  uint16 absolute = operand();
  absolute |= operand() << 8;
L write(absolute, data);
}

auto MOS6502::instructionAbsoluteWrite(uint8& data, uint8 index) -> void {
  uint16 absolute = operand();
  absolute |= operand() << 8;
  idlePageAlways(absolute, absolute + index);
L write(absolute + index, data);
}

auto MOS6502::instructionBranch(bool take) -> void {
  if(!take) {
  L operand();
  } else {
    int8 displacement = operand();
    idlePageCrossed(PC, PC + displacement);
  L idle();
    PC = PC + displacement;
  }
}

auto MOS6502::instructionBreak() -> void {
  operand();
  push(PCH);
  push(PCL);
  uint16 vector = 0xfffe;
  nmi(vector);
  push(P | 0x30);
  I = 1;
  PCL = read(vector++);
L PCH = read(vector++);
}

auto MOS6502::instructionCallAbsolute() -> void {
  uint16 absolute = operand();
  absolute |= operand() << 8;
  idle();
  PC--;
  push(PCH);
L push(PCL);
  PC = absolute;
}

auto MOS6502::instructionClear(bool& flag) -> void {
L idle();
  flag = 0;
}

auto MOS6502::instructionImmediate(fp alu, uint8& data) -> void {
L data = ALU(operand());
}

auto MOS6502::instructionImplied(fp alu, uint8& data) -> void {
L idle();
  data = ALU(data);
}

auto MOS6502::instructionIndirectXRead(fp alu, uint8& data) -> void {
  auto zeroPage = operand();
  load(zeroPage);
  uint16 absolute = load(zeroPage + X + 0);
  absolute |= load(zeroPage + X + 1) << 8;
L data = ALU(read(absolute));
}

auto MOS6502::instructionIndirectXWrite(uint8& data) -> void {
  auto zeroPage = operand();
  load(zeroPage);
  uint16 absolute = load(zeroPage + X + 0);
  absolute |= load(zeroPage + X + 1) << 8;
L write(absolute, data);
}

auto MOS6502::instructionIndirectYRead(fp alu, uint8& data) -> void {
  auto zeroPage = operand();
  uint16 absolute = load(zeroPage + 0);
  absolute |= load(zeroPage + 1) << 8;
  idlePageCrossed(absolute, absolute + Y);
L data = ALU(read(absolute + Y));
}

auto MOS6502::instructionIndirectYWrite(uint8& data) -> void {
  auto zeroPage = operand();
  uint16 absolute = load(zeroPage + 0);
  absolute |= load(zeroPage + 1) << 8;
  idlePageAlways(absolute, absolute + Y);
L write(absolute + Y, data);
}

auto MOS6502::instructionJumpAbsolute() -> void {
  uint16 absolute = operand();
L absolute |= operand() << 8;
  PC = absolute;
}

auto MOS6502::instructionJumpIndirect() -> void {
  uint16 absolute = operand();
  absolute |= operand() << 8;
  uint16 pc = read(absolute);
  absolute.byte(0)++;  //MOS6502: $00ff wraps here to $0000; not $0100
L pc |= read(absolute) << 8;
  PC = pc;
}

auto MOS6502::instructionNoOperation() -> void {
L idle();
}

auto MOS6502::instructionPull(uint8& data) -> void {
  idle();
  idle();
L data = pull();
  Z = data == 0;
  N = data.bit(7);
}

auto MOS6502::instructionPullP() -> void {
  idle();
  idle();
L P = pull();
}

auto MOS6502::instructionPush(uint8& data) -> void {
  idle();
L push(data);
}

auto MOS6502::instructionPushP() -> void {
  idle();
L push(P | 0x30);
}

auto MOS6502::instructionReturnInterrupt() -> void {
  idle();
  idle();
  P = pull();
  PCL = pull();
L PCH = pull();
}

auto MOS6502::instructionReturnSubroutine() -> void {
  idle();
  idle();
  PCL = pull();
  PCH = pull();
L idle();
  PC++;
}

auto MOS6502::instructionSet(bool& flag) -> void {
L idle();
  flag = 1;
}

auto MOS6502::instructionTransfer(uint8& source, uint8& target, bool flag) -> void {
L idle();
  target = source;
  if(!flag) return;
  Z = target == 0;
  N = target.bit(7);
}

auto MOS6502::instructionZeroPageModify(fp alu) -> void {
  auto zeroPage = operand();
  auto data = load(zeroPage);
  store(zeroPage, data);
L store(zeroPage, ALU(data));
}

auto MOS6502::instructionZeroPageModify(fp alu, uint8 index) -> void {
  auto zeroPage = operand();
  load(zeroPage);
  auto data = load(zeroPage + index);
  store(zeroPage + index, data);
L store(zeroPage + index, ALU(data));
}

auto MOS6502::instructionZeroPageRead(fp alu, uint8& data) -> void {
  auto zeroPage = operand();
L data = ALU(load(zeroPage));
}

auto MOS6502::instructionZeroPageRead(fp alu, uint8& data, uint8 index) -> void {
  auto zeroPage = operand();
  load(zeroPage);
L data = ALU(load(zeroPage + index));
}

auto MOS6502::instructionZeroPageWrite(uint8& data) -> void {
  auto zeroPage = operand();
L store(zeroPage, data);
}

auto MOS6502::instructionZeroPageWrite(uint8& data, uint8 index) -> void {
  auto zeroPage = operand();
  read(zeroPage);
L store(zeroPage + index, data);
}

//

auto MOS6502::instructionIndirectXModify(fp alu) -> void {
  auto zeroPage = operand();
  load(zeroPage);
  uint16 absolute = load(zeroPage + X + 0);
  absolute |= load(zeroPage + X + 1) << 8;
  auto data = read(absolute);
  write(absolute, data);
L write(absolute, ALU(data));
}

auto MOS6502::instructionIndirectYModify(fp alu) -> void {
  auto zeroPage = operand();
  uint16 absolute = load(zeroPage + 0);
  absolute |= load(zeroPage + 1) << 8;
  idlePageAlways(absolute, absolute + Y);
  auto data = read(absolute + Y);
  write(absolute + Y, data);
L write(absolute + Y, ALU(data));
}

auto MOS6502::instructionLASAbsolute(uint8 index) -> void {
  uint16 absolute = operand();
  absolute |= operand() << 8;
  idlePageCrossed(absolute, absolute + index);
L A = X = S = read(absolute + index) & S;
}

auto MOS6502::instructionLAXAbsolute() -> void {
  uint16 absolute = operand();
  absolute |= operand() << 8;
L A = X = algorithmLD(read(absolute));
}

auto MOS6502::instructionLAXAbsolute(uint8 index) -> void {
  uint16 absolute = operand();
  absolute |= operand() << 8;
  idlePageCrossed(absolute, absolute + index);
L A = X = algorithmLD(read(absolute + index));
}

auto MOS6502::instructionLAXIndirectX() -> void {
  auto zeroPage = operand();
  load(zeroPage);
  uint16 absolute = load(zeroPage + X + 0);
  absolute |= load(zeroPage + X + 1) << 8;
L A = X = algorithmLD(read(absolute));
}

auto MOS6502::instructionLAXIndirectY() -> void {
  auto zeroPage = operand();
  uint16 absolute = load(zeroPage + 0);
  absolute |= load(zeroPage + 1) << 8;
  idlePageCrossed(absolute, absolute + Y);
L A = X = algorithmLD(read(absolute + Y));
}

auto MOS6502::instructionLAXZeroPage() -> void {
  auto zeroPage = operand();
L A = X = algorithmLD(load(zeroPage));
}

auto MOS6502::instructionLAXZeroPage(uint8 index) -> void {
  auto zeroPage = operand();
  load(zeroPage);
L A = X = algorithmLD(load(zeroPage + index));
}

auto MOS6502::instructionLXAImmediate() -> void {
  //While other LAX addressing modes decode to LDA and LDX, this one decodes
  //to LDA, LDX, and TAX, causing line noise on the data bus to interfere.
  //http://atariage.com/forums/topic/168616-lxa-stable/

  //LXA/ATX/OAL is safe to use if the argument is 0x00 (lxa #$00) or if the
  //accumulator is 0xff prior to execution (lda #$ff...lxa #$??). All other
  //combinations are subject to corruption.
  //http://csdb.dk/release/?id=143981

  //blargg's and hex_usr's NES consoles both run LXA as if it is LAX,
  //(noise is 0xff on both), but that can vary based on temperature and other
  //things.

L A = X = algorithmLD((A | lxaNoise) & operand());
}

auto MOS6502::instructionNOPAbsolute() -> void {
  uint16 absolute = operand();
  absolute |= operand() << 8;
L idle();
}

auto MOS6502::instructionNOPAbsolute(uint8 index) -> void {
  uint16 absolute = operand();
  absolute |= operand() << 8;
  idlePageCrossed(absolute, absolute + index);
L idle();
}

auto MOS6502::instructionNOPImmediate() -> void {
L operand();
}

auto MOS6502::instructionNOPZeroPage() -> void {
  auto zeroPage = operand();
L idle();
}

auto MOS6502::instructionNOPZeroPage(uint8 index) -> void {
  auto zeroPage = operand();
  load(zeroPage);
L idle();
}

auto MOS6502::instructionSAXAbsolute() -> void {
  uint16 absolute = operand();
  absolute |= operand() << 8;
L write(absolute, A & X);
}

auto MOS6502::instructionSAXIndirectX() -> void {
  auto zeroPage = operand();
  load(zeroPage);
  uint16 absolute = load(zeroPage + X + 0);
  absolute |= load(zeroPage + X + 1) << 8;
L write(absolute, A & X);
}

auto MOS6502::instructionSAXZeroPage() -> void {
  auto zeroPage = operand();
L store(zeroPage, A & X);
}

auto MOS6502::instructionSAXZeroPage(uint8 index) -> void {
  auto zeroPage = operand();
  read(zeroPage);
L store(zeroPage + index, A & X);
}

auto MOS6502::instructionAHXAbsolute(uint8 index) -> void {
  //Takes one fewer cycle than other AbsoluteModify instructions
  uint16 absolute = operand();
  absolute |= operand() << 8;
  idlePageAlways(absolute, absolute + index);
L write(absolute + index, A & X & (absolute.byte(1) + 1));
}

auto MOS6502::instructionAHXIndirectY() -> void {
  auto zeroPage = operand();
  uint16 absolute = load(zeroPage + 0);
  absolute |= load(zeroPage + 1) << 8;
  idlePageAlways(absolute, absolute + Y);
L write(absolute + Y, A & X & (absolute.byte(1) + 1));
}

auto MOS6502::instructionSHXAbsolute(uint8 index) -> void {
  uint16 absolute = operand();
  absolute |= operand() << 8;
  idlePageAlways(absolute, absolute + index);
  if(absolute.byte(1) != (absolute + index) >> 8) absolute.byte(1) &= r.x;
L write(absolute + index, X & (absolute.byte(1) + 1));
}

auto MOS6502::instructionSHYAbsolute(uint8 index) -> void {
  uint16 absolute = operand();
  absolute |= operand() << 8;
  idlePageAlways(absolute, absolute + index);
  if(absolute.byte(1) != (absolute + index) >> 8) absolute.byte(1) &= r.y;
L write(absolute + index, Y & (absolute.byte(1) + 1));
}

auto MOS6502::instructionStop() -> void {
  operand();
  PC = 0xffff;
  r.mdr = 0xff;
  while(true) idle();
}

auto MOS6502::instructionTASAbsolute(uint8 index) -> void {
  uint16 absolute = operand();
  absolute |= operand() << 8;
  idlePageAlways(absolute, absolute + index);
  r.s = A & X;
L write(absolute + index, A & X & (absolute.byte(1) + 1));
}
