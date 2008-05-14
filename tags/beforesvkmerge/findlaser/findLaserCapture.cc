#include "myimages.h"
#include "v4lcapture.h"

#include <string>
#include <vector>
#include <iostream>


#include <X11/Xlib.h>
#include <stdio.h>


using namespace FindLaser;
using namespace std;

int main (int argc, char**  argv) {

  unsigned int imageSX = 320, imageSY = 240;

  ImageCapture cap("/dev/video0");

  if (!cap.Initialize()) {
    cerr << cap.GetError() << endl;
    return(1);
  }
  if (!cap.SetImageSize(imageSX, imageSY)) {
    cerr << cap.GetError() << endl;
    return(1);
  }

  /*if (!cap.AdjustBrightness()) {
    cerr << cap.GetError() << endl;
    return(1);
  }
  */

  cap.SetBrightness(10000);

  Color red(255,0,0);
  Color green(0,255,0);
  Color blue(0,0,255);

  const unsigned int screenSizeX = 1280, screenSizeY = 800;

  double sLastCX = 0.;
  double sLastCY = 0.;
  double lastCX = 0.;
  double lastCY = 0.;
  unsigned int frame = 0;
  while (1) {
    unsigned int size = 0;
    unsigned char* imgptr = cap.CaptureImagePointer(size);

    ColorImage img;
    img.ReverseCopyFromMemory(imageSX, imageSY, imgptr);
    //img.SaveAsJPEG("out.jpg", 100);

    //GreyImage normRedProj = *img.NormalizedColorProjection(red);
    //GreyImage light = *img.ToGreyscale();
  
    double cx = 0, cy = 0;
    double thresholdColor = 120;
    double thresholdLight = 220;
    img.FindLaserCentroid(
      cx, cy, red, thresholdColor, thresholdLight    
    );
//    if ( pow(lastCX-cx,2)+pow(lastCY-cy,2)
  
    frame++;
    cerr << frame;

    if (cx >= 0) {
      double relCX = (0.5*cx+0.3*lastCX+0.2*sLastCX) / img.GetColumns();
      double relCY = (0.5*cy+0.3*lastCY+0.2*sLastCX) / img.GetRows();
      sLastCX = lastCX;
      sLastCY = lastCY;
      lastCX = cx;
      lastCY = cy;
    
      unsigned int screenCX = (unsigned int) (screenSizeX * relCX);
      unsigned int screenCY = (unsigned int) (screenSizeY * relCY);

      Display *display = XOpenDisplay(NULL);
      Window root = DefaultRootWindow(display);
      XWarpPointer(display, None, root, 0, 0, 0, 0, screenCX, screenCY);
      XCloseDisplay(display);
      cerr << " " << cx << " " << cy << " - " << screenCX <<" " << screenCY;
    } 
    
    cerr << endl;
  }

  //light.SaveAsJPEG("normRedProj.jpg", 100);
  //

  //GreyImage redCut = *normRedProj.CutOnThreshold((unsigned char)thresholdColor, 0);
  //redCut.SaveAsJPEG("redCut.jpg", 100);
  
  /*unsigned char* ppmdata = redCut.AsPNM(size);
  FILE *fp = fopen("redCut.pnm", "wb");
  fwrite(ppmdata, size, 1, fp);
  fclose(fp);
  */
  return(0);
}


