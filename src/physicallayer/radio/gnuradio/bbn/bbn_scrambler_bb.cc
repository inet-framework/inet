/* -*- c++ -*- */
/*
 * Copyright 2006 Free Software Foundation, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <bbn_scrambler_bb.h>
#include <gnuradio/io_signature.h>

#define SCRAMBLER_DEBUG 0
#define DESCRAMBLER_DEBUG 0

bbn_scrambler_bb_sptr
bbn_make_scrambler_bb (bool transmit) {
  if(transmit) {
    return bbn_scrambler_bb_sptr (new bbn_scrambler_bb("tx_scrambler", true));
  } else {
    return bbn_scrambler_bb_sptr (new bbn_scrambler_bb("rx_scrambler", false));
  }    
}

bbn_scrambler_bb::bbn_scrambler_bb (const std::string &name, bool transmit)
  : gr::sync_block (name,
                    gr::io_signature::make (1, 1, sizeof (unsigned char)),
                    gr::io_signature::make (1, 1, sizeof (unsigned char))) {
  int i;

  d_table = new scramble_data_t[256 * 128];

  if(d_table != NULL) {
    for(i=0; i<(128 * 256); ++i) {
      d_table[i].data = i & 0xff;
      d_table[i].seed = i >> 8;
      if(transmit) {
        scramble_bits(&d_table[i].data, &d_table[i].seed, 8);
      } else {
        descramble_bits(&d_table[i].data, &d_table[i].seed, 8);
      }
    }
  }

  if(transmit) {
    d_shift_register = 0x6c;
  } else {
    d_shift_register = 0;
  }
}

bbn_scrambler_bb::~bbn_scrambler_bb() {
  if(d_table) {
    delete[] d_table;
    d_table = NULL;
  }
}

int
bbn_scrambler_bb::work (int noutput_items,
                        gr_vector_const_void_star &input_items,
                        gr_vector_void_star &output_items) {
  int i;
  int index;
  scramble_data_t *table_entry;
  const unsigned char *iptr = (unsigned char *) input_items[0];
  unsigned char *optr = (unsigned char *) output_items[0];

  if(d_table == NULL) {
    return 0;
  }

  for(i = 0; i < noutput_items; i++){
    index = ((int)d_shift_register << 8) | iptr[i];
    table_entry = d_table + index;

    optr[i] = table_entry->data;
    d_shift_register = table_entry->seed;
  }

  return noutput_items;
}

/* 
 * Sends a byte through the 802.11 scrambler or de-scrambler
 * (determined by constructor).  The byte is sent least significant
 * bit first.  The function updates "byte" with the scrambled byte,
 * and "seed" with the new scrambler state.  The first bit out the
 * scrambler is placed into the most significant bit of "byte".
 */
int bbn_scrambler_bb::process_byte(unsigned char *byte, unsigned char *seed) {
  int index;
  scramble_data_t *table_entry;

  if(d_table == NULL) {
    return -1;
  }

  *seed &= 0x7f;

  index = ((int)(*seed) << 8) | (*byte);
  table_entry = d_table + index;

  *byte = table_entry->data;
  *seed = table_entry->seed;

  return 0;
}

/*
 * Sends a series of 8 or less bits through the 802.11 scrambler.  The
 * bits in the "byte" parameter are send through the scrambler least
 * significant bit first, and the result is placed into the least
 * significant bits of "byte", but the rightmost bit contains the last
 * bit to come out of the scrambler.  In other words, input bits are
 * shifted out from the right, and output bits are shifted in from the
 * right. 
 */
int bbn_scrambler_bb::scramble_bits(unsigned char *byte, unsigned char *seed, 
                                    int num_bits) {
  int j;
  int data;
  int output;
  int next_bit;

#if SCRAMBLER_DEBUG  
  int k;
  int test;
#endif

  if((num_bits > 8) || (num_bits < 0)) {
    return -1;
  }

  output = 0;
  data = *byte;
  for(j=0; j<num_bits; ++j) {

#if SCRAMBLER_DEBUG
    test = *seed;

    printf("%d: ", (data >> 7) & 0x01);
    for(k=0; k<7; ++k) {
      printf("%d ", (test >> 6) & 0x01);
      test <<= 1;
    }
#endif

    next_bit = ( (data) +
                 (*seed >> 3) +
                 (*seed) ) & 0x01;
    output = (output << 1) | next_bit;

    *seed >>= 1;
    *seed += (next_bit << 6);
    data >>= 1;

#if SCRAMBLER_DEBUG
    printf("- %d    ", next_bit);
    if(j & 0x01) {
      printf("\n");
    }
#endif
  }

  *byte = (unsigned char)output;

  return 0;
}

/* 
 * Sends a byte through the 802.11 scrambler.  The byte is sent byte
 * significant bit first.  The function updates "byte" with the
 * scrambled byte, and "seed" with the new scrambler state.  The first
 * bit out the descrambler is placed into the most significant byte of
 * "byte".
 */
int bbn_scrambler_bb::descramble_bits(unsigned char *byte, unsigned char *seed,
                                      int num_bits) {
  int j;
  unsigned int data;
  unsigned char output;
  unsigned int next_bit;
#if DESCRAMBLER_DEBUG  
  int k;
  int test;
#endif

  if((num_bits > 8) || (num_bits < 0)) {
    return -1;
  }

  output = 0;
  data = *byte;
  for(j=0; j<num_bits; ++j) {
#if DESCRAMBLER_DEBUG
    test = *seed;
    
    printf("%d: ", (data >> 7) & 0x01);
    for(k=0; k<7; ++k) {
      printf("%d ", (test >> 6) & 0x01);
      test <<= 1;
    }
#endif

    next_bit = ( (data >> 7) +
                 (*seed >> 3) +
                 (*seed) ) & 0x01;
    output = (output << 1) | (next_bit);
      
    *seed >>= 1;
    *seed += ((data >> 1) & 0x40);
    data <<= 1;

#if DESCRAMBLER_DEBUG
    printf("- %d    ", next_bit);
    if(j & 0x01) {
      printf("\n");
    }
#endif
  }
  *byte = output;

  return 0;
}
