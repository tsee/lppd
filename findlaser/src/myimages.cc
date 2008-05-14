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


int read_JPEG_file (char* filename, unsigned char*& img, int& samples, int& columns, int& rows)
{
  /* This struct contains the JPEG decompression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   */
  if (!(img == NULL)) delete img;

  samples = 0;
  columns = 0;
  rows = 0;

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
  columns = cinfo.output_width;
  samples = cinfo.output_components;
  rows    = cinfo.output_height;

  const int rowsize = samples*columns;
  
  img = (unsigned char*) malloc(cinfo.output_height * samples * columns * sizeof(unsigned char));

  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)
		((j_common_ptr) &cinfo, JPOOL_IMAGE, rowsize, 1);

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  
  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */
  unsigned int lineNo = 0;
  while (cinfo.output_scanline < cinfo.output_height) {
    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
    /* Assume put_scanline_someplace wants a pointer and sample count. */
//    put_scanline_someplace(buffer[0], row_stride);
    unsigned char* templine = buffer[0];
    const unsigned int endOfRow = lineNo*rowsize + rowsize;
    for (unsigned int i = lineNo*rowsize; i < endOfRow; i++)
      img[i] = templine[i];
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
  char * filename, const int quality, const int image_height, const int image_width,
  unsigned char* image, int samples
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
    row_pointer[0] = &image[cinfo.next_scanline];
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

Image::Image() : fColumns(0), fImage(0), fRows(0) {
}

Image::~Image() {
  ClearImage();
}

void Image::ClearImage () {
  unsigned char* img = GetImage();
  if (img != NULL)
    delete img;
  img = NULL;
  SetImage(img);
  SetColumns(0);
  SetRows(0);
}

unsigned char* Image::AsPNM(unsigned int& datasize) {
  unsigned char* pnm;
  const unsigned char* img = GetImage();
  const unsigned int cols = GetColumns();
  const unsigned int samples = GetSamples();
  const unsigned int rows = GetRows();

  char header[100];
  if (samples == 3) {
    sprintf(header, "P6\n%u %u\n255\n", GetColumns(), GetRows());
  }
  else {
    sprintf(header, "P5\n%u %u\n255\n", GetColumns(), GetRows());
  }
  const unsigned int headerlength = strlen(header);

  const unsigned int rowsize = cols*samples;
  const unsigned int size = rowsize*rows * sizeof(unsigned char);
  
  pnm = (unsigned char*) malloc( size + headerlength*sizeof(char) );
  strcpy((char*)pnm, header);

  unsigned char* ptr = pnm+headerlength;
  memcpy(ptr, img, size);

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
  if (ptr == NULL) return false;
  ClearImage();
  SetColumns(width);
  SetRows(height);
  unsigned char* img = GetImage();

  const unsigned int rowsize = width * GetSamples() * sizeof(unsigned char);
  const unsigned int size = height * rowsize;
  img = (unsigned char*) malloc(size);

  cout << width << " " << height << " " << GetSamples() << endl;

  for (unsigned int i = 0; i < size; i+=3) {
    img[i]   = ptr[i+2];
    img[i+1] = ptr[i+1];
    img[i+2] = ptr[i];
  }

  SetImage(img);
  return true;
}

bool ColorImage::ReadFromFile (const string filename) {
  ClearImage();
  int samples, columns, rows;
  unsigned char* img = GetImage();
  int result = read_JPEG_file((char*)filename.c_str(), img, samples, columns, rows);
  if (result == 0) {
    ClearImage();
    return false;
  }
  SetImage(img);
  SetRows(rows);
  SetColumns(columns);
  return true;
}


GreyImage* ColorImage::ColorProjection(const Color& color) {
  GreyImage* grey = new GreyImage();
  unsigned char* greyData = grey->GetImage();
  unsigned char* colorData = GetImage();

  double projRed   = color.red / 255.;
  double projGreen = color.green / 255.;
  double projBlue  = color.blue / 255.;

  int cols = GetColumns();
  int rows = GetRows();
  const unsigned int greySize = cols*rows*1*sizeof(unsigned char);
  const unsigned int colorSize = greySize*GetSamples();

  greyData = (unsigned char*) malloc(greySize);

  for (unsigned int i = 0; i < greySize; i++) {
    const unsigned int ci = i * 3;
    greyData[i] = (unsigned char) (   projRed * colorData[ci+0]
                                     + projGreen * colorData[ci+1]
                                     + projBlue * colorData[ci+2]  );
  }

  grey->SetColumns(cols);
  grey->SetRows(rows);
  grey->SetImage(greyData);

  return grey;
}


inline GreyImage* ColorImage::RedProjection()   { return FixedColorProjection(0); }
inline GreyImage* ColorImage::GreenProjection() { return FixedColorProjection(1); }
inline GreyImage* ColorImage::BlueProjection()  { return FixedColorProjection(2); }


GreyImage* ColorImage::FixedColorProjection(unsigned int colorOffset) {
  GreyImage* grey = new GreyImage();
  unsigned char* greyData = grey->GetImage();
  unsigned char* colorData = GetImage();

  int cols = GetColumns();
  int rows = GetRows();

  const unsigned int greySize = cols*rows*1*sizeof(unsigned char);
  const unsigned int colorSize = greySize*GetSamples();
  greyData = (unsigned char*) malloc(greySize);

  for (unsigned int i = 0; i < greySize; i++) {
    const unsigned int ci = i * 3;
    greyData[i] =  colorData[ci + colorOffset];
  }

  grey->SetColumns(cols);
  grey->SetRows(rows);
  grey->SetImage(greyData);

  return grey;
}


GreyImage* ColorImage::NormalizedColorProjection(const Color& color, const unsigned char colorThreshold, const unsigned char colorDefault) {
  GreyImage* grey = new GreyImage();
  unsigned char* greyData = grey->GetImage();
  unsigned char* colorData = GetImage();

  const double projRed   = color.red / 255.;
  const double projGreen = color.green / 255.;
  const double projBlue  = color.blue / 255.;
  
  const double projLen = pow( projRed*projRed + projGreen*projGreen + projBlue*projBlue, 0.5 );

  const double normProjRed = projRed/projLen;
  const double normProjGreen = projGreen/projLen;
  const double normProjBlue = projBlue/projLen;

  int cols = GetColumns();
  int rows = GetRows();

  const unsigned int greySize = cols*rows*1*sizeof(unsigned char);
  const unsigned int colorSize = greySize*GetSamples();

  greyData = (unsigned char*) malloc(greySize);

  for (unsigned int i = 0; i < greySize; i++) {
    const unsigned int ci = i * 3;
    const unsigned int r = colorData[ci+0];
    const unsigned int g = colorData[ci+1];
    const unsigned int b = colorData[ci+2];

    const double colorLength = pow((double)(r*r+g*g+b*b), 0.5);
      
    greyData[i] = (unsigned char) ( 255 * (  normProjRed   * r / colorLength
                                           + normProjGreen * g / colorLength
                                           + normProjBlue  * b / colorLength ) );
    if (greyData[i] < colorThreshold)
      greyData[i] = colorDefault;
  }

  grey->SetColumns(cols);
  grey->SetRows(rows);
  grey->SetImage(greyData);

  return grey;
}


GreyImage* ColorImage::ToGreyscale(const unsigned char brightnessThreshold, const unsigned char brightnessDefault) {
  GreyImage* grey = new GreyImage();
  unsigned char* greyData = grey->GetImage();
  unsigned char* colorData = GetImage();

  const int cols = GetColumns();
  const int rows = GetRows();

  const unsigned int greySize = cols*rows*1*sizeof(unsigned char);
  const unsigned int colorSize = greySize*GetSamples();

  greyData = (unsigned char*) malloc(greySize);

  for (unsigned int i = 0; i < greySize; i++) {
    const unsigned int ci = i * 3;
    const unsigned int r = colorData[ci+0];
    const unsigned int g = colorData[ci+1];
    const unsigned int b = colorData[ci+2];
    greyData[i] = (unsigned char) (0.301*(float)r + 0.586*(float)g + 0.113*(float)b);
    if (greyData[i] < brightnessThreshold)
      greyData[i] = brightnessDefault; 
  }

  grey->SetColumns(cols);
  grey->SetRows(rows);
  grey->SetImage(greyData);

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

  unsigned char* colorData = GetImage();

  const float projRed   = color.red / 255.;
  const float projGreen = color.green / 255.;
  const float projBlue  = color.blue / 255.;
  
  const float projLen = pow( projRed*projRed + projGreen*projGreen + projBlue*projBlue, 0.5 );

  const float normProjRed = projRed/projLen*255;
  const float normProjGreen = projGreen/projLen*255;
  const float normProjBlue = projBlue/projLen*255;

  const unsigned int lightThresholdSquared = (unsigned int) lightThreshold * (unsigned int) lightThreshold;

  const int cols = GetColumns();
  const int rows = GetRows();

  const unsigned int rowSize = cols*GetSamples();

  float cx = 0, cy = 0;
  float cw = 0;

  for (unsigned int row = 0; row < rows; row++) {
    const unsigned int firstOnRow = row*rowSize;

    for (unsigned int c = 0; c < cols; c++) {
      const unsigned int firstIndex = firstOnRow + c*3;
      const unsigned int r = colorData[firstIndex+0];
      const unsigned int g = colorData[firstIndex+1];
      const unsigned int b = colorData[firstIndex+2];
      
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
  
  const unsigned char* oldimg = GetImage();
  unsigned int rows = GetRows();
  unsigned int cols = GetColumns();

  const unsigned int size = cols*rows*1*sizeof(unsigned char);

  GreyImage* grey = new GreyImage();
  unsigned char* newimg = grey->GetImage();
  newimg = (unsigned char*) malloc(size);

  grey->SetColumns(cols);
  grey->SetRows(rows);

  for (unsigned int i = 0; i < size; i++) {
    const unsigned char val = newimg[i];
    if (val < threshold) newimg[i] = replacement;
    else newimg[i] = val;
  }

  grey->SetImage(newimg);

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

