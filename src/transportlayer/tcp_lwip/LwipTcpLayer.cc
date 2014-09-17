//
// Copyright (C) 2010 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <stddef.h>
#include <string.h>

#include "lwip/lwip_tcp.h"

#include "lwip/memp.h"

#include "inet/networklayer/common/L3Address.h"
#include "inet/transportlayer/tcp_lwip/LwipTcpStackIf.h"

namespace inet {

namespace tcp {

LwipTcpLayer::LwipTcpLayer(LwipTcpStackIf& stackIfP) :
    stackIf(stackIfP),
    tcp_input_pcb(NULL),
    tcp_ticks(0),
    tcp_active_pcbs(NULL),
    tcp_tw_pcbs(NULL),
    tcp_tmp_pcb(NULL),
    tcphdr(NULL),
    iphdr(NULL),
    seqno(0),
    ackno(0),
    flags(0),
    tcplen(0),
    recv_flags(0),
    recv_data(NULL),
    tcp_bound_pcbs(NULL),
    port(TCP_LOCAL_PORT_RANGE_START),
    iss(6510),
    tcp_timer(0)
{
    tcp_listen_pcbs.pcbs = NULL;
    memset(&inseg, 0, sizeof(inseg));
}

void LwipTcpLayer::if_receive_packet(int interfaceId, void *data, int datalen)
{
    struct pbuf *p = pbuf_alloc(PBUF_RAW, datalen, PBUF_RAM);
    memcpy(p->payload, data, datalen);

    tcp_input(p, NULL    /*interface*/);
}

/**
 * Simple interface to ip_output_if. It finds the outgoing network
 * interface and calls upon ip_output_if to do the actual work.
 *
 * @param p the packet to send (p->payload points to the data, e.g. next
            protocol header; if dest == IP_HDRINCL, p already includes an IP
            header and p->payload points to that IP header)
 * @param src the source IP address to send from (if src == IP_ADDR_ANY, the
 *         IP  address of the netif used to send is used as source address)
 * @param dest the destination IP address to send the packet to
 * @param ttl the TTL value to be set in the IP header
 * @param tos the TOS value to be set in the IP header
 * @param proto the PROTOCOL to be set in the IP header
 *
 * @return ERR_RTE if no route is found
 *         see ip_output_if() for more return values
 */
err_t LwipTcpLayer::ip_output(LwipTcpLayer::tcp_pcb *pcb, struct pbuf *p,
        struct ip_addr *src, struct ip_addr *dest,
        u8_t ttl, u8_t tos, u8_t proto)
{
    assert(proto == IP_PROTO_TCP);
    assert(p);
    assert(p->len <= p->tot_len);

    stackIf.ip_output(pcb, src->addr, dest->addr, p->payload, p->tot_len);
    return 0;
}

struct netif *LwipTcpLayer::ip_route(struct ip_addr *addr)
{
    L3Address ipAddr;

    if (addr)
        ipAddr = addr->addr;

    return stackIf.ip_route(ipAddr);
}

u8_t LwipTcpLayer::ip_addr_isbroadcast(struct ip_addr *addr, struct netif *interf)
{
    // TODO implementing this if need
    return 0;
}

err_t LwipTcpLayer::lwip_tcp_event(void *arg, LwipTcpLayer::tcp_pcb *pcb,
        LwipTcpLayer::lwip_event event, struct pbuf *p, u16_t size, err_t err)
{
    return stackIf.lwip_tcp_event(arg, pcb, event, p, size, err);
}

void LwipTcpLayer::memp_free(memp_t type, void *ptr)
{
    if ((ptr != NULL) && ((type == MEMP_TCP_PCB) || (type == MEMP_TCP_PCB_LISTEN)))
        stackIf.lwip_free_pcb_event((LwipTcpLayer::tcp_pcb *)ptr);

    inet::tcp::memp_free(type, ptr);
}

void LwipTcpLayer::notifyAboutIncomingSegmentProcessing(LwipTcpLayer::tcp_pcb *pcb, uint32_t seqNo, const void *dataptr, int len)
{
    stackIf.notifyAboutIncomingSegmentProcessing(pcb, seqNo, dataptr, len);
}

} // namespace tcp

} // namespace inet

