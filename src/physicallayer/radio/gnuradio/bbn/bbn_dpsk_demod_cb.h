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


#ifndef INCLUDED_BBN_DPSK_DMOD_DB_H
#define INCLUDED_BBN_DPSK_DMOD_DB_H

#include <gnuradio/block.h>

#define RSSI_AVE_COUNT 128
#define RSSI_MAX 0
class bbn_dpsk_demod_cb;
typedef boost::shared_ptr<bbn_dpsk_demod_cb> bbn_dpsk_demod_cb_sptr;

bbn_dpsk_demod_cb_sptr bbn_make_dpsk_demod_cb();


class bbn_dpsk_demod_cb : public gr::block {
  friend bbn_dpsk_demod_cb_sptr bbn_make_dpsk_demod_cb ();

  bbn_dpsk_demod_cb ();
  void update_rssi();

public:
  ~bbn_dpsk_demod_cb ();
  void forecast (int noutput_items, gr_vector_int &ninput_items_required);
  int  general_work (int noutput_items,
                     gr_vector_int &ninput_items,
                     gr_vector_const_void_star &input_items,
                     gr_vector_void_star &output_items);
  void init_log_table();

private:
  gr_complex d_prev;
  float d_e_squared;
  int d_sample_count;
  short d_rssi;
  char d_log_table[64];
};

#endif
