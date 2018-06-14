AboutWindow::AboutWindow() {
  aboutWindow = this;

  setTitle("About nSide ...");
  setBackgroundColor({255, 255, 240});
  layout.setMargin(10);
  auto logo = image{Resource::Logo};
  logo.alphaBlend(0xfffff0);
  canvas.setIcon(logo);
  informationLeft.setFont(Font().setBold()).setAlignment(1.0).setText({
    "Version:\n",
    "Based on:\n",
    "Fork Author:\n",
    "License:\n",
    "Website of ", Emulator::Name, ":"
  });
  informationRight.setFont(Font().setBold()).setAlignment(0.0).setText({
    Emulator::ForkVersion, "\n",
    Emulator::Name, " ", Emulator::Version, " by ", Emulator::Author, "\n",
    Emulator::ForkAuthor, "\n",
    Emulator::License, "\n",
    Emulator::Website
  });
  informationBottom.setFont(Font().setBold()).setAlignment(0.0).setText({
    "Contributors to ", Emulator::Name, ":\n",
    Emulator::Contributors.merge("\n")
  });

  setResizable(false);
  setSize(layout.minimumSize());
  setCentered();
  setDismissable();
}
