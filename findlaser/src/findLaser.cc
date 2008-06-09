#include "Color.h"
#include "Image.h"
#include "ColorImage.h"
#include "GreyImage.h"

#include <string>
#include <vector>
#include <iostream>


#include <X11/Xlib.h>
#include <stdio.h>


using namespace FindLaser;
using namespace std;

int main (int argc, char**  argv) {
  if (argc < 2) {
    cerr << "Need more arguments!" << endl;
    return(1);
  }
  string file = string(argv[1]);

  ColorImage img;
  img.ReadFromFile(file);
  cerr << "Rows: " << img.GetRows() << " Cols: " << img.GetColumns() << endl;

  Color red(255,0,0);
  Color green(0,255,0);
  Color blue(0,0,255);

  GreyImage normRedProj = *img.NormalizedColorProjection(red);
  GreyImage light = *img.ToGreyscale();
  
  double cx = 0, cy = 0;
  double thresholdColor = 220;
  double thresholdLight = 120;
  img.FindLaserCentroid(
    cx, cy, red, thresholdColor, thresholdLight    
  );
  cerr << cx << " " << cy << endl;

  unsigned int screenSizeX = 1280, screenSizeY = 800;

  double relCX = cx / img.GetColumns();
  double relCY = cy / img.GetRows();
  
  unsigned int screenCX = (unsigned int) (screenSizeX * relCX);
  unsigned int screenCY = (unsigned int) (screenSizeY * relCY);

    Display *display = XOpenDisplay(NULL);
    Window root = DefaultRootWindow(display);
    XWarpPointer(display, None, root, 0, 0, 0, 0, screenCX, screenCY);
    XCloseDisplay(display);

  //light.SaveAsJPEG("normRedProj.jpg", 100);

  GreyImage redCut = *normRedProj.CutOnThreshold((unsigned char)thresholdColor, 0);
  redCut.SaveAsJPEG("redCut.jpg", 100);
  
  unsigned int size = 0;
  //unsigned char* ppmdata = redCut.AsPNM(size);
  //FILE *fp = fopen("redCut.pnm", "wb");
  //fwrite(ppmdata, size, 1, fp);
  //fclose(fp);

  
  return(0);
}


