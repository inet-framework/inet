/* Copyright (C) Uppsala University
 *
 * This file is distributed under the terms of the GNU general Public
 * License (GPL), see the file LICENSE
 *
 * Author: Erik Nordström, <erikn@it.uu.se>
 */
#ifndef _DSR_ACK_H
#define _DSR_ACK_H

#include "inet/routing/extras/dsr/dsr-uu/dsr.h"


#ifndef NO_GLOBALS

namespace inet {

namespace inetmanet {

struct dsr_ack_req_opt
{
    u_int8_t type;
    u_int8_t length;
    u_int16_t id;
};

struct dsr_ack_opt
{
    u_int8_t type;
    u_int8_t length;
    u_int16_t id;
    u_int32_t src;
    u_int32_t dst;
};

#define DSR_ACK_REQ_HDR_LEN sizeof(struct dsr_ack_req_opt)
#define DSR_ACK_REQ_OPT_LEN (DSR_ACK_REQ_HDR_LEN - 2)
#define DSR_ACK_HDR_LEN sizeof(struct dsr_ack_opt)
#define DSR_ACK_OPT_LEN (DSR_ACK_HDR_LEN - 2)

int dsr_ack_add_ack_req(struct in_addr neigh);

} // namespace inetmanet

} // namespace inet

#endif              /* NO_GLOBALS */

#ifndef NO_DECLS

struct dsr_ack_req_opt *dsr_ack_req_opt_add(struct dsr_pkt *dp,
        unsigned short id);

int dsr_ack_req_opt_recv(struct dsr_pkt *dp, struct dsr_ack_req_opt *areq);
int dsr_ack_opt_recv(struct dsr_ack_opt *ack);
int dsr_ack_req_send(struct in_addr neigh_addr, unsigned short id);
int dsr_ack_send(struct in_addr neigh_addr, unsigned short id);
int dsr_ack_req_send(struct dsr_pkt *dp);

#endif              /* NO_DECLS */



#endif              /* _DSR_ACK */
