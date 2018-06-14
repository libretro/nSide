unique_pointer<CPURegisterEditor> cpuRegisterEditor;

CPURegisterEditor::CPURegisterEditor() {
  cpuRegisterEditor = this;
  setTitle("CPU Register Editor");

  layout.setMargin(5);
  regALabel.setText("A:");
  regAValue.setFont(Font().setFamily(Font::Mono));
  regXLabel.setText("X:");
  regXValue.setFont(Font().setFamily(Font::Mono));
  regYLabel.setText("Y:");
  regYValue.setFont(Font().setFamily(Font::Mono));
  regSLabel.setText("S:");
  regSValue.setFont(Font().setFamily(Font::Mono));
  regDLabel.setText("D:");
  regDValue.setFont(Font().setFamily(Font::Mono));
  regBLabel.setText("B:");
  regBValue.setFont(Font().setFamily(Font::Mono));
  flagN.setText("N");
  flagV.setText("V");
  flagM.setText("M");
  flagX.setText("X");
  flagD.setText("D");
  flagI.setText("I");
  flagZ.setText("Z");
  flagC.setText("C");
  flagE.setText("E");
  update.setText("Update").onActivate([&] {
    saveRegisters();
    cpuDebugger->updateDisassembly();
    setVisible(false);
  });

  loadRegisters();

  setGeometry({{128, 128}, layout.minimumSize()});
}

void CPURegisterEditor::loadRegisters() {
  regAValue.setText(hex(SFC::cpu.r.a, 4L));
  regXValue.setText(hex(SFC::cpu.r.x, 4L));
  regYValue.setText(hex(SFC::cpu.r.y, 4L));
  regSValue.setText(hex(SFC::cpu.r.s, 4L));
  regDValue.setText(hex(SFC::cpu.r.d, 4L));
  regBValue.setText(hex(SFC::cpu.r.b, 2L));
  flagN.setChecked(SFC::cpu.r.p.n);
  flagV.setChecked(SFC::cpu.r.p.v);
  flagM.setChecked(SFC::cpu.r.p.m);
  flagX.setChecked(SFC::cpu.r.p.x);
  flagD.setChecked(SFC::cpu.r.p.d);
  flagI.setChecked(SFC::cpu.r.p.i);
  flagZ.setChecked(SFC::cpu.r.p.z);
  flagC.setChecked(SFC::cpu.r.p.c);
  flagE.setChecked(SFC::cpu.r.e);
}

void CPURegisterEditor::saveRegisters() {
  SFC::cpu.r.a = regAValue.text().hex();
  SFC::cpu.r.x = regXValue.text().hex();
  SFC::cpu.r.y = regYValue.text().hex();
  SFC::cpu.r.s = regSValue.text().hex();
  SFC::cpu.r.d = regDValue.text().hex();
  SFC::cpu.r.b = regBValue.text().hex();
  SFC::cpu.r.p.n = flagN.checked();
  SFC::cpu.r.p.v = flagV.checked();
  SFC::cpu.r.p.m = flagM.checked();
  SFC::cpu.r.p.x = flagX.checked();
  SFC::cpu.r.p.d = flagD.checked();
  SFC::cpu.r.p.i = flagI.checked();
  SFC::cpu.r.p.z = flagZ.checked();
  SFC::cpu.r.p.c = flagC.checked();
  SFC::cpu.r.e = flagE.checked();
  //SFC::cpu.updateTable();  //cache E/M/X flags
}
