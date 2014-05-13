/* -*- c++ -*- */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "bbn_encapsulator_bb.h"
#include "bbn_crc16.h"
#include <gnuradio/io_signature.h>

bbn_encapsulator_bb_sptr bbn_make_encapsulator_bb (bool generate_crc, const std::string& length_tag_name) {
  return bbn_encapsulator_bb_sptr (new bbn_encapsulator_bb (generate_crc, length_tag_name));
}

bbn_encapsulator_bb::bbn_encapsulator_bb(bool generate_crc, const std::string& length_tag_name)
  : tagged_stream_block("encapsulator_bb",
     gr::io_signature::make (1, 1, sizeof(char)),
     gr::io_signature::make (1, 1, sizeof(char)),
     length_tag_name), d_generate_crc(generate_crc)
{
    set_tag_propagation_policy(TPP_DONT);
}

int bbn_encapsulator_bb::calculate_output_stream_length(const gr_vector_int &ninput_items) {
    int len = ninput_items[0];
    return d_generate_crc ? 18 + 4 + 2 + len + 4 + 7 :
                            18 + 4 + 2 + len + 7;
}

int bbn_encapsulator_bb::work(int noutput_items,
                              gr_vector_int &ninput_items,
                              gr_vector_const_void_star &input_items,
                              gr_vector_void_star &output_items)
{
    const uint8_t *in = (const uint8_t*) input_items[0];
    uint8_t *out = (uint8_t *) output_items[0];
    size_t payload_len = ninput_items[0];

    int k = 0;

    // preamble
    for (int i=0; i < 16; i++) // sync
        out[k++] = 0xff;
    out[k++] = 0xa0;       // start frame delim
    out[k++] = 0xf3;

    // header
    out[k++] = 0x0A;       // signal 0x0A=1Mbps 0x14=2Mbps
    out[k++] = 0x00;       // service
    size_t length = d_generate_crc ? payload_len+4 : payload_len;
    out[k++] = (length << 3) & 0xff; // length
    out[k++] = (length >> 5) & 0xff;
    unsigned short plcp_crc = bbn_crc16(&out[18], k-18); // header (signal+service+length) crc
    out[k++] = plcp_crc & 0xff;
    out[k++] = plcp_crc >> 8;

    // payload + optional crc
    for (unsigned int i=0; i<payload_len; i++)
        out[k++] = in[i];
    if (d_generate_crc)
    {
        unsigned int payload_crc = bbn_crc32_le(in, payload_len);
        out[k++] = (payload_crc >> 0) & 0xff;
        out[k++] = (payload_crc >> 8) & 0xff;
        out[k++] = (payload_crc >> 16) & 0xff;
        out[k++] = (payload_crc >> 24) & 0xff;
    }

    // padding?
    for (int i=0; i < 7; i++)
        out[k++] = 0x00;

    return k;
}
