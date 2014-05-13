/* -*- c++ -*- */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "bbn_transmitter.h"

using namespace std;
using namespace gr;

bbn_transmitter_sptr bbn_make_transmitter (int spb, double alpha, double gain, bool use_barker, int msgq_limit) {
  return gnuradio::get_initial_sptr (new bbn_transmitter(spb, alpha, gain, use_barker, msgq_limit));
}

bbn_transmitter::~bbn_transmitter () {
}

bbn_transmitter::bbn_transmitter (int spb, double alpha, double gain, bool use_barker, int msgq_limit)
  : top_block ("bbn_transmitter")
{
    d_input_queue = msg_queue::make(msgq_limit);
    d_tx_input = blocks::message_source::make(sizeof(char), d_input_queue, "packet_len");
    d_transmit_path = bbn_make_transmit_path(spb, alpha, gain, use_barker, false, "packet_len");
    d_output_queue = msg_queue::make();
    d_tx_output = blocks::message_sink::make(sizeof(gr_complex), d_output_queue, true/*, "packet_len"*/);

    connect(d_tx_input, 0, d_transmit_path, 0);
    connect(d_transmit_path, 0, d_tx_output, 0);
}

void bbn_transmitter::send(const char* data, int length)
{
    string s(data, length);
    message::sptr msg = message::make_from_string(s);
    d_input_queue->insert_tail(msg);
}

void bbn_transmitter::end()
{
    message::sptr msg = message::make(1/*eof*/);
    d_input_queue->insert_tail(msg);
}

gr_complex* bbn_transmitter::transmit(const char* data, int &length /*inout*/)
{
    start();
    send(data, length);
    end();
    wait();

    string result;
    while (!d_output_queue->empty_p())
    {
        message::sptr msg = d_output_queue->delete_head();
        result += msg->to_string();
    }

    assert(result.size() % sizeof(gr_complex) == 0);
    length = result.size() / sizeof(gr_complex);
    gr_complex *d = new gr_complex[length];
    memcpy(d, result.data(), result.size());
    return d;
}
