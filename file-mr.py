#!/usr/bin/env python

# GIMP Plug-in for the Sega Dreamcast MR file format
# Copyright (C) 2019 by BBHodsta
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Based on logotools(pngtomr.c) source code by Andrew Kieschnick
# http://napalm-x.thegypsy.com/andrewk/dc/

from gimpfu import *

def save_mr(img, drawable, filename, raw_filename):
    src_width = drawable.width # image width
    src_height = drawable.height # image height

    # Display error if image is bigger than 320x90

    # Grab region of pixels
    src_rgn = drawable.get_pixel_rgn(0, 0, src_width, src_height, False, False)

    # Create a pixel array from the region
    src_pixels = array("B", src_rgn[0:src_width, 0:src_height])

    # Allocate raw and compressed outputs
    #raw_output = (char *)malloc(image.width * image.height);
    #compressed_output = (char *)malloc(image.width * image.height);

    # Generate raw_output and count the palette of the image

    # Display error message if image has more than 128 colors

    # Compress image (raw_output => compressed_output)

    # Display warning if compressed image is bigger than 8192 bytes

    # crap = 0;
    # mr.offset = 2 + 7*4 + palette.count*4;
    # mr.size = 2 + 7*4 + palette.count*4 + compressed_size;

    # fwrite("MR", 1, 2, output);
    # fwrite(&mr.size, 1, 4, output);
    # fwrite(&crap, 1, 4, output);
    # fwrite(&mr.offset, 1, 4, output);
    # fwrite(&mr.width, 1, 4, output);
    # fwrite(&mr.height, 1, 4, output);
    # fwrite(&crap, 1, 4, output);
    # fwrite(&mr.colors, 1, 4, output);

    # for(i=0; i<palette.count; i++) {
  # fwrite(&palette.color[i].b, 1, 1, output);
  # fwrite(&palette.color[i].g, 1, 1, output);
  # fwrite(&palette.color[i].r, 1, 1, output);
  # fwrite(&crap, 1, 1, output);
    # }
    # fwrite(compressed_output, compressed_size, 1, output);

    print "Hello, world!"

def load_mr(filename, raw_filename):
    print "Hello, world!"

def thumbnail_mr(filename, thumb_size):
    # FIXME: Untested. Does not seem to be used at all? should be run
    # when registered and there is no thumbnail in cache

    print "Hello, world!"

def register_load_handlers():
    gimp.register_load_handler('file-mr-load', 'mr', '')
    pdb['gimp-register-file-handler-mime']('file-mr-load', 'image/mr')
    pdb['gimp-register-thumbnail-loader']('file-mr-load', 'file-mr-load-thumb')

def register_save_handlers():
    gimp.register_save_handler('file-mr-save', 'mr', '')

register(
    'file-mr-load-thumb', #name
    'Load an MR (.mr) file', #description
    'Load an MR (.mr) file',
    'BBHoodsta', #author
    'BBHoodsta', #copyright
    '2019', #year
    None,
    None, #image type
    [   #input args. Format (type, name, description, default [, extra])
        (PF_STRING, 'filename', 'The name of the file to load', None),
        (PF_INT, 'thumb-size', 'Preferred thumbnail size', None),
    ],
    [   #results. Format (type, name, description)
        (PF_IMAGE, 'image', 'Thumbnail image'),
        (PF_INT, 'image-width', 'Width of full-sized image'),
        (PF_INT, 'image-height', 'Height of full-sized image')
    ],
    thumbnail_mr, #callback
)

register(
    'file-mr-save', #name
    'Save an MR (.mr) file', #description
    'Save an MR (.mr) file',
    'BBHoodsta', #author
    'BBHoodsta', #copyright
    '2019', #year
    'Sega Dreamcast MR image',
    '*',
    [   #input args. Format (type, name, description, default [, extra])
    (PF_IMAGE, "image", "Input image", None),
    (PF_DRAWABLE, "drawable", "Input drawable", None),
    (PF_STRING, "filename", "The name of the file", None),
    (PF_STRING, "raw-filename", "The name of the file", None),
    ],
    [], #results. Format (type, name, description)
    save_mr, #callback
    on_query = register_save_handlers,
    menu = '<Save>'
    )

register(
    'file-mr-load', #name
    'Load an MR (.mr) file', #description
    'Load an MR (.mr) file',
    'BBHoodsta', #author
    'BBHoodsta', #copyright
    '2019', #year
    'Sega Dreamcast MR image',
    None, #image type
    [   #input args. Format (type, name, description, default [, extra])
    (PF_STRING, 'filename', 'The name of the file to load', None),
    (PF_STRING, 'raw-filename', 'The name entered', None),
    ],
    [(PF_IMAGE, 'image', 'Output image')], #results. Format (type, name, description)
    load_mr, #callback
    on_query = register_load_handlers,
    menu = "<Load>",
    )

main()
