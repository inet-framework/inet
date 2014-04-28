
#ifndef INCLUDED_BBN_ENCAPSULATOR_BB_H
#define INCLUDED_BBN_ENCAPSULATOR_BB_H

#include <gnuradio/blocks/api.h>
#include <gnuradio/block.h>
#include <gnuradio/tagged_stream_block.h>

class bbn_encapsulator_bb;
typedef boost::shared_ptr<bbn_encapsulator_bb> bbn_encapsulator_bb_sptr;

bbn_encapsulator_bb_sptr bbn_make_encapsulator_bb(const std::string& lengthtagname="packet_len");

class bbn_encapsulator_bb : public gr::tagged_stream_block
{
  public:
    bbn_encapsulator_bb(const std::string& length_tag_name);

    virtual int calculate_output_stream_length(const gr_vector_int &ninput_items);

    int work(int noutput_items,
             gr_vector_int &ninput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
};

#endif
