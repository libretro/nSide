auto Theme::Font::load() -> void {
  file fontfile;

  if(fontfile.open(locate({"Themes/", theme->name, "/font.bfn"}), file::mode::read)) {
    uint glyphCount = fontfile.readl(4);

    for(uint index : range(glyphCount)) {
      uint codepoint = fontfile.readl(4);
      uint width     = fontfile.readl(1);
      uint height    = fontfile.readl(1);
      uint offset    = fontfile.readl(4);
      glyphs.append(new image);
      glyphs[index]->allocate(width, height);
      glyphs[index]->fill(0x00000000);
      u[codepoint] = glyphs[index];
    }

    double scale = graphics->buffer.height() / 240.0;
    for(auto& glyph : glyphs) {
      uint byteWidth = (glyph->width() + 7) / 8;
      uint8_t* dp = glyph->data();
      uint8_t byte;
      for(uint y : range(glyph->height())) {
        for(uint x : range(glyph->width())) {
          if((x & 7) == 0) byte = fontfile.read();
          if(byte & (0x80 >> (x & 7))) glyph->write(dp, color);
          dp += glyph->stride();
        }
      }
      glyph->scale(glyph->width() * scale, glyph->height() * scale, false);
    }

    fontfile.close();
  }
}
