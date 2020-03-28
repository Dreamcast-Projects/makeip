# IP creator (makeip) additional resources

**IP creator** (`makeip`) is an utility used for generating homebrew Sega
Dreamcast bootstrap files, also known as IP for Initial Program. This file is
often known as `IP.BIN` files.

This directory contains several useful resources:

* IP bootstrap template
* IP fields template
* Sample MR logos

## IP bootstrap template

Bootstrap templates are the files containing the bootstrap code itself. You may
pass this file to the **IP creator** program by using the `-t` switch.

* `ip.tmpl`: The IP template file provided in the **IP.BIN Replacement**
   package. This is the default IP template embedded in the **IP creator**
   utility.
* `ipalt.tmpl`: The original `IP.TMPL` provided with the original `makeip`
   package. It uses the `AIP` library from Sega, which has been completely
   removed in the **IP.BIN Replacement** package.

## IP fields template

These files may be used to pass arguments to the **IP creator** program instead
of using the command line arguments. To use it, you just have to enter 
`makeip ip.txt IP.BIN`.

* `ip.txt`: The new, modernized IP template file.
* `ipalt.txt`: The original `ip.txt` file provided with the original `makeip` 
   package, which is still supported in this version of **IP creator**.

## Sample MR logos

Some MR images are provided in the `iplogos` directory. They may be used when
making a `IP.BIN` file. Use the `-l` switch to apply a logo in the bootstrap
file.