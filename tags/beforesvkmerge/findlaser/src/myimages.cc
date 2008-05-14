#include "myimages.h"

#include <jpeglib.h>
#include <setjmp.h>
#include <cmath>

#include <string.h>

#include <iostream>
#include <algorithm>

using std::vector;
using std::string;
using std::cout;
using std::cerr;
using std::endl;

namespace FindLaser {

// Skip ahead to XXX for the C++ stuff

/***********************************************
 * JPEG READING
 ***********************************************/

struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}


int read_JPEG_file (char* filename, vector<unsigned char*>& img, int& samples)
{
  /* This struct contains the JPEG decompression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   */
  img.clear();
  samples = 0;

  struct jpeg_decompress_struct cinfo;
  /* We use our private extension JPEG error handler.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct my_error_mgr jerr;
  /* More stuff */
  FILE * infile;		/* source file */
  JSAMPARRAY buffer;		/* Output row buffer */
  //int row_stride;		/* physical row width in output buffer */

  /* In this example we want to open the input file before doing anything else,
   * so that the setjmp() error recovery below can assume the file is open.
   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
   * requires it in order to read binary files.
   */

  if ((infile = fopen(filename, "rb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    return 0;
  }

  /* Step 1: allocate and initialize JPEG decompression object */

  /* We set up the normal JPEG error routines, then override error_exit. */
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer)) {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_stdio_src(&cinfo, infile);

  /* Step 3: read file parameters with jpeg_read_header() */

  (void) jpeg_read_header(&cinfo, TRUE);
  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.doc for more info.
   */

  /* Step 4: set parameters for decompression */

  /* In this example, we don't need to change any of the defaults set by
   * jpeg_read_header(), so we do nothing here.
   */

  /* Step 5: Start decompressor */

  (void) jpeg_start_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */ 
  /* JSAMPLEs per row in output buffer */
  //row_stride = cinfo.output_width * cinfo.output_components;
  samples = cinfo.output_width * cinfo.output_components;
  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, samples, 1);

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */
  while (cinfo.output_scanline < cinfo.output_height) {
    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
    /* Assume put_scanline_someplace wants a pointer and sample count. */
//    put_scanline_someplace(buffer[0], row_stride);
    unsigned char* line = (unsigned char*) malloc(samples * sizeof(unsigned char));
    unsigned char* templine = buffer[0];
    for (unsigned int i = 0; i < samples; i++)
      line[i] = templine[i];

    img.push_back(line);
  }

  /* Step 7: Finish decompression */

  (void) jpeg_finish_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* Step 8: Release JPEG decompression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_decompress(&cinfo);

  /* After finish_decompress, we can close the input file.
   * Here we postpone it until after no more JPEG errors are possible,
   * so as to simplify the setjmp error logic above.  (Actually, I don't
   * think that jpeg_destroy can do an error exit, but why assume anything...)
   */
  fclose(infile);

  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   */

  /* And we're done! */
  return 1;
}


/**********************************************
 * JPEG Writing
 **********************************************/

