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

#include <bbn_dpsk_demod_cb.h>
#include <gnuradio/io_signature.h>
#include <gnuradio/expj.h>
#include <gnuradio/sincos.h>
#include <math.h>

#define BBN_SLICER_DEBUG 1

bbn_dpsk_demod_cb_sptr bbn_make_dpsk_demod_cb () {
  return bbn_dpsk_demod_cb_sptr (new bbn_dpsk_demod_cb ());
}

bbn_dpsk_demod_cb::~bbn_dpsk_demod_cb () {
}

bbn_dpsk_demod_cb::bbn_dpsk_demod_cb ()
  : gr::block ("bbn_dpsk_demod_cb",
               gr::io_signature::make (1, 1, sizeof (gr_complex)),
               gr::io_signature::make (1, 2, sizeof (unsigned short))),
    d_e_squared(0),
    d_sample_count(0),
    d_rssi(0) {
  d_prev = gr_complex(0,0);
  set_relative_rate((float)1 / (float)8);
  init_log_table();
}

void
bbn_dpsk_demod_cb::forecast (int noutput_items, 
                             gr_vector_int &ninput_items_required) {
  unsigned ninputs = ninput_items_required.size();

  for (unsigned i = 0; i < ninputs; i++) {
    ninput_items_required[i] = noutput_items * 8;
  }
}

int
bbn_dpsk_demod_cb::general_work (int noutput_items,
                                 gr_vector_int &ninput_items,
                                 gr_vector_const_void_star &input_items,
                                 gr_vector_void_star &output_items) {
  int nstreams = output_items.size();

  gr_complex *iptr = (gr_complex *) input_items[0];
  unsigned short *optr1 = (unsigned short *) output_items[0];
  unsigned short *optr2;
  int i, j;
  float innerProd;
  float outerProd;

  if(nstreams == 1) {
    for(i=0; i<noutput_items; ++i) {
      *optr1 = 0;
      for(j=0; j<8; ++j) {
	float mag_squared;
	mag_squared = (iptr->real() * iptr->real()) +
	  (iptr->imag() * iptr->imag());
	d_e_squared += mag_squared;
	++d_sample_count;
	
	if(d_sample_count == RSSI_AVE_COUNT) {
	  update_rssi();
	}

        innerProd = ((iptr->real() * d_prev.real()) + 
                     (iptr->imag() * d_prev.imag()) );
        if(innerProd > 0) {
          *optr1 <<= 1;
        } else {
          *optr1 = (*optr1 << 1) | 0x01;
        }
        d_prev = *iptr;
        ++iptr;
      }
      ++optr1;
    }
  } else {
    optr2 = (unsigned short *) output_items[1];
    for(i=0; i<noutput_items; ++i) {
      *optr1 = 0;
      *optr2 = 0;
      for(j=0; j<8; ++j) {
	float mag_squared;
	mag_squared = (iptr->real() * iptr->real()) +
	  (iptr->imag() * iptr->imag());
	d_e_squared += mag_squared;
	++d_sample_count;
	
	if(d_sample_count == RSSI_AVE_COUNT) {
	  update_rssi();
	}
        innerProd = ((iptr->real() * d_prev.real()) + 
                     (iptr->imag() * d_prev.imag()) );

        outerProd = ((iptr->imag() * d_prev.real()) -
                     (iptr->real() * d_prev.imag()) );
        *optr2 <<= 2;
        *optr1 <<= 1;
        if(innerProd > 0) {
          if(outerProd > 0) {
            if((innerProd - outerProd) < 0) {
              /* 90 degree rotation */
              *optr2 |= 0x01;
            } else {
              /* 0 degree rotation*/
              *optr2 |= 0x00;
            }
          } else {
            /* outerProd < 0 */
            if((innerProd + outerProd) < 0) {
              /* -90 degree rotation */
              *optr2 |= 0x02;
            } else {
              /* 0 degree rotation */
              *optr2 |= 0x00;
            }
          }
        } else {
          /* innerProd < 0  */
          *optr1 |= 0x01;
          if(outerProd > 0) {
            if((outerProd + innerProd) < 0) {
              /* 180 degree rotation */
              *optr2 |= 0x03;
            } else {
              /* 90 degree rotation */
              *optr2 |= 0x01;
            }
          } else {
            /* outerProd < 0 */
            if((outerProd - innerProd) < 0) {
              /* -90 degree rotation */
              *optr2 |= 0x02;
            } else {
              /* 180 degree rotation */
              *optr2 |= 0x03;
            }
          }
        }
        d_prev = *iptr;
        ++iptr;
	}

      *optr1 |= d_rssi;
      ++optr1;
      ++optr2;
    }
  }

  consume_each(noutput_items * 8);

  return noutput_items;
}

void bbn_dpsk_demod_cb::update_rssi() {
  unsigned long rssi_linear;
  int j;

  rssi_linear = (unsigned long)(d_e_squared / 
				((float)RSSI_AVE_COUNT * 10.0));
  d_rssi = RSSI_MAX;
  for(j=0; j<16; ++j) {
    if(rssi_linear & 0xC0000000) {
      break;
    }
    rssi_linear <<= 2;
    d_rssi -= 6;
  }
  
  rssi_linear >>= (32 - 6);
  rssi_linear &= 0x3f;
  d_rssi += d_log_table[rssi_linear];
  d_sample_count = 0;
  d_e_squared = 0;
  
  d_rssi <<= 8;
}

void bbn_dpsk_demod_cb::init_log_table() {
  float dB;
  int threshold;
  int i, j;  

  dB = 0.5;
  j = 0;
  for(i=0; i<7; ++i) {
    threshold = (int)ceil(powf(10, dB / 10) * 16);
    if(threshold > 64) {
      threshold = 64;
    }
    for(; j < threshold; ++j) {
      d_log_table[j] = i;
    }

    dB += 1;
  }
}
