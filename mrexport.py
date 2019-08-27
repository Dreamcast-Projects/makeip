#!/usr/bin/env python
 
from gimpfu import *
 
def plugin_main(timg, tdrawable):
    print "Hello, world!"
 
register(
        "python_fu_mrexport",
        "Export the image in MR format so it can be inserted into a IP.BIN",
        "Export the image in MR format so it can be inserted into a IP.BIN",
        "BBHoodsta",
        "BBHoodsta",
        "2019",
        "Export As MR",
        "*",
        [
            (PF_IMAGE, "image", "Input image", None),
            (PF_DRAWABLE, "drawable", "Input drawable", None),
        ],
        [],
        plugin_main,
        menu = "<Image>/File/Export")
 
main()