#ifndef INCLUDED_BBN_MESSAGE_SOURCE_H
#define INCLUDED_BBN_MESSAGE_SOURCE_H

#include <gnuradio/blocks/api.h>
#include <gnuradio/sync_block.h>
#include <gnuradio/msg_queue.h>
#include <gnuradio/message.h>

class bbn_message_source;
typedef boost::shared_ptr<bbn_message_source> bbn_message_source_sptr;

bbn_message_source_sptr bbn_make_message_source(size_t itemsize, int msgq_limit=0);
bbn_message_source_sptr bbn_make_message_source(size_t itemsize, gr::msg_queue::sptr msgq);
bbn_message_source_sptr bbn_make_message_source(size_t itemsize, gr::msg_queue::sptr msgq, const std::string& lengthtagname,
                                                const std::string &extra_tagname="", pmt::pmt_t extra_tagvalue=pmt::PMT_F, int extra_tagoffset=0);

/*
 * Same as gr::blocks::message_source with the capability to add
 * an extra tag to the stream for each message.
 * The tag location is given as an offset from the start (positive)
 * or end (negative) of the message.
 */
class bbn_message_source : virtual public gr::sync_block
{
    private:
        size_t        d_itemsize;
        gr::msg_queue::sptr   d_msgq;
        gr::message::sptr d_msg;
        unsigned      d_msg_offset;
        bool      d_eof;
        bool              d_tags;
        std::string       d_lengthtagname;
        std::string d_extra_tagname;
        pmt::pmt_t d_extra_tagvalue;
        int d_extra_tagoffset;
        int d_next_offset_to_tag; // relative to the start of the message

    public:
        bbn_message_source(size_t itemsize, int msgq_limit);
        bbn_message_source(size_t itemsize, gr::msg_queue::sptr msgq);
        bbn_message_source(size_t itemsize, gr::msg_queue::sptr msgq,
                const std::string& lengthtagname, const std::string &extra_tagname,
                pmt::pmt_t extra_tagvalue, int extra_tagoffset);

        ~bbn_message_source();

        gr::msg_queue::sptr msgq() const { return d_msgq; }

        int work(int noutput_items,
                gr_vector_const_void_star &input_items,
                gr_vector_void_star &output_items);
};


#endif /* INCLUDED_GR_MESSAGE_SOURCE_H */
