/* IP creator (makeip)
 *
 * Copyright (C) 2000, 2001, 2002, 2019, 2020 The KOS Team and contributors.
 * All rights reserved.
 *
 * This code was contributed to KallistiOS (KOS) by Andress Antonio Barajas
 * (BBHoodsta). It was originally made by Marcus Comstedt (zeldin). Some
 * portions of code were made by Andrew Kieschnick (ADK/Napalm). Heavily
 * updated by SiZiOUS. Bootstrap replacement (IP.TMPL) was made by Jacob
 * Alberty (LiENUS).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "mr.h"

typedef struct image_t {
  unsigned int size;
  unsigned int width;
  unsigned int height;
  unsigned char *data;
} image_t;

typedef struct color_t {
  unsigned char r;
  unsigned char g;
  unsigned char b;
} color_t;

typedef struct palette_t {
  color_t color[128];
  int count;
} palette_t;

typedef struct mr_t {
  unsigned int size;
  unsigned int offset;
  unsigned int width;
  unsigned int height;
  unsigned int colors;
  char *data;
} mr_t;

typedef struct mr_output_t {
  unsigned int size;
  unsigned char *data;
} mr_output_t;

int
mr_compress(char *in, char *out, int size)
{
  int length = 0;
  int position = 0;
  int run;

  while (position < size) {
    run = 1;

    while((position+run < size) && (in[position] == in[position+run]) && (run < 0x17f)) {
      run++;
    }

    if (run > 0xff) {
      out[length++] = 0x82;
      out[length++] = 0x80 | (run - 0x100);
      out[length++] = in[position];
    } else if (run > 0x7f) {
      out[length++] = 0x81;
      out[length++] = run;
      out[length++] = in[position];
    } else if (run > 1) {
      out[length++] = 0x80 | run;
      out[length++] = in[position];
    } else {
      out[length++] = in[position];
    }

    position += run;

  }

  return length;
}

int
mr_convert_raw(image_t *image, mr_output_t *output)
{
  palette_t palette;
  mr_t mr;
  int i;
  int *data;
  char *raw_output;
  char *compressed_output;
  int crap = 0;
  int compressed_size;

  palette.count = 0;

  data = (int *)image->data;

  raw_output = (char *)malloc(image->width * image->height);
  compressed_output = (char *)malloc(image->width * image->height);

  for(i = 0; i < image->width * image->height; i++) {
    int found = 0;
    int c = 0;

    while ((!found) && (c < palette.count)) {
      if (!memcmp(&data[i], &palette.color[c], 3)) {
        found = 1;
      } else {
        c++;
      }
   }

   if ((!found) && (c == 128)) {
     log_error("Reduce the number of colors to <= 128 and try again\n");
       return 0;
     }

     if (!found) {
       memcpy(&palette.color[c], &data[i], 3);
       palette.count++;
     }

     raw_output[i] = c;
   }

   log_notice("found %d colors\n", palette.count);

   mr.width = image->width;
   mr.height = image->height;
   mr.colors = palette.count;

   compressed_size = mr_compress(raw_output, compressed_output, image->width * image->height);

   log_notice("compressed %d bytes to %d bytes\n", image->width * image->height, compressed_size);

   mr.offset = 2 + 7 * 4 + palette.count * 4;
   mr.size = 2 + 7 * 4 + palette.count * 4 + compressed_size;

   size_t p = 0;

   output->size = mr.size;
   output->data = (unsigned char *) malloc(mr.size);

   bwrite(&p, output->data, "MR", 2);
   bwrite(&p, output->data, &mr.size, 4);
   bwrite(&p, output->data, &crap, 4);
   bwrite(&p, output->data, &mr.offset, 4);
   bwrite(&p, output->data, &mr.width, 4);
   bwrite(&p, output->data, &mr.height, 4);
   bwrite(&p, output->data, &crap, 4);
   bwrite(&p, output->data, &mr.colors, 4);

   for(i = 0; i < palette.count; i++) {
     bwrite(&p, output->data, &palette.color[i].b, 1);
     bwrite(&p, output->data, &palette.color[i].g, 1);
     bwrite(&p, output->data, &palette.color[i].r, 1);
     bwrite(&p, output->data, &crap, 1);
  }

  bwrite(&p, output->data, compressed_output, compressed_size);

  free(raw_output);
  free(compressed_output);

  return 1;
}

int
mr_read(char *file_name, mr_output_t *output)
{
  FILE *mr = fopen(file_name, "rb");
  if (mr == NULL) {
    log_error("can't open MR file \"%s\"\n", file_name);
    return 0;
  }

  fseek(mr, 0, SEEK_END);
  output->size = ftell(mr);
  fseek(mr, 0, SEEK_SET);

  int result = 1;

  if (!output->size) {
    log_error("MR file is empty\n");
    result = 0;
  } else {
    log_notice("loading MR file (\"%s\", %d bytes)\n", file_name, output->size);

    output->data = (unsigned char *) malloc (output->size);
	if (!fread(output->data, output->size, 1, mr)) {
      log_error("unable to read MR file\n");
      result = 0;
    }
  }

  fclose(mr);

  return result;
}

int
png_read(char *file_name, mr_output_t *output)
{
   image_t pngimg;
   png_structp png_ptr;
   png_infop info_ptr;
   unsigned int sig_read = 0;
   png_uint_32 width, height, row;
   int bit_depth, color_type, interlace_type;
   FILE *fp;
   png_color_16 *image_background;

   if ((fp = fopen(file_name, "rb")) == NULL)
     return 0;

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
      NULL, NULL, NULL);

   if (png_ptr == NULL) {
     fclose(fp);
     return 0;
   }

   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL) {
     fclose(fp);
     png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
     return 0;
   }

   if (setjmp(png_jmpbuf(png_ptr))) {
     png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
     fclose(fp);
     return 0;
   }

   png_init_io(png_ptr, fp);

   png_set_sig_bytes(png_ptr, sig_read);

   png_read_info(png_ptr, info_ptr);

   png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
     &interlace_type, NULL, NULL);

   pngimg.width = width;
   pngimg.height = height;

   pngimg.data = (unsigned char *) malloc(width*height*4);

   // Tell libpng to strip 16 bit/color files down to 8 bits/color
   png_set_strip_16(png_ptr);

   // Extract multiple pixels with bit depths of 1, 2, and 4 from a single byte
   // into separate bytes (useful for paletted and grayscale images).
   png_set_packing(png_ptr);

   // Expand paletted colors into true RGB triplets
   if (color_type == PNG_COLOR_TYPE_PALETTE)
      png_set_expand(png_ptr);

   // Expand grayscale images to the full 8 bits from 1, 2, or 4 bits/pixel
   if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
      png_set_expand(png_ptr);

   // Expand paletted or RGB images with transparency to full alpha channels so
   // the data will be available as RGBA quartets.
   if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
      png_set_expand(png_ptr);

   if (png_get_bKGD(png_ptr, info_ptr, &image_background))
      png_set_background(png_ptr, image_background,
                         PNG_BACKGROUND_GAMMA_FILE, 1, 1.0);

   // Add filler (or alpha) byte (before/after each RGB triplet)
   png_set_filler(png_ptr, 0xff, PNG_FILLER_AFTER);

   png_read_update_info(png_ptr, info_ptr);

   {
     png_bytep row_pointers[height];

     for (row = 0; row < height; row++)
	   row_pointers[row] = pngimg.data + pngimg.width * 4 * row;

     png_read_image(png_ptr, row_pointers);
   }

   png_read_end(png_ptr, info_ptr);

   png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

   fclose(fp);

   mr_convert_raw(&pngimg, output);

   free(pngimg.data);

   return 1;
}

int
mr_inject(char *ip, char *filename)
{
  file_type_t ftype = detect_file_type(filename);

  int r = 0;

  mr_output_t output;
  memset(&output, 0, sizeof(output));

  switch(ftype) {
    case MR:
	  r = mr_read(filename, &output);
      break;
    case PNG:
	  r = png_read(filename, &output);
      break;
    case UNSUPPORTED:
      log_error("unsupported file format\n");
      return 0;
      break;
    case INVALID:
      return 0;
  }

  if (r) {
    log_notice("loaded %s, %d\n", filename, output.size);
  } else {
    log_error("unable to read %s\n", filename);
    return 0;
  }

  if (output.size < 1) {
    log_error("empty file\n");
    return 0;
  }

  if (output.size > 8192) {
    log_warn("MR data is larger than 8192 bytes and may corrupt bootstrap\n");
  }

  memcpy(ip + 0x3820, output.data, output.size);

  free(output.data);

  return 1;
}
