/* -*- c++ -*- */

#ifndef INCLUDED_BBN_TRANSMITTER_H
#define INCLUDED_BBN_TRANSMITTER_H

#include <gnuradio/top_block.h>
#include <gnuradio/msg_queue.h>
#include <gnuradio/blocks/message_source.h>
#include <gnuradio/blocks/message_sink.h>
#include "bbn_transmit_path.h"

class bbn_transmitter;
typedef boost::shared_ptr<bbn_transmitter> bbn_transmitter_sptr;

bbn_transmitter_sptr bbn_make_transmitter(int spb, double alpha, double gain, bool use_barker=false, int msgq_limit=2);


class bbn_transmitter : virtual public gr::top_block {
  friend bbn_transmitter_sptr bbn_make_transmitter(int spb, double alpha, double gain, bool use_barker, int msgq_limit);

  bbn_transmitter(int spb, double alpha, double gain, bool use_barker, int msgq_limit);
  void send(const char* data, int length);
  void end();

public:
  ~bbn_transmitter();
  char* transmit(const char* data, int &length/*inout*/);

private:
  bbn_transmit_path_sptr d_transmit_path;
  gr::msg_queue::sptr d_input_queue;
  gr::blocks::message_source::sptr d_tx_input;
  gr::msg_queue::sptr d_output_queue;
  gr::blocks::message_sink::sptr d_tx_output;
};

#endif
