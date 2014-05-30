/* -*- c++ -*- */

#ifndef INCLUDED_BBN_RECEIVER_H
#define INCLUDED_BBN_RECEIVER_H

#include <gnuradio/top_block.h>
#include <gnuradio/msg_queue.h>
#include "bbn_message_source.h"
#include "bbn_receive_path.h"

class bbn_receiver;
typedef boost::shared_ptr<bbn_receiver> bbn_receiver_sptr;

bbn_receiver_sptr bbn_make_receiver(int spb, double alpha, bool use_barker, int msgq_limit=2);


class bbn_receiver : virtual public gr::top_block {
  friend bbn_receiver_sptr bbn_make_receiver(int spb, double alpha, bool use_barker, int msgq_limit);

  bbn_receiver(int spb, double alpha, bool use_barker, int msgq_limit);

public:
  ~bbn_receiver();
  char* receive(const gr_complex* data, int &length/*inout*/);

private:
  bbn_receive_path_sptr d_receive_path;
  gr::msg_queue::sptr d_input_queue;
  bbn_message_source_sptr d_tx_input;
  gr::msg_queue::sptr d_output_queue;
};

#endif
