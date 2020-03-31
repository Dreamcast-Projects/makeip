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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#include "utils.h"
#include "vector.h"

#include "iptmpl.h"
#include "ip.h"

#include "mr.h"
#include "field.h"

// Output IP.BIN filename
char *g_filename_out = NULL;

// Input image file (if any)
char *g_filename_image_in = NULL;

// Output image file (if any)
char *g_filename_image_out = NULL;

// ip.txt file (if any)
char *g_filename_in = NULL;

// data that will be written to the IP.BIN output file
char g_ip_data[INITIAL_PROGRAM_SIZE];

// command-line arguments not parsed by getopt
int g_real_argc = 0;
VECTOR_DECLARE(g_real_argv);

// options handled by makeip
#define OPTIONS "a:b:c:d:e:fg:hi:n:l:p:s:t:uv"
char *g_parameterized_options;

// fields input from command-line
char *g_field_inputs[NUM_FIELDS];

void
app_finalize(void)
{
  field_finalize();
  program_name_finalize();
  VECTOR_FREE(g_real_argv);
  free(g_parameterized_options);
  for(int i = 0; i < NUM_FIELDS; i++) {
    if (g_field_inputs[i] != NULL) {
      free(g_field_inputs[i]);
    }
  }
}

void
app_initialize(char *argv0)
{
  // extract program name from command line
  program_name_initialize(argv0);

  // register cleanup function
  if (atexit(app_finalize)) {
    halt("unable to register atexit!\n");
  }

  // initialize default values for fields
  field_initialize();

  // initializing IP default data
  memcpy(g_ip_data, default_ip_data, INITIAL_PROGRAM_SIZE);

  // initialize the array for real argv values
  VECTOR_INIT(g_real_argv);

  // retrieve parameterized options
  g_parameterized_options = retrieve_parameterized_options(OPTIONS);
}

void
usage(int print_field_information)
{
  printf("IP creator (makeip) v%s\n\n", MAKEIP_VERSION);
  printf("Creates homebrew Sega Dreamcast bootstrap files (i.e. IP.BIN).\n\n");
  printf("Usage:\n");
  printf("\t%s [options] [ip_fields] <IP.BIN>\n", program_name_get());
  printf("\t%s [options] [ip_fields] <ip.txt> <IP.BIN>\n", program_name_get());
  printf("\t%s -l <iplogo_in> -s <iplogo.mr>\n\n", program_name_get());
  if (!print_field_information) {
    printf("Options:\n");
    printf("\t-f                 Force overwrite output file if already exist\n");
    printf("\t-h                 Print usage information (you\'re looking at it)\n");
    printf("\t-l <infilename>    Load/insert an image into bootstrap (%s)\n", mr_get_friendly_supported_format());
    printf("\t-t <tmplfilename>  Use an external IP.TMPL file (override default)\n");
    printf("\t-u                 Print field usage information\n");
    printf("\t-s <outfilename>   Save image from <infilename> to MR format (see \'-l\')\n");
    printf("\t-v                 Enable verbose mode\n");
	printf("\nExamples:\n");
	printf("\t%s -l iplogo.mr ip.txt IP.BIN\n", program_name_get());
	printf("\t%s -g \"MY INCREDIBLE GAME\" -c \"INDIE DEV\" -t IP.TMPL -v -f IP.BIN\n", program_name_get());
	printf("\t%s -l iplogo.png -s iplogo.mr -v -f \n", program_name_get());
  } else {
    printf("IP (Initial Program) fields:\n");
    printf("\t-a <areasymbols>   Area sym (J)apan, (U)SA, (E)urope (default: %s)\n", field_get_pretty_value(AREA_SYMBOLS));
    printf("\t-b <bootfilename>  Boot filename (default: %s)\n", field_get_pretty_value(BOOT_FILENAME));
    printf("\t-c <companyname>   Company name / SW maker name (default: %s)\n", field_get_pretty_value(SW_MAKER_NAME));
    printf("\t-d <releasedate>   Release date (format: YYYYMMDD, default: %s)\n", field_get_pretty_value(RELEASE_DATE));
    printf("\t-e <version>       Product version (default: %s)\n", field_get_pretty_value(VERSION));
    printf("\t-g <gametitle>     Title of the software (default: %s)\n", field_get_pretty_value(GAME_TITLE));
    printf("\t-i <deviceinfo>    Device info (format: CD-ROMx/y, default: %s)\n", field_get_pretty_value(DEVICE_INFO));
    printf("\t-n <productno>     Product number (default: %s)\n", field_get_pretty_value(PRODUCT_NO));
    printf("\t-p <peripherals>   Peripherals (default: %s)\n", field_get_pretty_value(PERIPHERALS));
  }
}

