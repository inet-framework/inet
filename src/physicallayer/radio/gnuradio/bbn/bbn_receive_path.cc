/* -*- c++ -*- */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/blocks/pdu.h>
#include "bbn_receive_path.h"
#include "bbn_crc16.h"

using namespace std;

bbn_receive_path_sptr bbn_make_receive_path (gr::msg_queue::sptr target_queue, int spb, double alpha, bool use_barker, bool check_crc) {
  return gnuradio::get_initial_sptr (new bbn_receive_path(target_queue, spb, alpha, use_barker, check_crc));
}

bbn_receive_path::~bbn_receive_path () {
}

bbn_receive_path::bbn_receive_path (gr::msg_queue::sptr target_queue, int spb, double alpha, bool use_barker, bool check_crc)
  : gr::hier_block2 ("bbn_receive_path",
               gr::io_signature::make (1, 1, sizeof(gr_complex)),
               gr::io_signature::make (0, 0, 0))
{

    //message_port_register_hier_in(PDU_PORT_ID);

    // TODO: check spb >= 2

    int ntaps = 2 * spb -1;

    if (use_barker) {
        // TODO
    }
    else {
        vector<float> rrc_taps = gr::filter::firdes::root_raised_cosine(1, spb, 1.0, alpha, ntaps);
        d_rx_filter = gr::filter::fir_filter_ccf::make(1, rrc_taps);
    }

    d_slicer = bbn_make_slicer_cc(spb, 16);
    d_dpsk_demod = bbn_make_dpsk_demod_cb();
    d_plcp = bbn_make_plcp80211_bb(target_queue, check_crc);
    //d_decapsulator = bbn_make_decapsulator();

    connect(self(), 0, d_rx_filter, 0);
    connect(d_rx_filter, 0, d_slicer, 0);
    connect(d_slicer, 0, d_dpsk_demod, 0);
    connect(d_dpsk_demod, 0, d_plcp, 0);
    connect(d_dpsk_demod, 1, d_plcp, 1);
    //msg_connect(d_plcp, PDU_PORT_ID, d_decapsulator, PDU_PORT_ID);
    //msg_connect(d_decapsulator, PDU_PORT_ID, self(), PDU_PORT_ID);

    bbn_crc16_init(); // TODO find better place
}
