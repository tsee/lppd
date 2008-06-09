#include <string>
#include <iostream>

#include "GeometryCorrection.h"
using namespace std;
using namespace FindLaser;

int main (int argc, char** argv) {
  GeometryCorrection g;
  g.SetSourceImageDimensions( 320, 240 );
  g.SetTargetImageDimensions( 640, 480 );
  g.SetImageUpperLeft(0, 0);
  g.SetImageUpperRight(320, 0);
  g.SetImageLowerLeft(0, 240);
  g.SetImageLowerRight(320, 240);
  
  float x,y;
  g.GetImageCoordinates(0,0,x,y);
  cout << x << " " << y << endl;
  g.GetImageCoordinates(320,240,x,y);
  cout << x << " " << y << endl;
  g.GetImageCoordinates(160,120,x,y);
  cout << x << " " << y << endl;

  g.SetImageUpperLeft(0, 10);
  g.SetImageUpperRight(320, 10);
  g.SetImageLowerLeft(0, 240);
  g.SetImageLowerRight(320, 240);

  g.GetImageCoordinates(0,0,x,y);
  cout << x << " " << y << endl;
  g.GetImageCoordinates(320,240,x,y);
  cout << x << " " << y << endl;
  g.GetImageCoordinates(160,120,x,y);
  cout << x << " " << y << endl;
}


