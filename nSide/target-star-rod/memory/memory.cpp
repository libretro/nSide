#include "../laevateinn.hpp"
unique_pointer<MemoryEditor> memoryEditor;

MemoryEditor::MemoryEditor() {
  memoryEditor = this;

  layout.setMargin(5);
  setTitle("Memory Editor");

  gotoLabel.setText("Goto:");
  gotoAddress.setFont(Font().setFamily(Font::Mono)).onChange([&] {
    editor.setAddress(gotoAddress.text().hex());
    editor.update();
  }).onActivate([&] {
    gotoAddress.doChange();
  });
  source.append(ComboButtonItem().setText("CPU-Bus"));
  source.append(ComboButtonItem().setText("APU-Bus"));
  source.append(ComboButtonItem().setText("VRAM"));
  source.append(ComboButtonItem().setText("OAM"));
  source.append(ComboButtonItem().setText("CGRAM"));
  source.onChange({ &MemoryEditor::selectSource, this });
  exportMemory.setText("Export").onActivate({ &MemoryEditor::exportMemoryToDisk, this });
  autoUpdate.setText("Auto");
  update.setText("Update").onActivate({ &MemoryEditor::updateView, this });
  editor.setFont(Font().setFamily(Font::Mono));
  editor.setColumns(16).setRows(16);
  editor.onRead({ &MemoryEditor::read, this });
  editor.onWrite({ &MemoryEditor::write, this });

  setGeometry({128, 128, 585, 235});
}

auto MemoryEditor::read(uint addr) -> uint8_t {
  if(!SFC::system.loaded()) return 0x00;
  switch(source.selected().offset()) {
  case CPU:   return cpuDebugger->read(addr);
  case APU:   return smpDebugger->read(addr);
  case VRAM:  return SFC::ppu.vram[(addr & 0xffff) >> 1].byte(addr & 1);
  case OAM:   return SFC::ppu.obj.oam.read(addr % 544);
  case CGRAM: return SFC::ppu.screen.cgram[(addr & 0x1ff) >> 1].byte(addr & 1);
  }
  return ~0;
}

auto MemoryEditor::write(uint addr, uint8_t data) -> void {
  if(!SFC::system.loaded()) return;
  switch(source.selected().offset()) {
  case CPU:
    SFC::cartridge.rom.writeProtect(false);
    cpuDebugger->write(addr, data);
    SFC::cartridge.rom.writeProtect(true);
    debugger->cpuUsage.data[addr] = 0;
    break;
  case APU:
    smpDebugger->write(addr, data);
    debugger->apuUsage.data[addr] = 0;
    break;
  case VRAM:
    SFC::ppu.vram[(addr & 0xffff) >> 1].byte(addr & 1) = data;
    break;
  case OAM:
    SFC::ppu.obj.oam.write(addr % 544, data);
    break;
  case CGRAM:
    SFC::ppu.screen.cgram[(addr & 0x1ff) >> 1].byte(addr & 1) = data;
    break;
  }
}

auto MemoryEditor::selectSource() -> void {
  editor.setAddress(0);
  switch(source.selected().offset()) {
  case CPU:   editor.setLength(16 * 1024 * 1024); break;
  case APU:   editor.setLength(64 * 1024); break;
  case VRAM:  editor.setLength((SFC::ppu.vram.mask + 1) << 1); break;
  case OAM:   editor.setLength(544); break;
  case CGRAM: editor.setLength(512); break;
  }
  updateView();
}

auto MemoryEditor::exportMemoryToDisk() -> void {
  string filename = {program->mediumPaths(1), "debug/"};
  directory::create(filename);
  switch(source.selected().offset()) {
  case CPU:   filename.append("work.ram"); break;
  case APU:   filename.append("apu.ram"); break;
  case VRAM:  filename.append("video.ram"); break;
  case OAM:   filename.append("object.ram"); break;
  case CGRAM: filename.append("palette.ram"); break;
  }
  file fp;
  if(fp.open(filename, file::mode::write) == false) return;
  for(uint addr : range(editor.length())) fp.write(read(addr));
  fp.close();
  debugger->print("Exported memory to ", filename, "\n");
}

auto MemoryEditor::updateView() -> void {
  editor.update();
}
