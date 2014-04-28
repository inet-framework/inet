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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdexcept>
#include <cstdio>
#include <cstring>

extern "C" {
#ifdef __linux__
#include <linux/if_tun.h>
#endif
}

#include <bbn_tap.h>

bbn_tap_sptr bbn_make_tap(std::string dev_name, int freq) {
  return bbn_tap_sptr (new bbn_tap(dev_name, freq));
}


bbn_tap::bbn_tap(std::string dev_name, int freq) {
#ifdef __linux__
  struct ifreq ifr;

  d_tap_fd = open("/dev/net/tun", O_RDWR);
  if(d_tap_fd < 0) {
    perror("bbn_tap: open");
    return;
  }

  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
  strncpy(ifr.ifr_name, dev_name.data(), IFNAMSIZ);
  ifr.ifr_name[IFNAMSIZ - 1] = '\0';

  if(ioctl(d_tap_fd, TUNSETIFF, &ifr) != 0) {
    perror("bbn_tap: ioctl");
    close(d_tap_fd);
    d_name[0] = '\0';
    d_tap_fd = -1;
    return;
  }

  strncpy(d_name, ifr.ifr_name, IFNAMSIZ);
  printf("bbn_tap: created tap device %s\n", d_name);
#endif

#ifdef HAVE_BSD_TAP_RADIOTAP
  struct ifreq ifr;
  struct tapinfo info;
  struct tap_rx_radiotap_header *ra;

  d_tap_fd = open("/dev/tap", O_RDWR);
  if(d_tap_fd < 0) {
    perror("bbn_tap: open");
    return;
  }

  info.type = TAP_TYPE_BROADCAST; 
  info.eflags = 0; 
  info.speedtype=TAP_SPEED_SHARED;
  info.speed=1000000; 
  info.mtu = 1500;      
  info.macformat=TAP_MAC_IEEE80211_RADIO;

  if(ioctl(d_tap_fd, TAPSIFINFO, &info)) {
    perror("bbn_tap: TAPSIFINFO ioctl");
    return;    
  }
  
  memset(&ifr, 0, sizeof(ifr));
  if(ioctl(d_tap_fd, TAPGIFNAME, &ifr) != 0) {
    perror("bbn_tap: TAPGIFNAME ioctl");
    return;
  }

  strncpy(d_name, ifr.ifr_name, IFNAMSIZ);
  printf("bbn_tap: created tap device %s\n", d_name);

  ra = (struct tap_rx_radiotap_header *)d_pkt_data;
  ra->wr_ihdr.it_version = 0;
  ra->wr_ihdr.it_present = TAP_RX_RADIOTAP_PRESENT;
  ra->wr_ihdr.it_len = sizeof(struct tap_rx_radiotap_header);
  ra->wr_flags = 0;
  ra->wr_chan_flags = 0;
  ra->wr_chan_freq = freq;
  printf("bbn_tap: freq = %d\n", freq);
  ra->wr_antenna = 0;
  ra->wr_antnoise = 0;
#endif
}

bbn_tap::~bbn_tap() {
  if(d_tap_fd < 0) {
    return;
  }

  printf("bbn_tap: destroying tap device %s\n", d_name);

  close(d_tap_fd);
  d_tap_fd = -1;
}

int bbn_tap::tap_write(const std::string buf) {
  oob_hdr_t *oob;
#ifdef HAVE_BSD_TAP_RADIOTAP
  struct tap_rx_radiotap_header *ra;
#endif

  if(d_tap_fd == -1) {
    fprintf(stderr, "bbn_tap: tap_write: tap is not opened.\n");
    return -1;
  }

  if(buf.size() < sizeof(oob_hdr_t)) {
    fprintf(stderr, "bbn_tap: tap_write: buffer length (%d) is too small\n",
	    buf.size());
    return -1;
  }

  oob = (oob_hdr_t *)buf.data();
  if(oob->length != (buf.size() - sizeof(oob_hdr_t))) {
    fprintf(stderr, "bbn_tap: tap_write: buffer size mismatch\n");
    return -1;
  }

#ifdef HAVE_BSD_TAP_RADIOTAP
  if(oob->length > (2312 + sizeof(struct tap_rx_radiotap_header))) {
#else
  if(oob->length > 2312 ) {
#endif
    fprintf(stderr, "bbn_tap: tap_write: buffer length (%d) is too large\n",
	    buf.size());
    return -1;
  }

#ifdef __linux__
  write(d_tap_fd, oob + 1, oob->length);
#endif

#ifdef  HAVE_BSD_TAP_RADIOTAP
  ra = (struct tap_rx_radiotap_header *)d_pkt_data;

  ra->wr_rate = oob->rate / 5;
  ra->wr_antsignal = oob->rssi - (-80);
  ra->wr_tsf = oob->timestamp;
  memcpy(ra + 1, oob + 1, oob->length);
  write(d_tap_fd, ra, oob->length + sizeof(struct tap_rx_radiotap_header));
#endif

  return 0;
}

int bbn_tap::tap_read_fd() {
  if(d_tap_fd == -1) {
    return -1;
  }

#ifdef HAVE_BSD_TAP_RADIOTAP
  /* NetBSD Tap driver does not support read yet, so just block on
     standard input. */
  return d_tap_fd;
#endif

  return d_tap_fd;
}

#if 0
std::string bbn_tap::tap_read() {
  char buf[2400];
  int pktLen;

#ifdef HAVE_BSD_TAP_RADIOTAP
  struct tap_tx_radiotap_header *th;

  th = (struct tap_tx_radiotap_header *)buf;

  pktLen = read(d_tap_fd, buf, sizeof(buf));

  if(read(d_tap_fd, buf, sizeof(buf)) < 
     (int)sizeof(struct tap_tx_radiotap_header)) {
    if(pktLen < sizeof(tap_tx_radiotap_header)) {
      fprintf(stderr, "bbn_tap: tap_read: Length (%d) too small\n",
	      pktLen);
    }
    return std::string("");
  }

  pktLen -= (int)sizeof(tap_tx_radiotap_header);

  return std::string((char *)(th + 1), pktLen);
#else
  if(read(d_tap_fd, buf, sizeof(buf)) < 0) {
    return std::string("");
  }

  return std::string(buf);

#endif
}
#endif

std::string bbn_tap::tap_process_tx(std::string buf) {
#ifdef HAVE_BSD_TAP_RADIOTAP
  struct tap_tx_radiotap_header *th;

  th = (struct tap_tx_radiotap_header *)buf.data();

  if(buf.size() < (int)sizeof(*th)) {
    fprintf(stderr, "bbn_tap: tap_read: Length (%d) too small\n",
	    buf.size());
    return std::string("");
  }

  /* Sanity check.  The channel seems to always be set to 1 */
  if(th->wt_chan_freq != 2412) {
    return std::string("");
  }

  return std::string((char *)(th + 1),
		     buf.size() - (int)sizeof(*th));
#else
  return buf;
#endif
}
