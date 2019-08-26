#!/usr/bin/python
 
from gimpfu import *
 
def plugin_main(img, drawable):
    print "Hello, world!"
 
register(
        "python_fu_mr_export",
        "Export the image in MR format to insert into IP.BIN",
        "Export the image in MR format to insert into IP.BIN",
        "BBHoodsta",
        "BBHoodsta",
        "2019",
        "Export As MR",
        "RGB*, GRAY*",
        [
            (PF_IMAGE, "image", "Input image", None),
            (PF_DRAWABLE, "drawable", "Input drawable", None),
        ],
        [],
        plugin_main,
        menu = "<Image>/File/Save/")
 
main()