unique_pointer<SMPRegisterEditor> smpRegisterEditor;

SMPRegisterEditor::SMPRegisterEditor() {
  smpRegisterEditor = this;
  setTitle("SMP Register Editor");

  layout.setMargin(5);
  regALabel.setText("A:");
  regAValue.setFont(Font().setFamily(Font::Mono));
  regXLabel.setText("X:");
  regXValue.setFont(Font().setFamily(Font::Mono));
  regYLabel.setText("Y:");
  regYValue.setFont(Font().setFamily(Font::Mono));
  regSLabel.setText("S:01");
  regSValue.setFont(Font().setFamily(Font::Mono));
  flagN.setText("N");
  flagV.setText("V");
  flagP.setText("P");
  flagB.setText("B");
  flagH.setText("H");
  flagI.setText("I");
  flagZ.setText("Z");
  flagC.setText("C");
  update.setText("Update").onActivate([&] {
    saveRegisters();
    smpDebugger->updateDisassembly();
    setVisible(false);
  });

  loadRegisters();

  setGeometry({{128, 128}, layout.minimumSize()});
}

auto SMPRegisterEditor::loadRegisters() -> void {
  regAValue.setText(hex(SFC::smp.r.ya.byte.l, 2L));
  regXValue.setText(hex(SFC::smp.r.x, 2L));
  regYValue.setText(hex(SFC::smp.r.ya.byte.h, 2L));
  regSValue.setText(hex(SFC::smp.r.s, 2L));
  flagN.setChecked(SFC::smp.r.p.n);
  flagV.setChecked(SFC::smp.r.p.v);
  flagP.setChecked(SFC::smp.r.p.p);
  flagB.setChecked(SFC::smp.r.p.b);
  flagH.setChecked(SFC::smp.r.p.h);
  flagI.setChecked(SFC::smp.r.p.i);
  flagZ.setChecked(SFC::smp.r.p.z);
  flagC.setChecked(SFC::smp.r.p.c);
}

auto SMPRegisterEditor::saveRegisters() -> void {
  SFC::smp.r.ya.byte.l = regAValue.text().hex();
  SFC::smp.r.x = regXValue.text().hex();
  SFC::smp.r.ya.byte.h = regYValue.text().hex();
  SFC::smp.r.s = regSValue.text().hex();
  SFC::smp.r.p.n = flagN.checked();
  SFC::smp.r.p.v = flagV.checked();
  SFC::smp.r.p.p = flagP.checked();
  SFC::smp.r.p.b = flagB.checked();
  SFC::smp.r.p.h = flagH.checked();
  SFC::smp.r.p.i = flagI.checked();
  SFC::smp.r.p.z = flagZ.checked();
  SFC::smp.r.p.c = flagC.checked();
}
