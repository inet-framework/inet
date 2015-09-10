/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef _SEND_BUF_H
#define _SEND_BUF_H

#include "inet/routing/extras/dsr/dsr-uu/dsr.h"

#ifndef NO_GLOBALS

#define SEND_BUF_DROP 1
#define SEND_BUF_SEND 2

#ifndef OMNETPP
#ifdef NS2
#include "inet/routing/extras/dsr/dsr-uu/ns-agent.h"
typedef void (DSRUU::*xmit_fct_t) (struct dsr_pkt *);
#else
typedef int (*xmit_fct_t) (struct dsr_pkt *);
#endif
#else
#include "inet/routing/extras/dsr/dsr-uu-omnetpp.h"

namespace inet {

namespace inetmanet {

typedef void (DSRUU::*xmit_fct_t) (struct dsr_pkt *);

} // namespace inetmanet

} // namespace inet

#endif /* OMNETPP */

#endif              /* NO_GLOBALS */

#ifndef NO_DECLS


#ifdef OMNETPP
struct send_buf_entry * send_buf_entry_create(struct dsr_pkt *dp,xmit_fct_t okfn);
#endif

void send_buf_set_max_len(unsigned int max_len);
int send_buf_find(struct in_addr dst);
int send_buf_enqueue_packet(struct dsr_pkt *dp, xmit_fct_t okfn);
int send_buf_set_verdict(int verdict, struct in_addr dst);
int send_buf_init(void);
void send_buf_cleanup(void);
void send_buf_timeout(unsigned long data);

#endif              /* NO_DECLS */


#endif              /* _SEND_BUF_H */
