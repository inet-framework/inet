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


#ifndef INCLUDED_BBN_SCRAMBLER_BB_H
#define INCLUDED_BBN_SCRAMBLER_BB_H

#include <gnuradio/sync_block.h>

class bbn_scrambler_bb;
typedef boost::shared_ptr<bbn_scrambler_bb> bbn_scrambler_bb_sptr;

bbn_scrambler_bb_sptr bbn_make_scrambler_bb(bool transmit);

typedef struct scramble_data_struct {
  unsigned char data;
  unsigned char seed;
} scramble_data_t;

/*!
 * \brief 802.11 Scrambler
 * input: byte; output: byte
 */
class bbn_scrambler_bb : public gr::sync_block {
  friend bbn_scrambler_bb_sptr bbn_make_scrambler_bb(bool transmit);

protected:
  int d_shift_register;
  scramble_data_t *d_table;
  bbn_scrambler_bb (const std::string &name, bool transmit);

public:
  ~bbn_scrambler_bb();
  virtual int work (int noutput_items,
                    gr_vector_const_void_star &input_items,
                    gr_vector_void_star &output_items);
  int process_byte(unsigned char *byte, unsigned char *seed);
  int scramble_bits(unsigned char *byte, unsigned char *seed, int num_bits);
  int descramble_bits(unsigned char *byte, unsigned char *seed, int num_bits);
};


#endif
