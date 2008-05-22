#include "Color.h"
#include <cmath>

namespace FindLaser {

  Color::Color(unsigned char r, unsigned char g, unsigned char b) : red(r), green(g), blue(b) {
  }
   
  Color Color::GetNormalized() {
    float scale = 255. / pow((float) (red*red+green*green+blue*blue), 0.5);
    float r = (float)red   * scale;
    float g = (float)green * scale;
    float b = (float)blue  * scale;
    Color col((unsigned char)r, (unsigned char)g, (unsigned char)b);
    return col;
  }
} // end namespace FindLaser