void write_JPEG_file (
  char * filename, int quality, int image_height, int image_width,
  const vector<unsigned char*>& image, int samples
)
{
  /* This struct contains the JPEG compression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   * It is possible to have several such structures, representing multiple
   * compression/decompression processes, in existence at once.  We refer
   * to any one struct (and its associated working data) as a "JPEG object".
   */
  struct jpeg_compress_struct cinfo;
  /* This struct represents a JPEG error handler.  It is declared separately
   * because applications often want to supply a specialized error handler
   * (see the second half of this file for an example).  But here we just
   * take the easy way out and use the standard error handler, which will
   * print a message on stderr and call exit() if compression fails.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct jpeg_error_mgr jerr;
  /* More stuff */
  FILE * outfile;		/* target file */
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
  int row_stride;		/* physical row width in image buffer */

  /* Step 1: allocate and initialize JPEG compression object */

  /* We have to set up the error handler first, in case the initialization
   * step fails.  (Unlikely, but it could happen if you are out of memory.)
   * This routine fills in the contents of struct jerr, and returns jerr's
   * address which we place into the link field in cinfo.
   */
  cinfo.err = jpeg_std_error(&jerr);
  /* Now we can initialize the JPEG compression object. */
  jpeg_create_compress(&cinfo);

  /* Step 2: specify data destination (eg, a file) */
  /* Note: steps 2 and 3 can be done in either order. */

  /* Here we use the library-supplied code to send compressed data to a
   * stdio stream.  You can also write your own code to do something else.
   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
   * requires it in order to write binary files.
   */
  if ((outfile = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    exit(1);
  }
  jpeg_stdio_dest(&cinfo, outfile);

  /* Step 3: set parameters for compression */

  /* First we supply a description of the input image.
   * Four fields of the cinfo struct must be filled in:
   */
  cinfo.image_width = image_width; 	/* image width and height, in pixels */
  cinfo.image_height = image_height;
  cinfo.input_components = samples;		/* # of color components per pixel */
  if (samples == 1) {
    cinfo.in_color_space = JCS_GRAYSCALE; 	/* colorspace of input image */
  }
  else {
    cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  }
  /* Now use the library's routine to set default compression parameters.
   * (You must set at least cinfo.in_color_space before calling this,
   * since the defaults depend on the source color space.)
   */
  jpeg_set_defaults(&cinfo);
  /* Now you can set any non-default parameters you wish to.
   * Here we just illustrate the use of quality (quantization table) scaling:
   */
  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  /* Step 4: Start compressor */

  /* TRUE ensures that we will write a complete interchange-JPEG file.
   * Pass TRUE unless you are very sure of what you're doing.
   */
  jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  /*           jpeg_write_scanlines(...); */

  /* Here we use the library's state variable cinfo.next_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   * To keep things simple, we pass one scanline per call; you can pass
   * more if you wish, though.
   */
  row_stride = image_width * samples;	/* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) {
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
    row_pointer[0] = image[cinfo.next_scanline];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  /* Step 6: Finish compression */

  jpeg_finish_compress(&cinfo);
  /* After finish_compress, we can close the output file. */
  fclose(outfile);

  /* Step 7: release JPEG compression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_compress(&cinfo);
}





/**********************************************
 * XXX The C++ starts here!
 */

/************************
 * Image class
 */

Image::Image() : fColumns(0) {
}

Image::~Image() {
  ClearImage();
}

void Image::ClearImage () {
  for (unsigned int i = 0; i < fImage.size(); i++)
    free(fImage[i]);
  fImage.clear();
  fColumns = 0;
}

unsigned char* Image::AsPNM(unsigned int& datasize) {
  unsigned char* pnm;
  vector<unsigned char*>& img = GetImage();
  unsigned int cols = GetColumns();
  unsigned int samples = GetSamples();
  unsigned int rows = GetRows();

  char header[100];
  if (samples == 3) {
    sprintf(header, "P6\n%u %u\n255\n", GetColumns(), GetRows());
  }
  else {
    sprintf(header, "P5\n%u %u\n255\n", GetColumns(), GetRows());
  }
  unsigned int headerlength = strlen(header);

  const unsigned int rowsize = cols*samples;
  const unsigned int size = rowsize*rows * sizeof(unsigned char);
  
  pnm = (unsigned char*) malloc( size + headerlength*sizeof(char) );
  strcpy((char*)pnm, header);

  unsigned char* ptr = pnm+headerlength;
  for (unsigned int i = 0; i < rows; i++) {
    unsigned char* row = img[i];
    memcpy(ptr, row, rowsize);
    ptr += rowsize;
  }

  datasize = size + headerlength*sizeof(char);
  return pnm;
}
/************************
 * The ColorImage class
 */

ColorImage::ColorImage() {
}

ColorImage::~ColorImage() {}


bool ColorImage::ReverseCopyFromMemory (unsigned int width, unsigned int height, unsigned char* ptr) {
  ClearImage();
  if (ptr == NULL) return false;
  fColumns = width;
  vector<unsigned char*>& img = GetImage();
  img.resize(height);
  const unsigned int rowsize = width * 3 * sizeof(unsigned char);

  for (unsigned int i = 0; i < height; i++) {
    unsigned char* newline = (unsigned char*) malloc(rowsize);
    //memcpy(newline, ptr, rowsize);
    for (unsigned int j = 0; j < rowsize; j+=3) {
      newline[j]   = ptr[j+2];
      newline[j+1] = ptr[j+1];
      newline[j+2] = ptr[j];
    }
    ptr += rowsize;
    img[i] = newline;
  }

  return true;
}

bool ColorImage::ReadFromFile (const string filename) {
  ClearImage();
  int totalsamples;
  int result = read_JPEG_file((char*)filename.c_str(), fImage, totalsamples);
  if (result == 0) {
    ClearImage();
    return false;
  }
  fColumns = totalsamples / GetSamples();
  return true;
}


GreyImage* ColorImage::ColorProjection(const Color& color) {
  GreyImage* grey = new GreyImage();
  vector<unsigned char*>& greyData = grey->GetImage();
  vector<unsigned char*>& colorData = GetImage();

  double projRed   = color.red / 255.;
  double projGreen = color.green / 255.;
  double projBlue  = color.blue / 255.;

  int cols = GetColumns();
  int rows = colorData.size();
  greyData.resize( rows );
  for (unsigned int r = 0; r < rows; r++) {
    unsigned char* newline = (unsigned char*) malloc(cols * sizeof(unsigned char));
    unsigned char* colorline = colorData[r];
    for (unsigned int c = 0; c < cols; c++) {
      newline[c] = (unsigned char) (   projRed * colorline[c*3+0]
                                     + projGreen * colorline[c*3+1]
                                     + projBlue * colorline[c*3+2]  );
    }
    greyData[r] = newline;
  }
  grey->SetColumns(cols);

  return grey;
}


inline GreyImage* ColorImage::RedProjection()   { return FixedColorProjection(0); }
inline GreyImage* ColorImage::GreenProjection() { return FixedColorProjection(1); }
inline GreyImage* ColorImage::BlueProjection()  { return FixedColorProjection(2); }


GreyImage* ColorImage::FixedColorProjection(unsigned int colorOffset) {
  GreyImage* grey = new GreyImage();
  vector<unsigned char*>& greyData = grey->GetImage();
  vector<unsigned char*>& colorData = GetImage();

  int cols = GetColumns();
  int rows = colorData.size();
  greyData.resize( rows );
  for (unsigned int r = 0; r < rows; r++) {
    unsigned char* newline = (unsigned char*) malloc(cols * sizeof(unsigned char));
    unsigned char* colorline = colorData[r];
    for (unsigned int c = 0; c < cols; c++) {
      newline[c] =  colorline[c*3 + colorOffset];
    }
    greyData[r] = newline;
  }
  grey->SetColumns(cols);

  return grey;
}


GreyImage* ColorImage::NormalizedColorProjection(const Color& color, const unsigned char colorThreshold, const unsigned char colorDefault) {
  GreyImage* grey = new GreyImage();
  vector<unsigned char*>& greyData = grey->GetImage();
  vector<unsigned char*>& colorData = GetImage();

  const double projRed   = color.red / 255.;
  const double projGreen = color.green / 255.;
  const double projBlue  = color.blue / 255.;
  
  const double projLen = pow( projRed*projRed + projGreen*projGreen + projBlue*projBlue, 0.5 );

  const double normProjRed = projRed/projLen;
  const double normProjGreen = projGreen/projLen;
  const double normProjBlue = projBlue/projLen;

  int cols = GetColumns();
  int rows = colorData.size();
  greyData.resize( rows );
  for (unsigned int r = 0; r < rows; r++) {
    unsigned char* newline = (unsigned char*) malloc(cols * sizeof(unsigned char));
    unsigned char* colorline = colorData[r];
    for (unsigned int c = 0; c < cols; c++) {
      const unsigned int r = colorline[c*3+0];
      const unsigned int g = colorline[c*3+1];
      const unsigned int b = colorline[c*3+2];
      
      const double colorLength = pow((double)(r*r+g*g+b*b), 0.5);
      
      newline[c] = (unsigned char) ( 255 * (   normProjRed   * r / colorLength
                                             + normProjGreen * g / colorLength
                                             + normProjBlue  * b / colorLength ) );
      if (newline[c] < colorThreshold)
        newline[c] = colorDefault;
    }
    greyData[r] = newline;
  }
  grey->SetColumns(cols);

  return grey;
}


GreyImage* ColorImage::ToGreyscale(const unsigned char brightnessThreshold, const unsigned char brightnessDefault) {
  GreyImage* grey = new GreyImage();
  vector<unsigned char*>& greyData = grey->GetImage();
  vector<unsigned char*>& colorData = GetImage();

  const int cols = GetColumns();
  const int rows = colorData.size();
  greyData.resize( rows );
  for (unsigned int r = 0; r < rows; r++) {
    unsigned char* newline = (unsigned char*) malloc(cols * sizeof(unsigned char));
    const unsigned char* colorline = colorData[r];
    for (unsigned int c = 0; c < cols; c++) {
      const unsigned int r = colorline[c*3+0];
      const unsigned int g = colorline[c*3+1];
      const unsigned int b = colorline[c*3+2];
      
      newline[c] = (unsigned char) (0.301*(float)r + 0.586*(float)g + 0.113*(float)b);
      if (newline[c] < brightnessThreshold)
        newline[c] = brightnessDefault; 
    }
    greyData[r] = newline;
  }
  grey->SetColumns(cols);

  return grey;
}


void ColorImage::SaveAsJPEG(std::string filename, int quality) {
  write_JPEG_file(
    (char*)filename.c_str(), quality, GetRows(), GetColumns(), GetImage(), GetSamples()
  );
}

void ColorImage::FindLaserCentroid(
  double& centroidx, double& centroidy, const Color& color, 
  const double colorThreshold, const double lightThreshold
) {
  centroidx = -1.;
  centroidy = -1.;

  vector<unsigned char*>& colorData = GetImage();

  const float projRed   = color.red / 255.;
  const float projGreen = color.green / 255.;
  const float projBlue  = color.blue / 255.;
  
  const float projLen = pow( projRed*projRed + projGreen*projGreen + projBlue*projBlue, 0.5 );

  const float normProjRed = projRed/projLen*255;
  const float normProjGreen = projGreen/projLen*255;
  const float normProjBlue = projBlue/projLen*255;

  const unsigned int lightThresholdSquared = (unsigned int) lightThreshold * (unsigned int) lightThreshold;

  int cols = GetColumns();
  int rows = colorData.size();

  float cx = 0, cy = 0;
  float cw = 0;

  for (unsigned int row = 0; row < rows; row++) {
    unsigned char* colorline = colorData[row];
    for (unsigned int c = 0; c < cols; c++) {
      const unsigned int firstIndex = c*3;
      const unsigned int r = colorline[firstIndex+0];
      const unsigned int g = colorline[firstIndex+1];
      const unsigned int b = colorline[firstIndex+2];
      
      const unsigned int colsquares = r*r+g*g+b*b;
//      const float grey = (1./3.)*(float)(r+g+b);
      if (colsquares/3 > lightThresholdSquared) {
        const float colorLength = pow((float)(colsquares), 0.5);
        const float colorProjection =  (   normProjRed   * r / colorLength
                                         + normProjGreen * g / colorLength
                                         + normProjBlue  * b / colorLength );
        if (colorProjection >= colorThreshold) {
          const float w = colorProjection*colorProjection;
          cw += w;
          cx += w*c;
          cy += w*row;
        }
      }
    } // end for cols
  } // end for rows

  if (cw > 0) {
    centroidx = cx / cw;
    centroidy = cy / cw;
  }
}


/************************
 * The GreyImage class
 */

GreyImage::GreyImage() {
}

GreyImage::~GreyImage() {}

void GreyImage::SaveAsJPEG(std::string filename, int quality) {
  write_JPEG_file(
    (char*)filename.c_str(), quality, GetRows(), GetColumns(), GetImage(), GetSamples()
  );
}

GreyImage* GreyImage::CutOnThreshold(unsigned char threshold, unsigned char replacement) {
  
  const vector<unsigned char*>& oldimg = GetImage();
  unsigned int rows = GetRows();
  unsigned int cols = GetColumns();

  GreyImage* grey = new GreyImage();
  vector<unsigned char*>& newimg = grey->GetImage();
  newimg.resize(rows);
  grey->SetColumns(cols);

  for (unsigned int r = 0; r < rows; r++) {
    const unsigned char* row = oldimg[r];
    newimg[r] = (unsigned char*) malloc(cols * sizeof(unsigned char));
    unsigned char* newrow = newimg[r];    

    for (unsigned int c = 0; c < cols; c++) {
      const unsigned char val = row[c];
      if (val < threshold) newrow[c] = replacement;
      else newrow[c] = val;
    }
  }
  return grey;
}

/***********************************
 * The Color class
 ***********************************/
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





} // end namespace

