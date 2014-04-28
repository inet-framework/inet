
#ifndef INCLUDED_BBN_ENCAPSULATOR_H
#define INCLUDED_BBN_ENCAPSULATOR_H

#include <gnuradio/blocks/api.h>
#include <gnuradio/block.h>


class bbn_encapsulator;
typedef boost::shared_ptr<bbn_encapsulator> bbn_encapsulator_sptr;

bbn_encapsulator_sptr bbn_make_encapsulator();

class bbn_encapsulator : public gr::block
{
  public:
    bbn_encapsulator();
    void encapsulate(pmt::pmt_t msg);
};

#endif
