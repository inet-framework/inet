/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef _DSR_RERR_H
#define _DSR_RERR_H

#include "inet/routing/extras/dsr/dsr-uu/dsr.h"

#ifdef NS2
#ifndef OMNETPP
#include "endian.h"
#endif
#endif

#ifndef NO_GLOBALS

namespace inet {

namespace inetmanet {
struct node_unreach_info
{
    u_int32_t unr_node;
};

#define NODE_UNREACHABLE          1
#define FLOW_STATE_NOT_SUPPORTED  2
#define OPTION_NOT_SUPPORTED      3

} // namespace inetmanet

} // namespace inet

#endif              /* NO_GLOBALS */

#ifndef NO_DECLS

int dsr_rerr_send(struct dsr_pkt *dp_trigg, struct in_addr unr_addr);
int dsr_rerr_opt_recv(struct dsr_pkt *dp, struct dsr_rerr_opt *dsr_rerr_opt);

#endif              /* NO_DECLS */

#endif              /* _DSR_RERR_H */
