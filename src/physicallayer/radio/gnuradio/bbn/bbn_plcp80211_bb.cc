/* -*- c++ -*- */
/*
 * Copyright 2006 Free Software Foundation, Inc.
 *
 * Copyright (c) 2006 BBN Technologies Corp.  All rights reserved.
 * Effort sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and the Department of the Interior National Business
 * Center under agreement number NBCHC050166.
 * 
 * This file is part of GNU Radio
 * 
 * GNU Radio is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 * 
 * GNU Radio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <gnuradio/blocks/pdu.h>
#include <bbn_plcp80211_bb.h>
#include <bbn_crc16.h>
#include <bbn_tap.h>
#include <math.h>
#include <cstdio>
#include <queue>

#define PLCP_DEBUG 0

static inline unsigned char reverse_bits(unsigned char c);

#define REVERSE_BITS(c) ( bit_reverse_table[(c) & 0xff])

bbn_plcp80211_bb_sptr bbn_make_plcp80211_bb (gr::msg_queue::sptr target_queue,
                                             bool check_crc, const std::string &stop_tagname) {
  return bbn_plcp80211_bb_sptr (new bbn_plcp80211_bb (target_queue, check_crc, stop_tagname));
}

bbn_plcp80211_bb::~bbn_plcp80211_bb () {
}

bbn_plcp80211_bb::bbn_plcp80211_bb (gr::msg_queue::sptr target_queue,
                                    bool check_crc, const std::string &stop_tagname)
  : gr::block ("plcp80211_bb",
               gr::io_signature::make (1, 2, sizeof (unsigned short)),
               gr::io_signature::make (0,0,0)),
    d_symbol_count(0),
    d_packet_rx_time(0),
    d_packet_rate(0),
    d_target_queue(target_queue),
    d_state(PLCP_STATE_SEARCH_PREAMBLE),
    d_data(0),
    d_byte_count(0),
    d_pdu_len(0),
    d_rate(PLCP_RATE_1MBPS),
    d_data1_in(0),
    d_data2_in(0),
    d_check_crc(check_crc),
    d_scrambler_seed(0x62) {
  int i;
  set_relative_rate((float)1);

  for(i=0; i<256; ++i) {
    bit_reverse_table[i] = reverse_bits((unsigned char)i);
  }

  d_descrambler = bbn_make_scrambler_bb(false);

  d_stop_tagkey = stop_tagname.empty() ? pmt::PMT_F : pmt::string_to_symbol(stop_tagname);

  message_port_register_out(PDU_PORT_ID);
}


void
bbn_plcp80211_bb::forecast (int noutput_items,
                            gr_vector_int &ninput_items_required) {
  unsigned ninputs = ninput_items_required.size();

  for (unsigned i = 0; i < ninputs; i++) {
    ninput_items_required[i] = noutput_items/* + 3*/;
  }
}

static inline unsigned char reverse_bits(unsigned char c) {
  unsigned char result;

  result = c & 0x01;
  result <<= 1;
  c >>= 1;

  result |= c & 0x01;
  result <<= 1;
  c >>= 1;

  result |= c & 0x01;
  result <<= 1;
  c >>= 1;

  result |= c & 0x01;
  result <<= 1;
  c >>= 1;

  result |= c & 0x01;
  result <<= 1;
  c >>= 1;

  result |= c & 0x01;
  result <<= 1;
  c >>= 1;

  result |= c & 0x01;
  result <<= 1;
  c >>= 1;

  result |= c & 0x01;

  return result;
}

