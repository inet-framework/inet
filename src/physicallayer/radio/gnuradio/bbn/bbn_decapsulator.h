
#ifndef INCLUDED_BBN_DECAPSULATOR_H
#define INCLUDED_BBN_DECAPSULATOR_H

#include <gnuradio/blocks/api.h>
#include <gnuradio/block.h>


class bbn_decapsulator;
typedef boost::shared_ptr<bbn_decapsulator> bbn_decapsulator_sptr;

bbn_decapsulator_sptr bbn_make_decapsulator();

class bbn_decapsulator : public gr::block
{
  public:
    bbn_decapsulator();
    void decapsulate(pmt::pmt_t msg);
};

#endif
