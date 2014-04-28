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


#ifndef INCLUDED_BBN_SLICER_CC_H
#define INCLUDED_BBN_SLICER_CC_H

#include <gnuradio/block.h>

class bbn_slicer_cc;
typedef boost::shared_ptr<bbn_slicer_cc> bbn_slicer_cc_sptr;

bbn_slicer_cc_sptr bbn_make_slicer_cc (int samples_per_symbol,
				       int num_symbols);


/*!
 * \brief Carrier tracking PLL for QPSK
 * input: complex; output: complex
 */
class bbn_slicer_cc : public gr::block {
  friend bbn_slicer_cc_sptr bbn_make_slicer_cc (int samples_per_symbol,
						int num_symbols);

  bbn_slicer_cc (int samples_per_symbol, int num_symbols);

public:
  ~bbn_slicer_cc ();
  void forecast (int noutput_items, gr_vector_int &ninput_items_required);
  int  general_work (int noutput_items,
		     gr_vector_int &ninput_items,
		     gr_vector_const_void_star &input_items,
		     gr_vector_void_star &output_items);


private:
  int d_samples_per_symbol;
  int d_sample_block_size;
  int d_symbol_index;
  int d_offset;
  float d_f_offset;
  float d_f_samples_per_symbol;
  float d_gain;
  float *d_sums;
};

#endif
