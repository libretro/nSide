#include "../laevateinn.hpp"
unique_pointer<PropertiesViewer> propertiesViewer;

PropertiesViewer::PropertiesViewer() {
  propertiesViewer = this;

  layout.setMargin(5);
  setTitle("Properties Viewer");

  sourceLabel.setText("Source:");
  sourceSelection.append(ComboButtonItem().setText("PPU"));
  sourceSelection.append(ComboButtonItem().setText("CPU"));
  sourceSelection.append(ComboButtonItem().setText("DMA"));
  autoUpdate.setText("Auto");
  update.setText("Update");
  properties.setFont(Font().setFamily(Font::Mono));

  sourceSelection.onChange({ &PropertiesViewer::updateProperties, this });
  update.onActivate({ &PropertiesViewer::updateProperties, this });

  setGeometry({128, 128, 300, 400});
}

auto PropertiesViewer::updateProperties() -> void {
  string output;

  switch(sourceSelection.selected().offset()) {
  case PPU: {
    output.append("$2100  INIDISP\n"
      "  Display Brightness = ", SFC::ppu.io.displayBrightness, "\n",
      "  Display Disable = ", SFC::ppu.io.displayDisable, "\n",
      "\n"
    );

    output.append("$2101  OBSEL\n"
      "  OAM Tiledata Address = $", hex(SFC::ppu.obj.io.tiledataAddress << 1, 4L), "\n",
      "  OAM Name Select = $", hex(SFC::ppu.obj.io.nameselect << 12 << 1, 4L), "\n",
      "  OAM Base Size = ", SFC::ppu.obj.io.baseSize, "\n",
      "\n"
    );

    output.append("$2102  OAMADDL\n"
                  "$2103  OAMADDH\n"
      "  OAM Priority = ", SFC::ppu.io.oamPriority, "\n",
      "  OAM Base Address = $", hex(SFC::ppu.io.oamBaseAddress, 4L), "\n",
      "\n"
    );

    output.append("$2105  BGMODE\n"
      "  BG Mode = ", SFC::ppu.io.bgMode, "\n",
      "  BG Priority = ", SFC::ppu.io.bgPriority, "\n",
      "  BG1 Tile Size = ", SFC::ppu.bg1.io.tileSize ? "16×16" : "8×8", "\n",
      "  BG2 Tile Size = ", SFC::ppu.bg2.io.tileSize ? "16×16" : "8×8", "\n",
      "  BG3 Tile Size = ", SFC::ppu.bg3.io.tileSize ? "16×16" : "8×8", "\n",
      "  BG4 Tile Size = ", SFC::ppu.bg4.io.tileSize ? "16×16" : "8×8", "\n",
      "\n"
    );

    uint4 size = 1u + SFC::PPU::Background::Mosaic::size;
    output.append("$2106  MOSAIC\n"
      "  BG1 Mosaic = ", SFC::ppu.bg1.mosaic.enable ? "Enabled" : "Disabled", "\n",
      "  BG2 Mosaic = ", SFC::ppu.bg2.mosaic.enable ? "Enabled" : "Disabled", "\n",
      "  BG3 Mosaic = ", SFC::ppu.bg3.mosaic.enable ? "Enabled" : "Disabled", "\n",
      "  BG4 Mosaic = ", SFC::ppu.bg4.mosaic.enable ? "Enabled" : "Disabled", "\n",
      "  Size = ", size, "×", size, "\n",
      "\n"
    );

    static string_vector screenSizes = {"32x×32y", "64x×32y", "32x×64y", "64x×64y"};

    output.append("$2107  BG1SC\n"
                  "$2108  BG2SC\n"
                  "$2109  BG3SC\n"
                  "$210a  BG4SC\n"
      "  BG1 Screen Size = ", screenSizes[SFC::ppu.bg1.io.screenSize], "\n",
      "  BG1 Screen Address = $", hex(SFC::ppu.bg1.io.screenAddress << 1, 4L), "\n",
      "  BG2 Screen Size = ", screenSizes[SFC::ppu.bg2.io.screenSize], "\n",
      "  BG2 Screen Address = $", hex(SFC::ppu.bg2.io.screenAddress << 1, 4L), "\n",
      "  BG3 Screen Size = ", screenSizes[SFC::ppu.bg3.io.screenSize], "\n",
      "  BG3 Screen Address = $", hex(SFC::ppu.bg3.io.screenAddress << 1, 4L), "\n",
      "  BG4 Screen Size = ", screenSizes[SFC::ppu.bg4.io.screenSize], "\n",
      "  BG4 Screen Address = $", hex(SFC::ppu.bg4.io.screenAddress << 1, 4L), "\n",
      "\n"
    );

    output.append("$210b  BG12NBA\n"
                  "$210c  BG34NBA\n"
      "  BG1 Tiledata Address = $", hex(SFC::ppu.bg1.io.tiledataAddress << 1, 4L), "\n",
      "  BG2 Tiledata Address = $", hex(SFC::ppu.bg2.io.tiledataAddress << 1, 4L), "\n",
      "  BG3 Tiledata Address = $", hex(SFC::ppu.bg3.io.tiledataAddress << 1, 4L), "\n",
      "  BG4 Tiledata Address = $", hex(SFC::ppu.bg4.io.tiledataAddress << 1, 4L), "\n",
      "\n"
    );

    output.append("$210d  BG1HOFS\n"
                  "$210e  BG1VOFS\n"
                  "$210f  BG2HOFS\n"
                  "$2110  BG2VOFS\n"
                  "$2111  BG3HOFS\n"
                  "$2112  BG3VOFS\n"
                  "$2113  BG4HOFS\n"
                  "$2114  BG4VOFS\n"
      "  BG1 H Offset = $", hex(SFC::ppu.bg1.io.hoffset, 4L), "\n",
      "  BG1 V Offset = $", hex(SFC::ppu.bg1.io.voffset, 4L), "\n",
      "  BG2 H Offset = $", hex(SFC::ppu.bg2.io.hoffset, 4L), "\n",
      "  BG2 V Offset = $", hex(SFC::ppu.bg2.io.voffset, 4L), "\n",
      "  BG3 H Offset = $", hex(SFC::ppu.bg3.io.hoffset, 4L), "\n",
      "  BG3 V Offset = $", hex(SFC::ppu.bg3.io.voffset, 4L), "\n",
      "  BG4 H Offset = $", hex(SFC::ppu.bg4.io.hoffset, 4L), "\n",
      "  BG4 V Offset = $", hex(SFC::ppu.bg4.io.voffset, 4L), "\n",
      "\n"
    );

    static string_vector vramMappings = {"Direct", "2-bit", "4-bit", "8-bit"};
    output.append("$2115  VMAINC\n"
      "  VRAM Increment Mode = ", SFC::ppu.io.vramIncrementMode ? "high" : "low", "\n",
      "  VRAM Mapping        = ", vramMappings[SFC::ppu.io.vramMapping], "\n",
      "  VRAM Increment Size = ", SFC::ppu.io.vramIncrementSize, "\n",
      "\n"
    );

    output.append("$2116  VMADDL\n"
                  "$2117  VMADDH\n"
      "  VRAM Address = $", hex(SFC::ppu.io.vramAddress << 1, 4L),
      " ($", hex(SFC::ppu.io.vramAddress, 4L), ")\n",
      "\n"
    );

    output.append("$211a  M7SEL\n"
      "  Mode 7 H Flip = ", SFC::ppu.io.hflipMode7 ? "Enabled" : "Disabled", "\n",
      "  Mode 7 V Flip = ", SFC::ppu.io.vflipMode7 ? "Enabled" : "Disabled", "\n",
      "  Mode 7 Repeat = ", (
        SFC::ppu.io.repeatMode7 <= 1 ? "Repeat"   :
        SFC::ppu.io.repeatMode7 == 2 ? "BG Color" :
                                       "Tile 0"
      ), "\n",
      "\n"
    );

    output.append("$2126  WH0\n"
                  "$2127  WH1\n"
      "Window One Left  = 0x", hex(SFC::ppu.window.io.oneLeft, 2L), "\n"
      "Window One Right = 0x", hex(SFC::ppu.window.io.oneRight, 2L), "\n"
      "\n"
    );

    output.append("$2128  WH2\n"
                  "$2129  WH3\n"
      "Window Two Left  = 0x", hex(SFC::ppu.window.io.twoLeft, 2L), "\n"
      "Window Two Right = 0x", hex(SFC::ppu.window.io.twoRight, 2L), "\n"
      "\n"
    );

    output.append("$212c  TM\n"
      "  BG1 Above = ", SFC::ppu.bg1.io.aboveEnable ? "Enabled" : "Disabled", "\n",
      "  BG2 Above = ", SFC::ppu.bg2.io.aboveEnable ? "Enabled" : "Disabled", "\n",
      "  BG3 Above = ", SFC::ppu.bg3.io.aboveEnable ? "Enabled" : "Disabled", "\n",
      "  BG4 Above = ", SFC::ppu.bg4.io.aboveEnable ? "Enabled" : "Disabled", "\n",
      "  OBJ Above = ", SFC::ppu.obj.io.aboveEnable ? "Enabled" : "Disabled", "\n",
      "\n"
    );

    output.append("$212d  TS\n"
      "  BG1 Below = ", SFC::ppu.bg1.io.belowEnable ? "Enabled" : "Disabled", "\n",
      "  BG2 Below = ", SFC::ppu.bg2.io.belowEnable ? "Enabled" : "Disabled", "\n",
      "  BG3 Below = ", SFC::ppu.bg3.io.belowEnable ? "Enabled" : "Disabled", "\n",
      "  BG4 Below = ", SFC::ppu.bg4.io.belowEnable ? "Enabled" : "Disabled", "\n",
      "  OBJ Below = ", SFC::ppu.obj.io.belowEnable ? "Enabled" : "Disabled", "\n",
      "\n"
    );

    output.append("$212e  TMW\n"
      "  BG1 Above Window = ", SFC::ppu.window.io.bg1.aboveEnable ? "Enabled" : "Disabled", "\n",
      "  BG2 Above Window = ", SFC::ppu.window.io.bg2.aboveEnable ? "Enabled" : "Disabled", "\n",
      "  BG3 Above Window = ", SFC::ppu.window.io.bg3.aboveEnable ? "Enabled" : "Disabled", "\n",
      "  BG4 Above Window = ", SFC::ppu.window.io.bg4.aboveEnable ? "Enabled" : "Disabled", "\n",
      "  OBJ Above Window = ", SFC::ppu.window.io.obj.aboveEnable ? "Enabled" : "Disabled", "\n",
      "\n"
    );

    output.append("$212f  TSW\n"
      "  BG1 Below Window = ", SFC::ppu.window.io.bg1.belowEnable ? "Enabled" : "Disabled", "\n",
      "  BG2 Below Window = ", SFC::ppu.window.io.bg2.belowEnable ? "Enabled" : "Disabled", "\n",
      "  BG3 Below Window = ", SFC::ppu.window.io.bg3.belowEnable ? "Enabled" : "Disabled", "\n",
      "  BG4 Below Window = ", SFC::ppu.window.io.bg4.belowEnable ? "Enabled" : "Disabled", "\n",
      "  OBJ Below Window = ", SFC::ppu.window.io.obj.belowEnable ? "Enabled" : "Disabled", "\n",
      "\n"
    );

    output.append("$2130  CGWSEL\n"
      "  Direct Color = ", SFC::ppu.screen.io.directColor ? "Enabled" : "Disabled", "\n",
      "  Blend Mode   = ", SFC::ppu.screen.io.blendMode   ? "Enabled" : "Disabled", "\n",
      "  Window Color Below Mask = ", SFC::ppu.window.io.col.belowMask ? "Enabled" : "Disabled", "\n",
      "  Window Color Above Mask = ", SFC::ppu.window.io.col.aboveMask ? "Enabled" : "Disabled", "\n",
      "\n"
    );

    break;
  }
  
  case CPU: {
    output.append("$4200  NMITIMEN\n"
      "  Auto Joypad Poll = ", SFC::cpu.io.autoJoypadPoll, "\n",
      "  Horz IRQ = ", SFC::cpu.io.hirqEnabled ? "Enabled" : "Disabled", "\n",
      "  Vert IRQ = ", SFC::cpu.io.virqEnabled ? "Enabled" : "Disabled", "\n",
      "  NMI      = ", SFC::cpu.io.nmiEnabled  ? "Enabled" : "Disabled", "\n",
      "\n"
    );

    output.append("$4202  WRMPYA\n"
                  "$4203  WRMPYB\n"
      "  Multiplicand 1 = ", SFC::cpu.io.wrmpya, "\n",
      "  Multiplicand 2 = ", SFC::cpu.io.wrmpyb, "\n",
      "\n"
    );

    output.append("$4204  WRDIVL\n"
                  "$4205  WRDIVH\n"
                  "$4206  WRDIVB\n"
      "  Dividend = ", SFC::cpu.io.wrdiva, "\n",
      "  Divisor  = ", SFC::cpu.io.wrdivb, "\n",
      "\n"
    );

    output.append("$420d  MEMSEL\n"
      "  ROM Speed = ", SFC::cpu.io.romSpeed == 6 ? "Fast" : "Slow", "\n",
      "\n"
    );

    break;
  }
  
  case DMA: {
    for(uint c : range(8)) {
      output.append("$43", c, "0  DMAP", c, "\n"
        "  Direction        = ", SFC::cpu.channel[c].direction ? "decrement" : "increment", "\n",
        "  (HDMA) Indirect  = ", SFC::cpu.channel[c].indirect,        "\n",
        "  Reverse Transfer = ", SFC::cpu.channel[c].reverseTransfer, "\n",
        "  Fixed Transfer   = ", SFC::cpu.channel[c].fixedTransfer,   "\n",
        "  Transfer Mode    = ", SFC::cpu.channel[c].transferMode,    "\n",
        "\n"
      );

      output.append("$43", c, "1  DDBAD", c, "\n"
        "  Bus B Address = $", hex(SFC::cpu.channel[c].targetAddress, 2L), "\n",
        "\n"
      );

      output.append("$43", c, "2  A1T", c, "L\n"
                    "$43", c, "3  A1T", c, "H\n"
        "  Bus A Address = $", hex(SFC::cpu.channel[c].sourceAddress, 4L), "\n",
        "\n"
      );

      output.append("$43", c, "4  A1B", c, "\n"
        "  Bus A Bank = $", hex(SFC::cpu.channel[c].sourceBank, 2L), "\n",
        "\n"
      );

      output.append("$43", c, "5  DAS", c, "L\n"
                    "$43", c, "6  DAS", c, "H\n"
        "  (DMA) Transfer Size     = $", hex((SFC::cpu.channel[c].transferSize - 1) % 65536 + 1, 4L), "\n",
        "  (HDMA) Indirect Address = $", hex( SFC::cpu.channel[c].indirectAddress,               4L), "\n",
        "\n"
      );

      output.append("$43", c, "7  DASB", c, "\n"
        "  (HDMA) Indirect Bank = $", hex(SFC::cpu.channel[c].indirectBank, 2L), "\n",
        "\n"
      );
    }

    break;
  }

  }

  properties.setText(output);
}
