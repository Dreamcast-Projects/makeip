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

import os, sys, struct

from gimpfu import *
from array import array

def to_bytes(n, length, endianess='big'):
    if sys.version_info[0] < 3:
        h = '%x' % n
        s = ('0'*(len(h) % 2) + h).zfill(length*2).decode('hex')
        return s if endianess == 'big' else s[::-1]
    else:
        return n.to_bytes(length, byteorder=endianess)

def mrcompress(input, output, size):
    length = 0
    position = 0
    run = 0
    
    while (position < size):
        run = 1

        while((run < 0x17f) and (position+run < size) and (input[position] == input[position+run])):
            run += 1

        if(run > 0xff):
            output[length] = 0x82
            length += 1
            output[length] = 0x80 | (run - 0x100)
            length += 1
            output[length] = input[position]
            length += 1
        elif(run > 0x7f):
            output[length] = 0x81
            length += 1
            output[length] = run
            length += 1
            output[length] = input[position]
            length += 1
        elif(run > 1):
            output[length] = 0x80 | run
            length += 1
            output[length] = input[position]
            length += 1
        else:
            output[length] = input[position]
            length += 1
        
        position += run

    return length
        
def save_mr(img, drawable, filename, raw_filename):
    src_width = img.width   # image width
    src_height = img.height # image height

    # Display error if image is bigger than 320x90
    if src_width > 320 or src_height > 90:
        gimp.message("Your image should be 320x90 or smaller\n")
        return

    # Grab region of pixels
    src_rgn = drawable.get_pixel_rgn(0, 0, src_width, src_height, False, False)

    # Create a pixel array from the region
    src_pixels = array("B", src_rgn[0:src_width, 0:src_height])

    # Allocate raw and compressed outputs
    raw_output = array("B", "\x00" * (src_width * src_height )) 
    compressed_output = array("B", "\x00" * (src_width * src_height))

    psize = len(src_rgn[0,0])
    palette_count = 0
    palette_colors = array("B", "\x00" * (128 * 3)) # 128 colors (RGB)

    # Generate raw_output and create the palette of the image
    for i in range(src_width*src_height):
        found = False
        palette_index = 0

        pixel_index = i * psize
        while(not found and palette_index < palette_count):
            if(src_pixels[pixel_index:pixel_index+psize] == palette_colors[palette_index*3:palette_index*3+psize]):
                found = True
            else:
                palette_index += 1

        # Display error message if image has more than 128 colors
        if(not found and palette_index == 128):
            gimp.message("Reduce the number of colors to <= 128 and try again.\n")
            return
        
        if(not found):
            palette_colors[palette_index*3:palette_index*3+psize] = src_pixels[pixel_index:pixel_index+psize]
            palette_count += 1
        
        raw_output[i] = palette_index

    compressed_size = mrcompress(raw_output, compressed_output, src_width*src_height)

    # Display warning if compressed image is bigger than 8192 bytes
    if compressed_size > 8192:
        gimp.message("This will NOT fit in a normal ip.bin - it is %d bytes too big!\n", compressed_size - 8192)
    
    crap = 0
    endianness = 'litte'
    offset = 30 + palette_count*4 # 30 byte header
    size = offset + compressed_size

    with open(filename, 'wb') as output:
        output.write("MR")                                    # 
        output.write(to_bytes(size, 4, endianness))           # Filesize
        output.write(to_bytes(crap, 4, endianness))           # 
        output.write(to_bytes(offset, 4, endianness))         # Data offset
        output.write(to_bytes(src_width, 4, endianness))      # Image width
        output.write(to_bytes(src_height, 4, endianness))     # Image height
        output.write(to_bytes(crap, 4, endianness))           # 
        output.write(to_bytes(palette_count, 4, endianness))  # Amount of colors in palette

        for i in range(palette_count):
            palette_color = palette_colors[i*3:i*3+3]
            # Write RGB values in reverse => BGR
            for x in reversed(range(3)):
                output.write(to_bytes(palette_color[x], 1, endianness))
            output.write(to_bytes(crap, 1, endianness)) # Unused alpha

        for i in range(compressed_size):
            output.write(to_bytes(compressed_output[i], 1, endianness))

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
