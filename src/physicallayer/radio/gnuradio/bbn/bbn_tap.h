/* -*- c++ -*- */
/*
 * Copyright 2005 Free Software Foundation, Inc.
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
#ifndef INCLUDED_BBN_TAP_H
#define INCLUDED_BBN_TAP_H

extern "C" {
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <net/if.h>
#ifdef __NetBSD__
#include <net/if_tap.h>
#include <net80211/ieee80211_radiotap.h>
#endif
}

#include <boost/shared_ptr.hpp>
#include <gnuradio/msg_handler.h>
//#include <omnithread.h>

typedef struct oob_hdr_struct {
  long long timestamp; /* Relative time in microseconds */
  unsigned short length;
  char rssi; /* dB Scale */
  char rate; /* Receive rate in units of 100 kBps */
} __attribute__(( __packed__)) oob_hdr_t;

class bbn_tap;
typedef boost::shared_ptr<bbn_tap> bbn_tap_sptr;

bbn_tap_sptr bbn_make_tap(std::string dev_name, int freq=2437);

/*!
 * \brief thread-safe message queue
 */
class bbn_tap {
private:
  int d_tap_fd;
#if defined(__NetBSD__) && defined(TAP_TX_RADIOTAP_PRESENT)
#define HAVE_BSD_TAP_RADIOTAP
  char d_pkt_data[2312 + sizeof(struct tap_rx_radiotap_header)];
#else
  char d_pkt_data[2312];
#endif
  char d_name[IFNAMSIZ];

public:
  bbn_tap(std::string dev_name, int freq);
  ~bbn_tap();
  int fd() { return d_tap_fd; };
  std::string name() { return std::string(d_name); };

  int tap_write(const std::string buf);
  std::string tap_process_tx(std::string buf);
  int tap_read_fd();
};

#endif /* INCLUDED_BBN_TAP_H */
