# IP creator (`makeip`) additional resources

**IP creator** (`makeip`) is an utility used for generating homebrew Sega
Dreamcast bootstrap files, also known as IP for Initial Program. This file is
often known as `IP.BIN` files.

This directory contains several useful resources:

* IP bootstrap templates (`templates` directory)
* IP fields templates (`inputs` directory)
* Sample MR logos (`iplogos` directory)

## IP bootstrap templates (`templates` directory)

Bootstrap templates files contains the bootstrap code itself.

Since **IP creator 2+** , the IP bootstrap data is embedded in the program but
you may change the bootstrap by passing the `-t` switch to **IP creator**.

* `ip.tmpl`: This file is the IP template file provided in the
   **IP.BIN Replacement** package by **LiENUS**. This is the default IP template
   embedded in the **IP creator** utility.
* `ipalt.tmpl`: The original `IP.TMPL` provided with the original `makeip`
   package. It uses the `AIP` library from Sega, which has been completely
   removed in the **IP.BIN Replacement** version.

## IP fields templates (`inputs` directory)

These files may be used to pass arguments to the **IP creator** program instead
of using the command line arguments. To use it, you just have to enter 
`makeip ip.txt IP.BIN`.

* `ip.txt`: The new, modernized IP template file.
* `ipalt.txt`: The original `ip.txt` file provided with the original `makeip` 
   package, which is still supported.

## Sample MR logos (`iplogos` directory)

Some **MR** images are provided in the `iplogos` directory. They may be used
when making a `IP.BIN` file. Use the `-l` switch to apply a logo in the
bootstrap file.