int
bbn_plcp80211_bb::general_work (int noutput_items,
                                gr_vector_int &ninput_items,
                                gr_vector_const_void_star &input_items,
                                gr_vector_void_star &output_items) {
  int num_output = 0;
  unsigned int sfd_search;
  unsigned short crc;
  struct plcp_hdr_t {
    unsigned char signal;
    unsigned char service;
    unsigned short length;
    unsigned short crc;
  } plcp_hdrp;
  unsigned char descrambled_byte;
  unsigned char descrambled_byte2[2] = {0, 0};
  unsigned char saved_scrambler_seed;
  int symbol_delta;
  oob_hdr_t *oob;

  int i;
  unsigned short *iptr1 = (unsigned short *) input_items[0];
  unsigned short *iptr2 = (unsigned short *) input_items[1];

#if PLCP_DEBUG
  printf("PLCP requested: %d\n", noutput_items);
#endif

  // collect offsets where plcp must emit a stop marker
  uint64_t abs_N = nitems_read(0);
  std::queue<uint64_t> last_item_offsets;
  if (pmt::is_symbol(d_stop_tagkey))
  {
      std::vector<gr::tag_t> tags;
      get_tags_in_range(tags, 0, abs_N, abs_N + (uint64_t)(noutput_items), d_stop_tagkey);
      for (size_t i = 0; i < tags.size(); i++)
          last_item_offsets.push(tags[i].offset);
  }

  num_output = 0;
  for(i=0; i<noutput_items; ++i) {

    saved_scrambler_seed = d_scrambler_seed;

    d_data1_in = (d_data1_in << 8) | (*iptr1 & 0xff);
    d_data2_in = (d_data2_in << 16) | *iptr2;
    if(d_rate == PLCP_RATE_1MBPS) {
      descrambled_byte = (d_data1_in >> d_shift) & 0xff;
      d_descrambler->process_byte(&descrambled_byte, &d_scrambler_seed);
    } else {
      descrambled_byte2[0] = (d_data2_in >> (d_shift + 8)) & 0xff;
      d_descrambler->process_byte(&descrambled_byte2[0], &d_scrambler_seed);

      descrambled_byte2[1] = (d_data2_in >> d_shift) & 0xff;
      d_descrambler->process_byte(&descrambled_byte2[1], &d_scrambler_seed);
    }

#if PLCP_DEBUG
    printf("PLCP i: %d state: %d, data: %d\n", i, (int)d_state, (int)descrambled_byte);
#endif

    switch(d_state) {
    case PLCP_STATE_SEARCH_PREAMBLE:
      if(descrambled_byte == 0xff) {
        d_state = PLCP_STATE_SEARCH_SFD1;
      } else if(descrambled_byte == 0x00) {
        d_state = PLCP_STATE_SEARCH_RSFD1;
      }
      break;
    case PLCP_STATE_SEARCH_SFD1:
      if(descrambled_byte != 0xff) {
        d_data = 0xff000000 | ((unsigned int)descrambled_byte) << 16;
        d_state = PLCP_STATE_SEARCH_SFD2;
      }
      break;
    case PLCP_STATE_SEARCH_SFD2:
      d_data |= ((unsigned int)descrambled_byte) << 8;
      d_state = PLCP_STATE_SEARCH_SFD3;
      break;
    case PLCP_STATE_SEARCH_SFD3:
      d_data |= ((unsigned int)descrambled_byte);
      sfd_search = d_data;

      for(d_shift=0; d_shift<8; ++d_shift) {
        if( (sfd_search & 0x00FFFF00) == (SFD << 8)) {
          break;
        }
        sfd_search <<= 1;
      }

      if(d_shift == 8) {
        d_state = PLCP_STATE_SEARCH_PREAMBLE;
        d_shift = 0;
        break;
      }

      descrambled_byte = d_data1_in & 0xff;
      d_descrambler->descramble_bits(&descrambled_byte, 
                                     &saved_scrambler_seed,
                                     d_shift);

      d_shift = (8 - d_shift);
      d_scrambler_seed = saved_scrambler_seed;

      d_state = PLCP_STATE_HDR;
      d_byte_count = 0;
      break;
    case PLCP_STATE_SEARCH_RSFD1:
      if(descrambled_byte != 0xff) {
        d_data = ((unsigned int)descrambled_byte) << 16;
        d_state = PLCP_STATE_SEARCH_RSFD2;
      }
      break;
    case PLCP_STATE_SEARCH_RSFD2:
      d_data |= ((unsigned int)descrambled_byte) << 8;
      d_state = PLCP_STATE_SEARCH_RSFD3;
      break;
    case PLCP_STATE_SEARCH_RSFD3:
      d_data |= ((unsigned int)descrambled_byte);
      sfd_search = d_data;

      for(d_shift=0; d_shift<8; ++d_shift) {
        if( (sfd_search & 0x00FFFF00) == (RSFD << 8)) {
          break;
        }
        sfd_search <<= 1;
      }

      if(d_shift == 8) {
        d_state = PLCP_STATE_SEARCH_PREAMBLE;
        d_shift = 0;
        break;
      }

      descrambled_byte = d_data1_in & 0xff;
      d_descrambler->descramble_bits(&descrambled_byte, 
                                     &saved_scrambler_seed,
                                     d_shift);

      d_shift = (8 - d_shift) << 1;
      d_scrambler_seed = saved_scrambler_seed;

      d_rate = PLCP_RATE_2MBPS;
      d_state = PLCP_STATE_SHDR;
      d_byte_count = 0;
      break;
    case PLCP_STATE_HDR:
      d_hdr[d_byte_count] = REVERSE_BITS(descrambled_byte);
      ++d_byte_count;
      if(d_byte_count == 6) {
        plcp_hdrp.signal = d_hdr[0];
        plcp_hdrp.service = d_hdr[1];
        plcp_hdrp.length = (( (unsigned short)d_hdr[3]) << 8) | d_hdr[2];
        plcp_hdrp.crc = (( (unsigned short)d_hdr[5]) << 8) | d_hdr[4];
        crc = bbn_crc16(d_hdr, (size_t)4);

#if PLCP_DEBUG
        printf("Recieved header!\n");
        printf("  signal: 0x%02X\n", plcp_hdrp.signal);
        printf("  service: 0x%02X\n", plcp_hdrp.service);
        printf("  length: 0x%04X\n", plcp_hdrp.length);
        printf("  crc: 0x%04X\n", plcp_hdrp.crc);
        printf("Calculated crc: 0x%04X\n", crc);
#endif

        if(plcp_hdrp.crc != crc) {
          d_state = PLCP_STATE_SEARCH_PREAMBLE;
          d_shift = 0;
          break;
        }

        if(plcp_hdrp.signal == 0x0a) {
          /* 1 Mbps */
          d_pdu_len = plcp_hdrp.length >> 3;
          d_state = PLCP_STATE_SEARCH_PREAMBLE;
        } else if(plcp_hdrp.signal == 0x14) {
          /* 2 Mbps */
          d_rate = PLCP_RATE_2MBPS;
          d_shift <<= 1;
          d_pdu_len = plcp_hdrp.length >> 2;
        } else {
          /* Rate is not 1 Mpbs or 2 Mbps */
          d_state = PLCP_STATE_SEARCH_PREAMBLE;
          d_shift = 0;
#if PLCP_DEBUG
          printf("Unsupported Rate = %2.1f Mbps\n",
                 (float)(plcp_hdrp.signal) / 10.0);
#endif
          break;
        }

#if PLCP_DEBUG
        printf("Packet Length: %d bytes.\n", d_pdu_len);
#endif    

        if(d_pdu_len > MAX_PDU_LENGTH) {
#if PLCP_DEBUG
          printf("Ignoring packet because it is too long\n");
#endif
          d_state = PLCP_STATE_SEARCH_PREAMBLE;
          d_shift = 0;
          d_rate = PLCP_RATE_1MBPS;
          break;
        }

        d_state = PLCP_STATE_PDU;
        d_byte_count = 0;
        d_packet_rate = plcp_hdrp.signal;
        symbol_delta = (((unsigned long)iptr1 - (unsigned long)input_items[0])
                        << 3);
        d_packet_rx_time = d_symbol_count + (long long)symbol_delta;

      }
      break;
    case PLCP_STATE_SHDR:
      d_hdr[d_byte_count] = REVERSE_BITS(descrambled_byte2[0]);
      ++d_byte_count;

      d_hdr[d_byte_count] = REVERSE_BITS(descrambled_byte2[1]);
      ++d_byte_count;

      if(d_byte_count == 6) {
        plcp_hdrp.signal = d_hdr[0];
        plcp_hdrp.service = d_hdr[1];
        plcp_hdrp.length = (( (unsigned short)d_hdr[3]) << 8) | d_hdr[2];
        plcp_hdrp.crc = (( (unsigned short)d_hdr[5]) << 8) | d_hdr[4];
        crc = bbn_crc16(d_hdr, (size_t)4);
        
#if PLCP_DEBUG
        printf("Recieved (short) header!\n");
        printf("  signal: 0x%02X\n", plcp_hdrp.signal);
        printf("  service: 0x%02X\n", plcp_hdrp.service);
        printf("  length: 0x%04X\n", plcp_hdrp.length);
        printf("  crc: 0x%04X\n", plcp_hdrp.crc);
        printf("Calculated crc: 0x%04X\n", crc);
#endif

        if(plcp_hdrp.crc != crc) {
          d_state = PLCP_STATE_SEARCH_PREAMBLE;
          d_rate = PLCP_RATE_1MBPS;
          d_shift = 0;
          break;
        }

        if(plcp_hdrp.signal == 0x14) {
          /* 2 Mbps */
          d_pdu_len = plcp_hdrp.length >> 2;
        } else {
          /* Rate is not 1 Mpbs or 2 Mbps */
          d_state = PLCP_STATE_SEARCH_PREAMBLE;
          d_rate = PLCP_RATE_1MBPS;
          d_shift = 0;
#if PLCP_DEBUG
          printf("Unsupported Short Pramble Rate = %2.1f Mbps\n",
                 (float)(plcp_hdrp.signal) / 10.0);
#endif
          break;
        }

#if PLCP_DEBUG
        printf("Packet Length: %d bytes.\n", d_pdu_len);
#endif    

        if(d_pdu_len > MAX_PDU_LENGTH) {
#if PLCP_DEBUG
          printf("Ignoring packet because it is too long\n");
#endif
          d_state = PLCP_STATE_SEARCH_PREAMBLE;
          d_rate = PLCP_RATE_1MBPS;
          d_shift = 0;
          break;
        }

        d_state = PLCP_STATE_PDU;
        d_byte_count = 0;
        d_packet_rate = plcp_hdrp.signal;

        symbol_delta = (((unsigned long)iptr1 - (unsigned long)input_items[0])
                        << 3);
        d_packet_rx_time = d_symbol_count + (long long)symbol_delta;

      }
      break;
    case PLCP_STATE_PDU:
      if(d_rate == PLCP_RATE_1MBPS) {
#if PLCP_DEBUG
        printf("Read byte %d\n", d_byte_count);
#endif
        d_pkt_data[d_byte_count] = REVERSE_BITS(descrambled_byte);
        ++d_byte_count;
      } else {
        d_pkt_data[d_byte_count] = REVERSE_BITS(descrambled_byte2[0]);
        ++d_byte_count;

        d_pkt_data[d_byte_count] = REVERSE_BITS(descrambled_byte2[1]);
        ++d_byte_count;
      }

      if(d_byte_count >= d_pdu_len) {
        if(d_check_crc) {
          if(bbn_crc32_le(d_pkt_data, d_pdu_len) != 0x2144df1c) {
            /* Payload crc check failed */
            d_state = PLCP_STATE_SEARCH_PREAMBLE;
            d_shift = 0;
            d_rate = PLCP_RATE_1MBPS;
            break;
          }
          d_pdu_len -= 4; /* Strip the crc from the payload */
        }
        gr::message::sptr msg = gr::message::make(0, 0, 0, 
                                              d_pdu_len + sizeof(*oob));

        memcpy(msg->msg() + sizeof(*oob), d_pkt_data, d_pdu_len);
        oob = (oob_hdr_t *)msg->msg();
        oob->timestamp = d_packet_rx_time; /* Relative time in microseconds */
	oob->length = d_pdu_len;
        oob->rssi = (char)(*iptr1 >> 8); /* dB Scale */
        oob->rate = d_packet_rate; /* Receive rate in units of 100 kBps */

        pmt::pmt_t vecpmt(pmt::make_blob(msg->msg(), msg->length()));
        pmt::pmt_t pdu(pmt::cons(pmt::PMT_NIL, vecpmt));
        message_port_pub(PDU_PORT_ID, pdu);

        d_target_queue->insert_tail(msg);               // send it
        msg.reset();                            // free it up

        d_state = PLCP_STATE_SEARCH_PREAMBLE;
        d_shift = 0;
        d_rate = PLCP_RATE_1MBPS;
      }
      break;
    }


    // emit next stop marker
    // (an empty message is inserted into the output
    //  so the receiver can synchronize with the gr block).
    if (!last_item_offsets.empty() && last_item_offsets.front() == abs_N+i) // last byte processed?
    {
        gr::message::sptr msg = gr::message::make(0, 0, 0, 0);
        d_target_queue->insert_tail(msg);               // send it
        msg.reset();                            // free it up

        last_item_offsets.pop();
    }

    ++iptr1;
    ++iptr2;
  }

  consume_each (noutput_items);
  d_symbol_count += (noutput_items << 3);

#if PLCP_DEBUG
  printf("PLCP produced: %d\n", num_output);
#endif

  return num_output;
}
