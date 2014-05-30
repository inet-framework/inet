#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "bbn_message_source.h"
#include <gnuradio/io_signature.h>
#include <cstdio>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdexcept>
#include <string.h>

using namespace gr;
using namespace pmt;

bbn_message_source_sptr
bbn_make_message_source(size_t itemsize, int msgq_limit)
{
    return gnuradio::get_initial_sptr
            (new bbn_message_source(itemsize, msgq_limit));
}

bbn_message_source_sptr
bbn_make_message_source(size_t itemsize, msg_queue::sptr msgq)
{
    return gnuradio::get_initial_sptr
            (new bbn_message_source(itemsize, msgq));
}

bbn_message_source_sptr
bbn_make_message_source(size_t itemsize, msg_queue::sptr msgq,
        const std::string& lengthtagname, const std::string &extra_tagname,
        pmt_t extra_tagvalue, int extra_tagoffset)
{
    return gnuradio::get_initial_sptr
            (new bbn_message_source(itemsize, msgq, lengthtagname, extra_tagname, extra_tagvalue, extra_tagoffset));
}

bbn_message_source::bbn_message_source(size_t itemsize, int msgq_limit)
: sync_block("message_source",
        io_signature::make(0, 0, 0),
        io_signature::make(1, 1, itemsize)),
        d_itemsize(itemsize), d_msgq(msg_queue::make(msgq_limit)),
        d_msg_offset(0), d_eof(false), d_tags(false)
{
}

bbn_message_source::bbn_message_source(size_t itemsize, msg_queue::sptr msgq)
: sync_block("message_source",
        io_signature::make(0, 0, 0),
        io_signature::make(1, 1, itemsize)),
        d_itemsize(itemsize), d_msgq(msgq),
        d_msg_offset(0), d_eof(false), d_tags(false)
{
}

bbn_message_source::bbn_message_source(size_t itemsize, msg_queue::sptr msgq,
        const std::string& lengthtagname,
        const std::string &extra_tagname, pmt_t extra_tagvalue, int extra_tagoffset)
: sync_block("message_source",
        io_signature::make(0, 0, 0),
        io_signature::make(1, 1, itemsize)),
        d_itemsize(itemsize), d_msgq(msgq), d_msg_offset(0), d_eof(false),
        d_tags(true), d_lengthtagname(lengthtagname),
        d_extra_tagname(extra_tagname), d_extra_tagvalue(extra_tagvalue),
        d_extra_tagoffset(extra_tagoffset), d_next_offset_to_tag(0)
{
}

bbn_message_source::~bbn_message_source()
{
}

int
bbn_message_source::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
{
    char *out = (char*)output_items[0];
    int nn = 0;

    while(nn < noutput_items) {
        if (d_msg){
            //
            // Consume whatever we can from the current message
            //
            int mm = std::min(noutput_items - nn, (int)((d_msg->length() - d_msg_offset) / d_itemsize));
            memcpy (out, &(d_msg->msg()[d_msg_offset]), mm * d_itemsize);

            if (d_tags && (d_msg_offset == 0)) {
                const uint64_t offset = this->nitems_written(0) + nn;
                pmt::pmt_t key = pmt::string_to_symbol(d_lengthtagname);
                pmt::pmt_t value = pmt::from_long(d_msg->length());
                this->add_item_tag(0, offset, key, value);
            }
            nn += mm;
            out += mm * d_itemsize;
            d_msg_offset += mm * d_itemsize;
            assert(d_msg_offset <= d_msg->length());

            // write the extra tag into the stream
            if (d_tags && !d_extra_tagname.empty()) {
                d_next_offset_to_tag -= mm;
                if (d_next_offset_to_tag < 0 && d_next_offset_to_tag + mm >= 0) {
                    const uint64_t offset = this->nitems_written(0) + nn + d_next_offset_to_tag;
                    pmt::pmt_t key = pmt::string_to_symbol(d_extra_tagname);
                    this->add_item_tag(0, offset, key, d_extra_tagvalue);
                }
            }

            if (d_msg_offset == d_msg->length()){
                if (d_msg->type() == 1)            // type == 1 sets EOF
                    d_eof = true;
                d_msg.reset();
            }
        }
        else {
            //
            // No current message
            //
            if (d_msgq->empty_p() && nn > 0){    // no more messages in the queue, return what we've got
                break;
            }

            if (d_eof)
                return -1;

            d_msg = d_msgq->delete_head();       // block, waiting for a message
            d_msg_offset = 0;

            if ((d_msg->length() % d_itemsize) != 0)
                throw std::runtime_error("msg length is not a multiple of d_itemsize");

            if (d_tags) {
                int item_count = d_msg->length()/d_itemsize;
                d_next_offset_to_tag = d_extra_tagoffset >= 0 ?
                                     std::min(d_extra_tagoffset, item_count-1) :
                                     std::max(item_count + d_extra_tagoffset, 0);
            }
        }
    }

    return nn;
}

