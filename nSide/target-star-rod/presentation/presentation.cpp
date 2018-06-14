#include "../laevateinn.hpp"
unique_pointer<Presentation> presentation;

Presentation::Presentation() {
  presentation = this;
  setTitle("Video");
  setBackgroundColor({0, 0, 0});
  setResizable(false);
  statusBar.setFont(Font().setBold());
  statusBar.setVisible();

  setSize({512, 480});
  setAlignment({0.0, 0.0});

  splash.allocate(512, 480);
  splash.verticalGradient(0xff00005f, 0xff000000, 512, 480, 256, 0);
  nall::image floor;
  floor.allocate(512, 480);
  floor.radialGradient(0xffff0000, 0x00000000, 384, 240, 256, 415);
  splash.impose(image::blend::sourceAlpha, 0, 0, floor, 0, 0, floor.width(), floor.height());
  nall::image logo(resource::star_rod);
  splash.impose(image::blend::sourceAlpha, (512 - logo.width()) / 2, (480 - logo.height()) / 2, logo, 0, 0, logo.width(), logo.height());
}

auto Presentation::drawSplashScreen() -> void {
  uint32_t* output;
  uint pitch;
  if(video->lock(output, pitch, 512, 480)) {
    for(uint y = 0; y < 480; y++) {
      memcpy((uint8_t*)output + y * pitch, splash.data() + y * splash.pitch(), 512 * sizeof(uint32_t));
    }
    video->unlock();
    video->output();
  }
}
