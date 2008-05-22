#include "Color.h"
#include "Image.h"
#include "ColorImage.h"
#include "GreyImage.h"

#include "v4lcapture.h"
#include "settings.h"

#include <string>
#include <vector>
#include <iostream>


#include <X11/Xlib.h>
#include <stdio.h>


using namespace FindLaser;
using namespace std;

int main (int argc, char**  argv) {
  
  const string confFile = "settings.txt";
  Configuration cfg(confFile);

  const unsigned int imageSX = cfg.GetCameraImageSizeX(), imageSY = cfg.GetCameraImageSizeY();

  ImageCapture cap(cfg.GetCameraDevice());
  cap.SetVerbosity(1);

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

  cap.SetCaptureProperties(cfg.GetCameraBrightness(), cfg.GetCameraContrast());
  cout << "Brightness:           " << cap.GetBrightness()    << endl;
  cout << "Contrast:             " << cap.GetContrast()    << endl;
  const double thresholdLight = cfg.GetBrightnessThreshold();
  const double thresholdColor = cfg.GetColorThreshold();
  cout << "Brightness threshold: " << thresholdLight    << endl;
  cout << "Color threshold:      " << thresholdColor    << endl;

  Color red(255,0,0);
  Color green(0,255,0);
  Color blue(0,0,255);

  const unsigned int screenSizeX = 1280, screenSizeY = 800;

  double sLastCX = 0.;
  double sLastCY = 0.;
  double lastCX = 0.;
  double lastCY = 0.;
  unsigned int frame = 0;
  ColorImage img;
  while (1) {
    unsigned int size = 0;
    unsigned char* imgptr = cap.CaptureImagePointer(size);

/*    ColorImage img2;
    img2.ReverseCopyFromMemory(imageSX, imageSY, imgptr);
    unsigned int size2 = 0;
    unsigned char* ppmdata = img2.AsPNM(size2);
    char buffer[1024];
    sprintf(buffer, "foo_%u.pnm", frame);
    FILE *fp = fopen(buffer, "wb");
    fwrite(ppmdata, size2, 1, fp);
    fclose(fp);
    //img2.SaveAsJPEG("foo.jpg",100);
*/

    double cx = 0, cy = 0;
    img.PtrFindLaserCentroid(
      imgptr, imageSX, imageSY, true, cx, cy, red, thresholdColor, thresholdLight
    );

//    if ( pow(lastCX-cx,2)+pow(lastCY-cy,2)
  
    frame++;
    cerr << frame;

    if (cx >= 0) {
      double relCX = (0.5*cx+0.3*lastCX+0.2*sLastCX) / imageSX;
      double relCY = (0.5*cy+0.3*lastCY+0.2*sLastCX) / imageSY;
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


