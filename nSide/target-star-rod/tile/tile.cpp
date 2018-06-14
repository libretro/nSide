#include "../laevateinn.hpp"
unique_pointer<TileViewer> tileViewer;

TileViewer::TileViewer() {
  tileViewer = this;

  layout.setMargin(5);
  setTitle("Tile Viewer");
  statusBar.setFont(Font().setBold());
  statusBar.setVisible();

  modeLabel.setText("Mode:");
  modeSelection.append(ComboButtonItem().setText("2BPP"));
  modeSelection.append(ComboButtonItem().setText("4BPP"));
  modeSelection.append(ComboButtonItem().setText("8BPP"));
  modeSelection.append(ComboButtonItem().setText("Mode 7"));
  modeSelection.onChange({ &TileViewer::modeChanged, this });
  paletteLabel.setText("Palette:");
  paletteSelection.onChange({ &TileViewer::updateTiles, this });
  autoUpdate.setText("Auto");
  update.setText("Update").onActivate({ &TileViewer::updateTiles, this });

  columnLinear.setText("Linear Tiles").onActivate({ &TileViewer::updateTiles, this }).setChecked();
  column16.setText("16-tile Columns").onActivate({ &TileViewer::updateTiles, this });

  memoryVRAM.setText("VRAM").onActivate({ &TileViewer::updateTiles, this }).setChecked();
  memoryWRAM.setText("WRAM").onActivate({ &TileViewer::updateTiles, this });

  wramAddress.setText("7e0000").onChange({ &TileViewer::updateTiles, this });

  canvas.setSize({512, 512});

  canvas.onMouseLeave([&] { statusBar.setText(""); });
  canvas.onMouseMove([&](Position position) {
    bool doubleVRAM = SFC::ppu.vram.mask & 0x8000;

    uint x = position.x(), y = position.y();
    uint mode = modeSelection.selected().offset();

    x >>= 3, y >>= 3;
    if((x >= columnCount * columnWidth)
    || (y >= columnHeight)) { statusBar.setText(""); return; }
    string output = { x, ", ", y, ", " };
    uint tile = 0, address = 0;

    tile = x / columnWidth * columnWidth * columnHeight + y * columnWidth + x % columnWidth;
    if(memoryVRAM.checked()) tile += scroll.position() * (0x40 << mode);
    address = tile * (16 << mode) + (mode == SFC::PPU::Background::Mode::Mode7);
    if(memoryWRAM.checked()) address += wramAddress.text().hex();
    output.append(
      "Tile: 0x", hex(tile, doubleVRAM ? 4L : 3L), ", "
      "Address: 0x", hex(address, memoryWRAM.checked() ? 6L : doubleVRAM ? 5L : 4L)
    );
    statusBar.setText(output);
  });

  scroll.setLength(1).setPosition(0).onChange({ &TileViewer::updateTiles, this });

  modeChanged();

  setSize(layout.minimumSize());
  setAlignment({1.0, 0.0});
}

auto TileViewer::modeChanged() -> void {
  paletteSelection.reset();
  switch(modeSelection.selected().offset()) {
  case SFC::PPU::Background::Mode::BPP2:
    //When using 128 KiB VRAM, enable the scrollbar.
    scroll.setLength((bool)(SFC::ppu.vram.mask & 0x8000) * 64 + 1);
    for(uint bg : range(4))
    for(uint palette : range(8)) {
      paletteSelection.append(ComboButtonItem().setText({"BG", bg, " ", palette}));
    }
    break;
  case SFC::PPU::Background::Mode::BPP4:
    scroll.setLength(1);
    for(uint palette : range(8)) {
      paletteSelection.append(ComboButtonItem().setText({"BG ", palette}));
    }
    for(uint palette : range(8)) {
      paletteSelection.append(ComboButtonItem().setText({"SP ", palette}));
    }
    break;
  case SFC::PPU::Background::Mode::BPP8:
    scroll.setLength(1);
    paletteSelection.append(ComboButtonItem().setText("BG"));
    break;
  case SFC::PPU::Background::Mode::Mode7:
    scroll.setLength(1);
    paletteSelection.append(ComboButtonItem().setText("BG"));
    paletteSelection.append(ComboButtonItem().setText("EXTBG"));
    break;
  }
  scroll.setPosition(min(scroll.position(), scroll.length()));
  updateTiles();
}

