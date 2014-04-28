/* -*- c++ -*- */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "bbn_transmit_path.h"
#include "bbn_crc16.h"

using namespace std;

bbn_transmit_path_sptr bbn_make_transmit_path (int spb, double alpha, double gain, bool use_barker, const std::string& lengthtagname) {
  return gnuradio::get_initial_sptr (new bbn_transmit_path(spb, alpha, gain, use_barker, lengthtagname));
}

bbn_transmit_path::~bbn_transmit_path () {
}

bbn_transmit_path::bbn_transmit_path (int spb, double alpha, double gain, bool use_barker, const std::string& lengthtagname)
  : gr::hier_block2 ("bbn_transmit_path",
               gr::io_signature::make (1, 1, sizeof(char)),
               gr::io_signature::make (1, 1, sizeof(gr_complex)))
{

    // TODO: check spb >= 2

    int bits_per_chunk = 1;

    int ntaps = 2 * spb -1;
    d_bytes_to_chunks = gr::blocks::packed_to_unpacked_bb::make(bits_per_chunk, gr::GR_MSB_FIRST);

    vector<gr_complex> constellation;
    constellation.push_back(gr_complex(0.707,0.707));
    constellation.push_back(gr_complex(-0.707,-0.707));
    d_chunks_to_symbols = gr::digital::chunks_to_symbols_bc::make(constellation);

    d_encapsulator = bbn_make_encapsulator_bb(lengthtagname);
    d_scrambler = bbn_make_scrambler_bb(true);
    d_diff_encoder = gr::digital::diff_encoder_bb::make(2);

    if (use_barker) {
        // TODO
    }
    else {
        vector<float> rrc_taps = gr::filter::firdes::root_raised_cosine(4*gain, spb, 1.0, alpha, ntaps);
        d_tx_filter = gr::filter::interp_fir_filter_ccf::make(spb, rrc_taps);
    }


    connect(self(), 0, d_encapsulator, 0);
    connect(d_encapsulator, 0, d_scrambler, 0);
    connect(d_scrambler, 0, d_bytes_to_chunks, 0);
    connect(d_bytes_to_chunks, 0, d_diff_encoder, 0);
    connect(d_diff_encoder, 0, d_chunks_to_symbols, 0);
    connect(d_chunks_to_symbols, 0, d_tx_filter, 0);
    connect(d_tx_filter, 0, self(), 0);

    bbn_crc16_init(); // TODO find better place
}