void
parse_real_args(int argc, char *argv[])
{
  for(; optind < argc; optind++) {
    VECTOR_ADD(g_real_argv, argv[optind]);
  }

  g_real_argc = VECTOR_TOTAL(g_real_argv);

  switch(g_real_argc) {
    case 1:
      g_filename_out = VECTOR_GET(g_real_argv, char*, 0);
      break;
    case 2:
      g_filename_in = VECTOR_GET(g_real_argv, char*, 0);
	  g_filename_out = VECTOR_GET(g_real_argv, char*, 1);
      break;
  }
}

void
set_input_value(int index, char *optarg)
{
  g_field_inputs[index] = strdup(optarg);
}

int
main(int argc, char *argv[])
{
  int c, overwrite = 0, export_logo_only = 0;

  app_initialize(argv[0]);

  if(argc < 2) {
    usage(0);
    exit(EXIT_FAILURE);
  }

  // read the options
  opterr = 0; // suppress default getopt error messages
  while ((c = getopt(argc, argv, OPTIONS)) != -1) {
    switch (c) {
      case 'a':
        set_input_value(AREA_SYMBOLS, optarg);
        break;
      case 'b':
        set_input_value(BOOT_FILENAME, optarg);
        break;
      case 'c':
        set_input_value(SW_MAKER_NAME, optarg);
        break;
      case 'd':
        set_input_value(RELEASE_DATE, optarg);
        break;
      case 'e':
        set_input_value(VERSION, optarg);
        break;
      case 'f':
        overwrite = 1;
        break;
      case 'g':
        set_input_value(GAME_TITLE, optarg);
        break;
      case 'h':
        usage(0);
        exit(EXIT_SUCCESS);
        break;
      case 'i':
        set_input_value(DEVICE_INFO, optarg);
        break;
      case 'n':
        set_input_value(PRODUCT_NO, optarg);
        break;
      case 'l':
        g_filename_image_in = optarg;
        break;
      case 'p':
        set_input_value(PERIPHERALS, optarg);
        break;
      case 's':
        g_filename_image_out = optarg;
        break;
      case 't':
        ip_read(g_ip_data, optarg);
        break;
      case 'u':
        usage(1);
        exit(EXIT_SUCCESS);
        break;
      case 'v':
        verbose_enable();
	break;
      case '?':
        if (is_in_char_array(optopt, g_parameterized_options)) {
          halt("option \"-%c\" requires an argument\n", optopt);
        } else if (isprint(optopt)) {
          halt("unknown option \"-%c\"\n", optopt);
        } else {
          halt("unknown option character \"\\x%x\"\n", optopt);
        }
      default:
        abort();
    }
  }  

  // get extra arguments which are not parsed
  parse_real_args(argc, argv);  
  
  // check if we just want to export the logo
  export_logo_only = !g_real_argc && g_filename_image_in != NULL &&
    g_filename_image_out != NULL;
  
  // we don't know how to deal with that  
  if (g_real_argc > 2) {
    halt("too many arguments\n");
  }
  
  // no arguments was passed... but if we just want to export the logo, it's ok  
  if (g_real_argc < 1 && !export_logo_only) {    
    halt("too few arguments\n");
  }
  
  if (!export_logo_only) {
  
    // assign field values from the ip template file
    // use an 'IP.TXT' file for input
    if (g_filename_in != NULL) {
      field_load(g_filename_in);
    }

    // assign field values from the command-line options
    for (int i = 0; i < NUM_FIELDS; i++) {
      if (g_field_inputs[i] != NULL) {
        field_set_value(i, g_field_inputs[i]);
      }
    }

    // stop if an error was detected when setting a field value
    if (field_erroneous()) {
      halt("field error; fix incorrect value(s) and try again\n");
    }

    // write data to the ip data
    field_write(g_ip_data);

    // check if the output IP.BIN is writable
    if (!overwrite && is_file_exist(g_filename_out)) {
      halt("output bootstrap file \"%s\" already exist\n", g_filename_out);
    }

    // writing the file onto disk
    log_notice("writing bootstrap to \"%s\"\n", g_filename_out);
    ip_write(g_ip_data, g_filename_out, g_filename_image_in, g_filename_image_out);

    log_notice("bootstrap successfully written to \"%s\"\n", g_filename_out);
	
  } else {
    log_notice("entering in MR image conversion only mode\n");
	
    // check if the output MR logo is writable
    if (!overwrite && is_file_exist(g_filename_image_out)) {
      halt("output MR file \"%s\" already exist\n", g_filename_image_out);
    }
	
    // convert the input image to output MR file	
	mr_export(g_filename_image_in, g_filename_image_out);
  }

  return EXIT_SUCCESS;
}
