/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef _DSR_DEV_H
#define _DSR_DEV_H

#include <linux/netdevice.h>
#include <linux/init.h>

#include "inet/routing/extras/dsr/dsr-uu/dsr.h"
#include "inet/routing/extras/dsr/dsr-uu/dsr-pkt.h"

namespace inet {

namespace inetmanet {

#define DELIVER(pkt) dsr_dev_deliver(pkt)

int dsr_dev_xmit(struct dsr_pkt *dp);
int dsr_dev_deliver(struct dsr_pkt *dp);

int __init dsr_dev_init(char *ifname);
void __exit dsr_dev_cleanup(void);

} // namespace inetmanet

} // namespace inet

#endif

