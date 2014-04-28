/* -*- c++ -*- */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "bbn_encapsulator.h"
#include "bbn_crc16.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/blocks/pdu.h>

bbn_encapsulator_sptr bbn_make_encapsulator () {
  return bbn_encapsulator_sptr (new bbn_encapsulator ());
}


bbn_encapsulator::bbn_encapsulator()
  : block("encapsulator",
     gr::io_signature::make (0, 0, 0),
     gr::io_signature::make (0, 0, 0))
{
  message_port_register_out(PDU_PORT_ID);
  message_port_register_in(PDU_PORT_ID);
  set_msg_handler(PDU_PORT_ID, boost::bind(&bbn_encapsulator::encapsulate, this, _1));
}

void
bbn_encapsulator::encapsulate(pmt::pmt_t msg)
{
    pmt::pmt_t meta = pmt::car(msg);
    pmt::pmt_t vector = pmt::cdr(msg);
    size_t len = pmt::length(vector);
    size_t offset(0);
    const uint8_t* d = (const uint8_t*) pmt::uniform_vector_elements(vector, offset);

    std::vector<unsigned char> vec;
    for (int i=0; i < 16; i++) // sync
        vec.push_back(0xff);
    vec.push_back(0xa0);       // start frame delim
    vec.push_back(0xf3);

    vec.push_back(0x0A);       // signal 0x0A=1Mbps 0x14=2Mbps
    vec.push_back(0x00);       // service
    vec.push_back(((len+4) << 3) & 0xff); // length
    vec.push_back(((len+4) >> 5) & 0xff);

    unsigned short plcp_crc = bbn_crc16(&vec[18], vec.size()-18); // header (signal+service+length) crc
    vec.push_back(plcp_crc & 0xff);
    vec.push_back(plcp_crc >> 8);

    for (unsigned int i=0; i<len; i++)
        vec.push_back(d[i]);

    unsigned int payload_crc = bbn_crc32_le(d, len);
    vec.push_back((payload_crc >> 0) & 0xff);
    vec.push_back((payload_crc >> 8) & 0xff);
    vec.push_back((payload_crc >> 16) & 0xff);
    vec.push_back((payload_crc >> 24) & 0xff);

    for (int i=0; i < 7; i++)
        vec.push_back(0x00);

    // send the vector
    pmt::pmt_t vecpmt(pmt::make_blob(&vec[0], vec.size()));
    pmt::pmt_t pdu(pmt::cons(meta, vecpmt));
    message_port_pub(PDU_PORT_ID, pdu);
}
