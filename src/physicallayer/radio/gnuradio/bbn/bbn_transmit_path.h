/* -*- c++ -*- */

#ifndef INCLUDED_BBN_TRANSMIT_PATH_CC_H
#define INCLUDED_BBN_TRANSMIT_PATH_CC_H

#include <gnuradio/hier_block2.h>
#include <gnuradio/blocks/packed_to_unpacked_bb.h>
#include <gnuradio/digital/chunks_to_symbols_bc.h>
#include <gnuradio/digital/diff_encoder_bb.h>
#include <gnuradio/filter/interp_fir_filter_ccf.h>
#include <gnuradio/filter/firdes.h>
#include "bbn_encapsulator_bb.h"
#include "bbn_scrambler_bb.h"

class bbn_transmit_path;
typedef boost::shared_ptr<bbn_transmit_path> bbn_transmit_path_sptr;

bbn_transmit_path_sptr bbn_make_transmit_path(int spb, double alpha, double gain, bool use_barker=false,
                                              bool generate_crc=true, const std::string& lengthtagname="packet_len");


class bbn_transmit_path : virtual public gr::hier_block2 {
  friend bbn_transmit_path_sptr bbn_make_transmit_path(int spb, double alpha, double gain, bool use_barker,
                                                       bool generate_crc, const std::string& lengthtagname);

  bbn_transmit_path(int spb, double alpha, double gain, bool use_barker, bool generate_crc, const std::string& lengthtagname);

public:
  ~bbn_transmit_path();
private:
  bbn_encapsulator_bb_sptr d_encapsulator;
  bbn_scrambler_bb_sptr d_scrambler;
  gr::blocks::packed_to_unpacked_bb::sptr d_bytes_to_chunks;
  gr::digital::chunks_to_symbols_bc::sptr d_chunks_to_symbols;
  gr::digital::diff_encoder_bb::sptr d_diff_encoder;
  gr::filter::interp_fir_filter_ccf::sptr d_tx_filter;
};

#endif
