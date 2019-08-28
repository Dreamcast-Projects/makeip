#!/usr/bin/env python

from gimpfu import *

def save_mr(img, drawable, filename, raw_filename):
    print "Hello, world!"

def load_mr(filename, raw_filename):
    print "Hello, world!"

# register(
#         "python_fu_mrexport",
#         "Export the image in MR format so it can be inserted into a IP.BIN",
#         "Export the image in MR format so it can be inserted into a IP.BIN",
#         "BBHoodsta",
#         "BBHoodsta",
#         "2019",
#         "Export As MR",
#         "*",
#         [
#             (PF_IMAGE, "image", "Input image", None),
#             (PF_DRAWABLE, "drawable", "Input drawable", None),
#         ],
#         [],
#         plugin_main,
#         menu = "<Image>/File/Export")

register(
         'file-mr-save', #name
         'Save an MR (.mr) file', #description
         'Save an MR (.mr) file',
         'BBHoodsta', #author
         'BBHoodsta', #copyright
         '2019', #year
         'MR',
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
         'MR',
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
