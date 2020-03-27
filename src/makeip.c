/* IP Creator (makeip)
 * 
 * Copyright (C) 2000, 2001, 2002, 2019, 2020 KallistiOS Team and contributors.
 * All rights reserved.
 * 
 * This code was contributed to KallistiOS by Andress Antonio Barajas 
 * (BBHoodsta). It was originally made by Marcus Comstedt (zeldin). Some
 * portions were made by Andrew Kieschnick (ADK/Napalm). Heavily updated by
 * SiZiOUS. Bootstrap replacement (IP.TMPL) was made by Jacob Alberty (LiENUS).
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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

char *mr_file = NULL;

char *in_file = NULL;
char *out_file = NULL;

char ip_data[INITIAL_PROGRAM_SIZE];

int real_argc = 0;
VECTOR_DECLARE(real_argv);

void
app_finalize(void)
{
  field_finalize();
  free(program_name); 
  VECTOR_FREE(real_argv);
}

void
app_initialize(char *argv0)
{  
  // extract program name from command line	
  set_program_name(argv0);  
  
  // register cleanup function
  if (atexit(app_finalize)) {
    halt("unable to register atexit!\n");	  
  } 
  
  field_initialize();
  
  // initializing IP default data
  memcpy(ip_data, default_ip_data, INITIAL_PROGRAM_SIZE);

  VECTOR_INIT(real_argv);
}

void
usage(void)
{
  printf("IP creator (makeip) v%s\n\n", MAKEIP_VERSION);
  printf("Creates homebrew Sega Dreamcast bootstrap files (i.e. IP.BIN).\n\n");
  printf("Usage:\n");
  printf("\t%s <IP.BIN> [options]\n", program_name);
  printf("\t%s <ip.txt> <IP.BIN> [options]\n\n", program_name);
  printf("Options:\n");
  printf("\t-f                  Force overwrite <IP.BIN> output file if exist\n");
  printf("\t-h                  Usage information (you\'re looking at it)\n");  
  printf("\t-l <mrfilename>     Insert a MR image into the IP.BIN\n"); 
  printf("\t-t <tmplfilename>   Use an external IP.TMPL file (override default)\n");     
  printf("\t-v                  Enable verbose mode\n");    
  printf("\nInitial Program (IP) fields:\n");
  printf("\t-a <areasymbols>    Area sym (J)apan, (U)SA, (E)urope (default: %s)\n", "JUE"); // TODO
  printf("\t-b <bootfilename>   Boot filename (default: %s)\n", field_get_value(BOOT_FILENAME));
  printf("\t-c <companyname>    Company name / SW maker name (default: %s)\n", field_get_value(SW_MAKER_NAME));
  printf("\t-d <releasedate>    Release date (format: YYYYMMDD, default: %s)\n", field_get_value(RELEASE_DATE));
  printf("\t-e <version>        Product version (default: %s)\n", field_get_value(VERSION));
  printf("\t-g <gametitle>      Title of the software (default: %s)\n", field_get_value(GAME_TITLE));  
  printf("\t-i <deviceinfo>     Device info (format: CD-ROMx/y, default: %s)\n", "CD-ROM1/1");  // TODO
  printf("\t-n <productno>      Product number (default: %s)\n", field_get_value(PRODUCT_NO));
  printf("\t-p <peripherals>    Peripherals (default: %s)\n", field_get_value(PERIPHERALS));
}

void
parse_real_args()
{
  real_argc = VECTOR_TOTAL(real_argv);
  
  if (real_argc < 1) {
	halt("too few arguments");  
  }
  
  if (real_argc > 2) {
    halt("too many arguments");
  }
  
  switch(real_argc) {
    case 1:
      out_file = VECTOR_GET(real_argv, char*, 0);
      break;
    case 2:
      in_file = VECTOR_GET(real_argv, char*, 0);
	  out_file = VECTOR_GET(real_argv, char*, 1);
      break;	
  }
}

int
main(int argc, char *argv[])
{
  int c, overwrite = 0;
    
  app_initialize(argv[0]);
      
  if(argc < 2) {
    usage();
    exit(EXIT_FAILURE);
  }
    
  // read the options    
  while ((c = getopt(argc, argv, ":a:b:c:d:e:fg:hi:n:l:p:t:v")) != -1) {
    switch (c) {
      case 'a':
        field_set_value(AREA_SYMBOLS, optarg);	  
        break;
      case 'b':
        field_set_value(BOOT_FILENAME, optarg);
        break;
      case 'c':
        field_set_value(SW_MAKER_NAME, optarg);
        break;
      case 'd':
        field_set_value(RELEASE_DATE, optarg);
        break;
      case 'e':
        field_set_value(VERSION, optarg);
        break;
      case 'f':
        overwrite = 1;	  
        break;	  
      case 'g':
        field_set_value(GAME_TITLE, optarg);
        break;
      case 'h':
        usage();
        exit(EXIT_SUCCESS);
        break;	
      case 'i':
        field_set_value(DEVICE_INFO, optarg);
        break;	
      case 'n':
        field_set_value(PRODUCT_NO, optarg);
        break;
      case 'l':
        mr_file = optarg;
        break;
      case 'p':
        field_set_value(PERIPHERALS, optarg);
        break;
      case 't':
        ip_read(ip_data, optarg);
		break;
	  case 'v':
	    verbose_enable();
		break;
	  case ':':
	    halt("option \"-%c\" requires an argument.\n", optopt);
	    break;
      case '?':
        if (isprint (optopt)) {
          halt("unknown option \"-%c\".\n", optopt);
        } else {
          halt("unknown option character \"\\x%x\".\n", optopt);
		}
      default:
        abort();
    }
  }
    
  // get extra arguments which are not parsed 
  for(; optind < argc; optind++) {
    VECTOR_ADD(real_argv, argv[optind]);
  }
  parse_real_args();
 
  // use an IP.TXT file for input 
  if (in_file != NULL) {    	  
    field_load(in_file);
  }
  
  // stop if an error was detected when setting a field value
  if (field_erroneous()) {
    halt("field(s) incorrect(s); fix and try again");	  
  } 
 
  // write data to the ip data 
  field_write(ip_data);

  // check if the output IP.BIN is writable 
  if (!overwrite && is_file_exist(out_file)) {
    halt("output bootstrap file \"%s\" already exist\n", out_file);
  }  
 
  // writing the file onto disk
  log_notice("writing bootstrap to \"%s\"\n", out_file);
  ip_write(ip_data, mr_file, out_file);
  
  return EXIT_SUCCESS;
}
