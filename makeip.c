#include "iptmpl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#define NUM_FIELDS 11

enum InputTypes {
  HardwareID = 0, 
  MakerID,
  DeviceInfo,
  AreaSymbols,
  Peripherals,
  ProductNo,
  Version,
  ReleaseDate,
  BootFilename,
  SWMakerName,
  GameTitle
}; 

static char *inputs[NUM_FIELDS] = {
  "SEGA SEGAKATANA",
  "SEGA ENTERPRISES",
  "0000 CD-ROM1/1",
  "JUE",
  "E000F10",
  "T0000",
  "V1.000",
  "20000627",
  "1ST_READ.BIN",
  "COMPANY",
  "GAMETITLE"
};

struct field;

int check_areasym(char *, struct field *);

struct field {
  char *name;
  int pos;
  int len;
  int (*extra_check)(char *, struct field *);
} fields[NUM_FIELDS] = {
  { "Hardware ID", 0x0, 0x10, NULL },
  { "Maker ID", 0x10, 0x10, NULL },
  { "Device Info", 0x20, 0x10, NULL },
  { "Area Symbols", 0x30, 0x8, check_areasym },
  { "Peripherals", 0x38, 0x8, NULL },
  { "Product No", 0x40, 0xa, NULL },
  { "Version", 0x4a, 0x6, NULL },
  { "Release Date", 0x50, 0x10, NULL },
  { "Boot Filename", 0x60, 0x10, NULL },
  { "SW Maker Name", 0x70, 0x10, NULL },
  { "Game Title", 0x80, 0x80, NULL },
};

int filled_in[NUM_FIELDS];

int check_areasym(char *ptr, struct field *f)
{
  int i, a = 0;
  for(i=0; i<f->len; i++) {
    switch(ptr[i]) {
     case 'J':
       a |= (1<<0);
       break;
     case 'U':
       a |= (1<<1);
       break;
     case 'E':
       a |= (1<<2);
       break;
     case ' ':
       break;
     default:
       fprintf(stderr, "Unknown area symbol '%c'.\n", ptr[i]);
       return 0;
    }
  }

  for(i=0; i<f->len; i++) {
    if((a & (1<<i)) == 0)
      ptr[i] = ' ';
    else
      ptr[i] = "JUE"[i];
  }

  return 1;
}

void trim(char *str)
{
  int l = strlen(str);
  while(l > 0 && 
       (str[l-1] == '\r' || str[l-1] == '\n' ||
		    str[l-1] == ' ' || str[l-1] == '\t')) {
    str[--l]='\0';
  }
}

int insert_mr(char *ip, char *mrfn)
{
  int mr_size;
  char *mr_data;
  FILE *mr = fopen(mrfn, "rb");

  if (mr == NULL) {
    fprintf(stderr, "Can't open mr file \"%s\".\n", mrfn);
    return 1;
  }

  fseek(mr, 0, SEEK_END);
  mr_size = ftell(mr);
  fseek(mr, 0, SEEK_SET);

  if (mr_size > 8192)
    fprintf(stderr, "Warning: this image is larger than 8192 bytes and will corrupt a normal ip.bin, inserting anyway!\n");

  mr_data = (char *)malloc(mr_size);
  fread(mr_data, mr_size, 1, mr);

  memcpy(ip+0x3820, mr_data, mr_size);
  free(mr_data);

  return 0;
}

int parse_input(FILE *fh, char *ip)
{
  static char buf[80];
  int i;

  memset(filled_in, 0, sizeof(filled_in));

  while(fgets(buf, sizeof(buf), fh)) {
    char *p;
    trim(buf);
    if(*buf) {
      if((p = strchr(buf, ':'))) {
        *p++ = '\0';
        trim(buf);
        for(i=0; i<NUM_FIELDS; i++) {
          if(!strcmp(buf, fields[i].name))
            break;
        }

        if(i >= NUM_FIELDS) {
          fprintf(stderr, "Unknown field \"%s\".\n", buf);
          return 0;
        }
        memset(ip+fields[i].pos, ' ', fields[i].len);

        while(*p == ' ' || *p == '\t')
          p++;

        if(strlen(p)>fields[i].len) {
          fprintf(stderr, "Data for field \"%s\" is too long.\n", fields[i].name);
          return 0;
        }
        memcpy(ip+fields[i].pos, p, strlen(p));
        if(fields[i].extra_check!=NULL &&
          !(*fields[i].extra_check)(ip+fields[i].pos, &fields[i]))
          return 0;

        filled_in[i] = 1;
      } else {
        fprintf(stderr, "Missing : on line.\n");
        return 0;
      }
    }
  }

  for(i=0; i<NUM_FIELDS; i++) {
    if(!filled_in[i]) {
      fprintf(stderr, "Missing value for \"%s\".\n", fields[i].name);
      return 0;
    }
  }

  return 1;
}

