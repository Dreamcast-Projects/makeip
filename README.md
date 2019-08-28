# makeip

Tool to create IP.BIN files that are used to make self bootable CD-Rs for the Sega Dreamcast.

makeip code originally from Marcus Comstedt (http://mc.pp.se/dc/sw.html) but enhanced to take command line arguments instead.
Also makeip is not longer dependent on IP.TMPL.  Instead we include this inside the binary.

Added the ability to insert .mr images into the IP.BIN.  Code taken from logotools (logoinsert.c) (http://napalm-x.thegypsy.com/andrewk/dc/)

