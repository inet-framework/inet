/* -*- c++ -*- */

#ifndef INCLUDED_BBN_RECEIVE_PATH_CC_H
#define INCLUDED_BBN_RECEIVE_PATH_CC_H

#include <gnuradio/hier_block2.h>
#include <gnuradio/filter/fir_filter_ccf.h>
#include <gnuradio/filter/firdes.h>
#include "bbn_slicer_cc.h"
#include "bbn_dpsk_demod_cb.h"
#include "bbn_plcp80211_bb.h"
#include "bbn_decapsulator.h"

class bbn_receive_path;
typedef boost::shared_ptr<bbn_receive_path> bbn_receive_path_sptr;

bbn_receive_path_sptr bbn_make_receive_path(gr::msg_queue::sptr target_queue, int spb, double alpha, bool use_barker=false, bool check_crc=true,
                                            const std::string &stop_tagname="");


class bbn_receive_path : virtual public gr::hier_block2 {
  friend bbn_receive_path_sptr bbn_make_receive_path(gr::msg_queue::sptr target_queue, int spb, double alpha, bool use_barker, bool check_crc,
                                                     const std::string &stop_tagname);

  bbn_receive_path(gr::msg_queue::sptr target_queue, int spb, double alpha, bool use_barker, bool check_crc, const std::string &stop_tagname);

public:
  ~bbn_receive_path();
private:
  gr::filter::fir_filter_ccf::sptr d_rx_filter;
  bbn_slicer_cc_sptr d_slicer;
  bbn_dpsk_demod_cb_sptr d_dpsk_demod;
  bbn_plcp80211_bb_sptr d_plcp;
};

#endif
