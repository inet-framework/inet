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

#include <bbn_slicer_cc.h>
#include <gnuradio/io_signature.h>
#include <gnuradio/expj.h>
#include <gnuradio/sincos.h>
#include <math.h>

#define BBN_SLICER_DEBUG 0
float bbn_last_rssi;

bbn_slicer_cc_sptr bbn_make_slicer_cc (int samples_per_symbol, 
                                       int num_symbols) {
  return bbn_slicer_cc_sptr (new bbn_slicer_cc (samples_per_symbol,
                                                num_symbols));
}

bbn_slicer_cc::~bbn_slicer_cc () {
  if(d_sums) {
    delete d_sums;
    d_sums = 0;
  }
}

bbn_slicer_cc::bbn_slicer_cc (int samples_per_symbol, int num_symbols)
  : gr::block ("bbn_slicer_cc",
               gr::io_signature::make (1, 1, sizeof (gr_complex)),
               gr::io_signature::make (1, 1, sizeof (gr_complex))),
    d_samples_per_symbol(samples_per_symbol) {
  int i;

  d_sample_block_size = num_symbols * d_samples_per_symbol;
  d_symbol_index = 0;
  d_sums = new float[d_samples_per_symbol];
  d_offset = 0;

  d_f_offset = 0;
  d_f_samples_per_symbol = (float)d_samples_per_symbol;
  d_gain = 0.75;

  for(i=0; i<d_samples_per_symbol; ++i) {
    d_sums[i] = 0;
  }

  set_relative_rate((float)1 / (float)samples_per_symbol);
}


void
bbn_slicer_cc::forecast (int noutput_items, 
                         gr_vector_int &ninput_items_required) {
  unsigned ninputs = ninput_items_required.size();

  for (unsigned i = 0; i < ninputs; i++) {
    ninput_items_required[i] = noutput_items * d_samples_per_symbol;
  }
}

int
bbn_slicer_cc::general_work (int noutput_items,
                             gr_vector_int &ninput_items,
                             gr_vector_const_void_star &input_items,
                             gr_vector_void_star &output_items) {
  gr_complex *iptr = (gr_complex *) input_items[0];
  gr_complex *optr = (gr_complex *) output_items[0];
  int samples_to_process;
  float max_val;

#if BBN_SLICER_DEBUG
  printf("Slicer requested: %d\n", noutput_items);
#endif

  samples_to_process = noutput_items * d_samples_per_symbol;
  while(samples_to_process > 0) {
    int i;
    int j;
    int max_idx;
    int ntaps;
    float delta;

    if((samples_to_process + d_symbol_index) >= d_sample_block_size) {
      ntaps = (d_sample_block_size - d_symbol_index);
      for(i=0; i < ntaps / d_samples_per_symbol; ++i) {
        for(j=0; j < d_samples_per_symbol; ++j) {
          d_sums[j] += 
            (iptr[j].real() * iptr[j].real()) + (iptr[j].imag() * iptr[j].imag());
        }
        *optr = iptr[d_offset];
        iptr += d_samples_per_symbol;
        ++optr;
      }

      samples_to_process -= ntaps;
      d_symbol_index = 0;

      max_val = 0;
      max_idx = 0;
      for(i=0; i<d_samples_per_symbol; ++i) {
        if(max_val < d_sums[i]) {
          max_val = d_sums[i];
          max_idx = i;
        }
      }

      delta = (float)max_idx - d_f_offset;
#if BBN_SLICER_DEBUG
      printf("d_f_offset IN = %f, delta = %f.\n", d_f_offset, delta);
#endif
      if(fabsf(delta) > (d_f_samples_per_symbol * 0.5)) {
        if(delta > 0) {
          delta -= d_f_samples_per_symbol;
        } else {
          delta += d_f_samples_per_symbol;
        }
      }
      d_f_offset += d_gain * delta;

      while(d_f_offset >= (d_f_samples_per_symbol - 0.5)) {
        d_f_offset -= (d_f_samples_per_symbol);
      }

      while(d_f_offset < -0.5) {
        d_f_offset += (d_f_samples_per_symbol);
      }
      d_offset = (int)roundf(d_f_offset);

#if BBN_SLICER_DEBUG
      printf("Process %d samples. d_f_offset = %f.\n", d_sample_block_size, 
             d_f_offset);
      printf("max_idx = %d, d_offset = %d\n", max_idx, d_offset);
#endif
    } else {
      ntaps = (samples_to_process);
      for(i=0; i < ntaps / d_samples_per_symbol; ++i) {
        for(j=0; j < d_samples_per_symbol; ++j) {
          d_sums[j] += 
            (iptr[j].real() * iptr[j].real()) + (iptr[j].imag() * iptr[j].imag());
        }
        *optr = iptr[d_offset];
        iptr += d_samples_per_symbol;
        ++optr;
      }

      d_symbol_index += ntaps;
      samples_to_process = 0;
    }
  }

  consume_each (noutput_items * d_samples_per_symbol);

#if BBN_SLICER_DEBUG
  printf("Slicer produced: %d\n", noutput_items);
#endif

  return noutput_items;
}
