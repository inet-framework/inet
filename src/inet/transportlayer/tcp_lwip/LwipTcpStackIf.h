//
// Copyright (C) 2010 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_LWIPTCPSTACKIF_H
#define __INET_LWIPTCPSTACKIF_H

#include "inet/networklayer/common/L3Address.h"
#include "lwip/lwip_tcp.h"
#include "lwip/opt.h"
#include "lwip/pbuf.h"

namespace inet {
namespace tcp {

/**
 * Interface class between TcpLwip and LwipTcpLayer
 */
class LwipTcpStackIf
{
  public:
    virtual ~LwipTcpStackIf() {}

    /**
     * TCP layer send a packet to IP layer
     * @param pcb:    the lwip pcb or nullptr (tipically when send a RESET )
     * @param src:    the source IP addr
     * @param dest:   the destination IP addr
     * @param tcpseg: pointer to TCP segment (message)
     * @param len:    length of tcpseg
     */
    virtual void ip_output(LwipTcpLayer::tcp_pcb *pcb,
            L3Address const& src, L3Address const& dest, void *tcpseg, int len) = 0;

    /**
     * TCP layer events
     */
    virtual err_t lwip_tcp_event(void *arg, LwipTcpLayer::tcp_pcb *pcb,
            LwipTcpLayer::lwip_event, struct pbuf *p, u16_t size, err_t err) = 0;

    /**
     * TCP layer event
     * called before LWIP freeing a pcb.
     * @param pcb: pointer to pcb
     */
    virtual void lwip_free_pcb_event(LwipTcpLayer::tcp_pcb *pcb) = 0;

    /**
     * Get the network interface
     */
    virtual netif *ip_route(L3Address const& ipAddr) = 0;

    virtual void notifyAboutIncomingSegmentProcessing(LwipTcpLayer::tcp_pcb *pcb, uint32_t seqNo,
            const void *dataptr, int len) = 0;
};

} // namespace tcp
} // namespace inet

#endif

