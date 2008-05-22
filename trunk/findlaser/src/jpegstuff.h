#ifndef __FindLaser__JPEGStuff_H
#define __FindLaser__JPEGStuff_H

#include <stddef.h>
#include <stdio.h>
#include <setjmp.h>
#include <jpeglib.h>
#include <string.h>

namespace FindLaser {
  namespace JPEGStuff {

    struct my_error_mgr {
      struct jpeg_error_mgr pub;	/* "public" fields */
      jmp_buf setjmp_buffer;	/* for return to caller */
    };

    typedef struct my_error_mgr * my_error_ptr;

    int read_JPEG_file (char* filename, unsigned char*& img, int& samples, int& columns, int& rows);

    void write_JPEG_file (
        char * filename, const int quality, const int image_height, const int image_width,
        unsigned char* image, int samples);

  } // end namespace FindLaser::JPEGStuff
} // end namespace FindLaser

#endif
