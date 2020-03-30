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

#include "utils.h"

int g_verbose = 0;

char *g_program_name;

// Thanks to alk
// See: https://stackoverflow.com/a/30141322
void
ltrim(char *str)
{
  char *s_tmp = str;
  size_t len = strlen(str);
  while (isblank(*s_tmp)) {
    ++s_tmp;
  }
  len = len - (str - s_tmp);
  memmove(str, s_tmp, len);
  str[len] = '\0';
}

void
rtrim(char *str)
{
  int l = strlen(str);
  while(l > 0 &&
       (str[l-1] == '\r' || str[l-1] == '\n' ||
        str[l-1] == ' '  || str[l-1] == '\t')) {
    str[--l] = '\0';
  }
}

void
trim(char *str)
{
  rtrim(str);
  ltrim(str);
}

void
program_name_initialize(char *argv0)
{
  char *result = basename(argv0);
  char *buf = strrchr(result, '.');
  if (buf != NULL) {
    int offset = buf - result;
    result[offset] = '\0';
  }
  g_program_name = strdup(result);
}

char *
program_name_get()
{
  return g_program_name;
}

void
program_name_finalize()
{
  free(g_program_name);	
}

void
log_notice(const char *format, ...)
{
  if (g_verbose) {
    va_list args;
  
    fprintf(stdout, "%s: ", g_program_name);
  
    va_start(args, format);  
    vfprintf(stdout, format, args);
    va_end(args);
  }
}

void
log_warn(const char *format, ...)
{
  va_list args;
  
  fprintf(stdout, "%s: warning: ", g_program_name);
  
  va_start(args, format);  
  vfprintf(stdout, format, args);
  va_end(args);
}

void
log_error(const char *format, ...)
{
  va_list args;
  
  fprintf(stderr, "%s: error: ", g_program_name);
  
  va_start(args, format);  
  vfprintf(stderr, format, args);
  va_end(args);
}

void
halt(const char *format, ...)
{
  va_list args;
  
  fprintf(stderr, "%s: fatal: ", g_program_name);
  
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

int
is_in_char_array(char needle, char *haystack)
{
  for(int i = 0; i < strlen(haystack); i++)
  {
    if(haystack[i] == needle)
      return 1;
  }
  return 0;  
}

char *
retrieve_parameterized_options(char *opts)
{
  char c = '\0';
  int j = 0;

  char *buf = (char *) malloc(strlen(opts) * sizeof(char));
  memset(buf, '\0', strlen(opts));
  
  for(int i = 0; i < strlen(opts); i++) {
    if (opts[i]	== ':' && c != '\0') {
      buf[j++] = c;
      c = '\0';	  
	}
	c = opts[i];
  }
  
  return buf; // must be destroyed later
}

file_type_t
detect_file_type(char *filename)
{
  char data[4];
  
  memset(data, 0x0, sizeof(data));

  FILE *f = fopen(filename, "rb");
  if (f == NULL) {
    log_error("can't open file \"%s\"\n", filename);
    return INVALID;
  }
  
  int result = 1;
  if (!fread(data, sizeof(data), 1, f)) {
    log_error("unable to read file\n");
    result = 0;
  }
  
  fclose(f);
  
  if (result) {
    if (!memcmp(data, "MR", 2)) {
      return MR;	  
    } else if (!memcmp(data + 1, "PNG", 3)) {
	  return PNG;
	}
  }
  
  return UNSUPPORTED;
}

void
bwrite(size_t *pos, void *dest, const void *source, size_t num)
{
  memcpy(dest + *pos, source, num);
  *pos = *pos + num;
}
