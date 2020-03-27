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
 
#include "utils.h"

int g_verbose = 0;

void
trim(char *str)
{
  int l = strlen(str);
  while(l > 0 && 
       (str[l-1] == '\r' || str[l-1] == '\n' ||
		    str[l-1] == ' ' || str[l-1] == '\t')) {
    str[--l]='\0';
  }
}

void
set_program_name(char *argv0)
{
  char *result = basename(argv0);
  char *buf = strrchr(result, '.');
  if (buf != NULL) {
    int offset = buf - result;
    result[offset] = '\0';
  }
  program_name = strdup(result);
}

void
log_notice(const char *format, ...)
{
  if (g_verbose) {
    va_list args;
  
    fprintf(stdout, "%s: ", program_name);
  
    va_start(args, format);  
    vfprintf(stdout, format, args);
    va_end(args);
  }
}

void
log_warn(const char *format, ...)
{
  va_list args;
  
  fprintf(stdout, "%s: warning: ", program_name);
  
  va_start(args, format);  
  vfprintf(stdout, format, args);
  va_end(args);
}

void
log_error(const char *format, ...)
{
  va_list args;
  
  fprintf(stderr, "%s: error: ", program_name);
  
  va_start(args, format);  
  vfprintf(stderr, format, args);
  va_end(args);
}

void
halt(const char *format, ...)
{
  va_list args;
  
  fprintf(stderr, "%s: fatal: ", program_name);
  
  va_start(args, format);  
  vfprintf(stderr, format, args);
  va_end(args);
  
  exit(EXIT_FAILURE);
}

void
verbose_enable()
{
  g_verbose = 1;	
}

int
long_parse(char *str, long *result)
{	
  char *dummy; 
  *result = strtol(str, &dummy, 10);
  return (dummy == NULL) || (dummy != NULL && !strlen(dummy));  
}

int
substr_long_parse(char *str, int start, int length, long *result)
{
  int success = 0;  
  char *buf = (char *) malloc((length + 1) * sizeof(char)); 
  
  strncpy(buf, str + start, length);
  buf[length] = '\0';  
  success = long_parse(buf, result);
  
  free(buf);
 
  return success;  
}

int
is_valid_hex(char *str)
{
  int result = 1;	
  for(int i = 0; i < strlen(str); i++) {
    if (!isxdigit(str[i])) {
      result = 0;
	  break;
    }
  }
  return result;  
}

// Date format should be YYYYMMDD
// Extracted and adapted from: https://aticleworld.com/check-valid-date/
int
is_valid_date(char *str)
{
  long y, m, d;
  
  if (!substr_long_parse(str, 0, 4, &y) || 
    !substr_long_parse(str, 4, 2, &m) || !substr_long_parse(str, 6, 2, &d))
    return 0;
    
  // Check if year is leap  
  int is_leap_year = (((y % 4 == 0) && (y % 100 != 0)) || (y % 400 == 0));

  // Check range of year, month and day
  if (y > MAX_YR || y < MIN_YR)
    return 0;
  if (m < 1 || m > 12)
    return 0;
  if (d < 1 || d > 31)
    return 0;

  // Handle feb days in leap year
  if (m == 2) {
    if (is_leap_year)
      return (d <= 29);
    else
      return (d <= 28);
  }

  // Handle months which has only 30 days
  if (m == 4 || m == 6 || m == 9 || m == 11)
    return (d <= 30);

  return 1;
}

int
is_strict_bool(char c) {
  return c == '1' || c == '0';	
}

int
is_file_exist(char *filename)
{
  struct stat stats;
  return (stat(filename, &stats) == 0);
}
