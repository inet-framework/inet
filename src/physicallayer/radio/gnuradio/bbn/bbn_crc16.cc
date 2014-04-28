/* -*- c++ -*- */
/*
 * Copyright 2005 Free Software Foundation, Inc.
 *
 * Copyright (c) 2006 BBN Technologies Corp.  All rights reserved.
 * Effort sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and the Department of the Interior National Business
 * Center under agreement number NBCHC050166.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * See also ISO 3309 [ISO-3309] or ITU-T V.42 [ITU-V42] for a formal
 * specification.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <bbn_crc16.h>

#define CRC_DEBUG 0

static unsigned int
manual_update_crc(unsigned int crc, const unsigned char *data, 
		  int len, unsigned int poly);

static bool crc_table_initialized = false;
static unsigned short crc16_table[256];
static unsigned int crc32_table[256];

#define CCITT_CRC16_POLY (0x8408)
#define IEEE_802_3_CRC32_POLY (0xEDB88320)

void bbn_crc16_init(void) {
  int i;
  unsigned char c;

  if(crc_table_initialized == false) {
    for(i=0; i<256; ++i) {
      c = (unsigned char)i;
      crc16_table[i] = manual_update_crc(0, &c, 1, CCITT_CRC16_POLY);
      crc32_table[i] = manual_update_crc(0, &c, 1, IEEE_802_3_CRC32_POLY);
    }
    crc_table_initialized = true;
  }
}

unsigned short
bbn_update_crc16(unsigned short crc, const unsigned char *data, 
		 int len) {
  int i;
#if CRC_DEBUG
  int k;
#endif

  if(crc_table_initialized == false) {
    bbn_crc16_init();
  }

  for(i=0; i<len; ++i) {
    crc = (crc >> 8) ^ crc16_table[(crc ^ data[i]) & 0xff];
#if CRC_DEBUG
      for(k=0; k<16; ++k) {
	if(crc & (0x01 << k)) {
	  printf("1");
	} else {
	  printf("0");
	}
      }
      printf("\n");
#endif
  }

#if CRC_DEBUG
  printf("\n");
  printf("crc = %04X\n", crc);
#endif
  return crc;
}

unsigned int
bbn_update_crc32_le(unsigned int crc, const unsigned char *data, 
		    int len) {
  int i;

  if(crc_table_initialized == false) {
    bbn_crc16_init();
  }

  for(i=0; i<len; ++i) {
    crc = (crc >> 8) ^ crc32_table[(crc ^ data[i]) & 0xff];
  }

  return crc;
}

static unsigned int
manual_update_crc(unsigned int crc, const unsigned char *data, 
		  int len, unsigned int poly) {
  int i;
  int j;
  unsigned short ch;
#if CRC_DEBUG
  int k;
#endif

  for(i=0; i<len; ++i) {
    ch = data[i];
    for(j=0; j<8; ++j) {

#if CRC_DEBUG
      if(ch & 0x0001) {
	printf("1: ");
      } else {
	printf("0: ");
      }
#endif

      if((crc ^ ch) & 0x0001) {
	crc = (crc >> 1) ^ poly;
      } else {
	crc = (crc >> 1);
      }
      ch >>= 1;

#if CRC_DEBUG
      for(k=0; k<16; ++k) {
	if(crc & (0x01 << k)) {
	  printf("1");
	} else {
	  printf("0");
	}
      }
      printf("\n");
#endif
    }
  }

#if CRC_DEBUG
  printf("   ");
  for(k=0; k<16; ++k) {
    if(~crc & (0x01 << k)) {
      printf("1");
    } else {
      printf("0");
    }
  }
  printf("\n");
  printf("crc = %04X\n", crc);
#endif

  return crc;
}

unsigned short bbn_update_crc16(unsigned short crc, const std::string s) {
  return bbn_update_crc16(crc, (const unsigned char *) s.data(), s.size());
}
    
unsigned short bbn_crc16(const unsigned char *buf, int len) {
  return bbn_update_crc16(0xffff, buf, len) ^ 0xffff;
}

unsigned short bbn_crc16(const std::string s) {
  return bbn_crc16((const unsigned char *) s.data(), s.size());
}

unsigned int bbn_update_crc32_le(unsigned int crc, const std::string s) {
  return bbn_update_crc32_le(crc, (const unsigned char *) s.data(), s.size());
}
    
unsigned int bbn_crc32_le(const unsigned char *buf, int len) {
  return bbn_update_crc32_le(0xffffffff, buf, len) ^ 0xffffffff;
}

unsigned int bbn_crc32_le(const std::string s) {
  return bbn_crc32_le((const unsigned char *) s.data(), s.size());
}
