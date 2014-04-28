/* -*- c++ -*- */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "bbn_decapsulator.h"
#include <bbn_tap.h>
#include <gnuradio/io_signature.h>
#include <gnuradio/blocks/pdu.h>

bbn_decapsulator_sptr bbn_make_decapsulator () {
  return bbn_decapsulator_sptr (new bbn_decapsulator());
}


bbn_decapsulator::bbn_decapsulator()
  : block("decapsulator",
     gr::io_signature::make (0, 0, 0),
     gr::io_signature::make (0, 0, 0))
{
  message_port_register_out(PDU_PORT_ID);
  message_port_register_in(PDU_PORT_ID);
  set_msg_handler(PDU_PORT_ID, boost::bind(&bbn_decapsulator::decapsulate, this, _1));
}

void
bbn_decapsulator::decapsulate(pmt::pmt_t msg)
{
    pmt::pmt_t meta = pmt::car(msg);
    pmt::pmt_t vector = pmt::cdr(msg);
    size_t len = pmt::length(vector);
    size_t offset(0);
    const uint8_t* d = (const uint8_t*) pmt::uniform_vector_elements(vector, offset);

    oob_hdr_t *oob;
    if (len < sizeof(*oob))
        return;

    pmt::pmt_t dict(pmt::make_dict());

    oob = (oob_hdr_t *)d;
    dict = pmt::dict_add(dict, pmt::mp("timestamp"), pmt::from_uint64(oob->timestamp));
    dict = pmt::dict_add(dict, pmt::mp("rssi"), pmt::from_long(oob->rssi));
    dict = pmt::dict_add(dict, pmt::mp("rate"), pmt::from_long(oob->rate));

    if (oob->length < len-sizeof(*oob))
        return;

    pmt::pmt_t vecpmt(pmt::make_blob(d+sizeof(*oob), oob->length));
    pmt::pmt_t pdu(pmt::cons(dict, vecpmt));
    message_port_pub(PDU_PORT_ID, pdu);
}
