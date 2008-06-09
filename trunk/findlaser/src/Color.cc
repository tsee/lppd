#include "Color.h"
#include <cmath>

namespace FindLaser {
  float InvSqrt(float x) {
/*    const float xhalf = 0.5f*x;
    int i = *(int*)&x;
    i = 0x5f375a86- (i>>1);
    x = *(float*)&i;
    return x*(1.5f-xhalf*x*x);
*/
    return 1./sqrt(x);
  }

  Color::Color(unsigned char r, unsigned char g, unsigned char b) : red(r), green(g), blue(b) {
  }
   
  Color Color::GetNormalized() {
    float scale = 255. *InvSqrt((float) (red*red+green*green+blue*blue));
    //float scale = 255. / pow((float) (red*red+green*green+blue*blue), 0.5);
    float r = (float)red   * scale;
    float g = (float)green * scale;
    float b = (float)blue  * scale;
    Color col((unsigned char)r, (unsigned char)g, (unsigned char)b);
    return col;
  }

} // end namespace FindLaser
