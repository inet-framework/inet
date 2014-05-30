/* -*- c++ -*- */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "bbn_receiver.h"

using namespace std;
using namespace gr;

bbn_receiver_sptr bbn_make_receiver (int spb, double alpha, bool use_barker, int msgq_limit) {
  return gnuradio::get_initial_sptr (new bbn_receiver(spb, alpha, use_barker, msgq_limit));
}

bbn_receiver::~bbn_receiver () {
}

bbn_receiver::bbn_receiver (int spb, double alpha, bool use_barker, int msgq_limit)
  : top_block ("bbn_receiver")
{
    d_input_queue = msg_queue::make(msgq_limit);
    d_tx_input = bbn_make_message_source(sizeof(gr_complex), d_input_queue, "packet_len", "last-item", pmt::PMT_T, -64);
    d_output_queue = msg_queue::make();
    d_receive_path = bbn_make_receive_path(d_output_queue, spb, alpha, use_barker, false, "last-item");

    connect(d_tx_input, 0, d_receive_path, 0);
}

char* bbn_receiver::receive(const gr_complex* data, int &length /*inout*/)
{
    string s((const char *)data, length*sizeof(gr_complex));
    message::sptr msg = message::make_from_string(s);
    d_input_queue->insert_tail(msg);

    bool messageReceived = false;
    string result;
    while (!messageReceived)
    {
        while (d_output_queue->empty_p())
            ;
        while (!d_output_queue->empty_p())
        {
            message::sptr msg = d_output_queue->delete_head();
            if (msg->length() == 0)
            {
                messageReceived = true;
                break;
            }
            else
                result += msg->to_string();
        }
    }

    length = result.size();
    char *d = new char[length];
    memcpy(d, result.data(), length);
    return d;
}