int calcCRC(const unsigned char *buf, int size)
{
  int i, c, n = 0xffff;
  for (i = 0; i < size; i++) {
    n ^= (buf[i]<<8);
    for (c = 0; c < 8; c++) {
      if (n & 0x8000)
        n = (n << 1) ^ 4129;
      else
        n = (n << 1);
    }
  }

  return n & 0xffff;
}

void update_crc(char *ip)
{
  int n = calcCRC((unsigned char *)(ip+0x40), 16);
  char buf[5];

  sprintf(buf, "%04X", n);
  if(memcmp(buf, ip+0x20, 4)) {
    memcpy(ip+0x20, buf, 4);
  }
}

void makeip_with_options(char *mr_file, char *out)
{
  int i; 
  char *p;
  FILE *fh;

  for (i = HardwareID; i <= GameTitle; i++) {
    memset(ip+fields[i].pos, ' ', fields[i].len);

    p = inputs[i];
    while(*p == ' ' || *p == '\t')
      p++;

    if(strlen(p)>fields[i].len) {
      fprintf(stderr, "Data for field \"%s\" is too long.\n", fields[i].name);
      return;
    }

    memcpy(ip+fields[i].pos, p, strlen(p));
  }

  update_crc(ip);

  if(mr_file != NULL && strcmp(mr_file, ""))
    insert_mr(ip, mr_file);

  fh = fopen(out, "wb");
  if(fh == NULL) {
    fprintf(stderr, "Can't open \"%s\".\n", out);
    exit(1);
  }

  if(fwrite(ip, 1, 0x8000, fh) != 0x8000) {
    fprintf(stderr, "Write error.\n");
    exit(1);
  }

  fclose(fh);
}

void makeip(char *in, char *out)
{
  FILE *fh = fopen(in, "r");
  if(fh == NULL) {
    fprintf(stderr, "Can't open \"%s\".\n", in);
    exit(1);
  }

  if(!parse_input(fh, ip))
    exit(1);

  fclose(fh);
  update_crc(ip);

  fh = fopen(out, "wb");
  if(fh == NULL) {
    fprintf(stderr, "Can't open \"%s\".\n", out);
    exit(1);
  }

  if(fwrite(ip, 1, 0x8000, fh) != 0x8000) {
    fprintf(stderr, "Write error.\n");
    exit(1);
  }

  fclose(fh);
}

void usage(void)
{
    printf("makeip V1.5 \n");
    printf("Usage: makeip [options] IP.BIN\n");
    printf("       makeip ip.txt IP.BIN\n\n");
    printf("Options:\n");
    printf("\t-b <bootfilename> Boot filename (default: 1ST_READ.BIN)\n");
    printf("\t-c <companyname>  Company name (default: COMPANY)\n");
    printf("\t-d <releasedate>  Release date (format: YYYYMMDD, default: 20000627)\n");
    printf("\t-g <gametitle>    Name of the software (default: GAMETITLE)\n");
    printf("\t-h                Usage information (you\'re looking at it)\n");
    printf("\t-i <mrfilename>   Insert a mr image into the IP.BIN\n");
    printf("\t-v <version>      Product version (default: V1.000)\n\n");
}

int main(int argc, char *argv[])
{
  int c;
  char *mr_file = "";
  int parsed_options = 0;

  if(argc < 2) {
    usage();
    exit(1);
  }

  while ((c = getopt (argc, argv, "b:c:d:g:hi:v:")) != -1)
  {
    switch (c)
    {
      case 'b':
        parsed_options++;
        inputs[BootFilename] = optarg;
        break;
      case 'c':
        parsed_options++;
        inputs[SWMakerName] = optarg;
        break;
      case 'd':
        parsed_options++;
        inputs[ReleaseDate] = optarg;
        break;
      case 'g':
        parsed_options++;
        inputs[GameTitle] = optarg;
        break;
      case 'h':
        usage();
        exit(0);
        break;
      case 'i':
        parsed_options++;
        mr_file = optarg;
        break;
      case 'v':
        parsed_options++;
        inputs[Version] = optarg;
        break;
      case '?':
        fprintf(stderr, "Unknown option: %c\n", optopt); 
        break;
    }
  }

  if(parsed_options == 0 && argc == 3) {
    makeip(argv[1], argv[2]);
  } 
  else if((parsed_options > 0 && argc > 3) ||
          (parsed_options == 0 && argc == 2)){
    makeip_with_options(mr_file, argv[argc-1]);
  }
  else {
    fprintf(stderr, "Something went wrong. You're probably missing an argument for an option.\n");
  }

  return 0;
}