auto TileViewer::updateTiles() -> void {
  bool wram = memoryWRAM.checked();
  scroll.setVisible(!wram && scroll.length() > 1);

  uint32_t* dp = canvas.data();
  for(uint x : range(canvas.geometry().width() * canvas.geometry().height())) {
    *dp++ = 0xff800000;
  }
  dp = canvas.data();
  const uint16* vp = nullptr;
  uint wp;

  uint mode = modeSelection.selected().offset();

  if(wram) {
    wp = wramAddress.text().hex();
    if(column16.checked()) {
      columnCount  =  4;
      columnWidth  = 16;
      columnHeight = 64;
    } else {
      columnCount  =  1;
      columnWidth  = 64;
      columnHeight = 64;
    }
  } else {
    vp = SFC::ppu.vram.data + scroll.position() * (0x200 << mode);
    bool doubleVRAM = SFC::ppu.vram.mask & 0x8000;
    if(column16.checked()) {
      columnWidth  = 16;
      columnHeight = min(64, (256 << doubleVRAM) >> mode);
      columnCount  = min(4, ((4096 << doubleVRAM) >> mode) / (columnWidth * columnHeight));
    } else {
      columnCount  =  1;
      columnWidth  = 64;
      columnHeight = min(64, (64 << doubleVRAM) >> mode);
    }
  }

  uint color;
  #define sp(index)\
    (wram ? (uint16)(cpuDebugger->read(wp + index * 2) | cpuDebugger->read(wp + index * 2 + 1) << 8) : vp[index])
  #define advance(shift)\
    if(wram) wp += (shift) * 2;\
    else     vp += (shift);
  switch(mode) {
  case SFC::PPU::Background::Mode::BPP2:
  case SFC::PPU::Background::Mode::BPP4:
  case SFC::PPU::Background::Mode::BPP8:
    for(uint column : range(columnCount))
    for(uint tileY : range(columnHeight))
    for(uint tileX : range(columnWidth)) {
      for(uint y : range(8)) {
        uint16 d[] = {sp(0), sp(8), sp(16), sp(24)};
        for(uint x : range(8)) {
          color = 0;
          for(uint p : range(1 << mode)) {
            color += d[p] & 0x0080 ? 1 << (p << 1) : 0;
            color += d[p] & 0x8000 ? 2 << (p << 1) : 0;
          }
          for(auto& b : d) b <<= 1;
          color += paletteSelection.selected().offset() << (2 << mode);
          color = SFC::ppu.screen.cgram[color];
          color = (255u << 24) |
            (image::normalize(color >>  0 & 31, 5, 8) << 16) |
            (image::normalize(color >>  5 & 31, 5, 8) <<  8) |
            (image::normalize(color >> 10 & 31, 5, 8) <<  0);
          dp[(column * columnWidth * 8) + (tileY * 8 + y) * 512 + (tileX * 8 + x)] = color;
        }
        advance(1);
      }
      advance((8 << mode) - 8);
    }
    break;

  case SFC::PPU::Background::Mode::Mode7:
    for(uint column : range(columnCount))
    for(uint tileY : range(columnHeight))
    for(uint tileX : range(columnWidth)) {
      for(uint y : range(8)) {
        for(uint x : range(8)) {
          color = sp(x) >> 8;
          if(paletteSelection.selected().offset() == 1) color &= 0x7f;
          color = SFC::ppu.screen.cgram[color];
          color = (255u << 24) |
            (image::normalize(color >>  0 & 31, 5, 8) << 16) |
            (image::normalize(color >>  5 & 31, 5, 8) <<  8) |
            (image::normalize(color >> 10 & 31, 5, 8) <<  0);
          dp[(column * columnWidth * 8) + (tileY * 8 + y) * 512 + (tileX * 8 + x)] = color;
        }
        advance(8);
      }
    }
    break;
  }
  #undef sp
  #undef advance

  canvas.update();
}
