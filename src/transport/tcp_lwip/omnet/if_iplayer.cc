#include "lwip/ip.h"
#include "lwip/opt.h"
#include "lwip/tcp.h"
#include "lwip/memp.h"

#include "IPvXAddress.h"

#include "LwipTcpStackIf.h"

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

    IPvXAddress srcAddr, destAddr;

    srcAddr.set(ntohl(src->addr));
    destAddr.set(ntohl(dest->addr));

    stackIf.ip_output(pcb, srcAddr, destAddr, p->payload, p->len);
    return 0;
}

struct netif * LwipTcpLayer::ip_route(struct ip_addr *addr)
{
    IPvXAddress ipAddr;
    if(addr)
        ipAddr.set(ntohl(addr->addr));
    return stackIf.ip_route(ipAddr);
}

u8_t LwipTcpLayer::ip_addr_isbroadcast(struct ip_addr * addr, struct netif * interf)
{
    // TODO implementing this
    return 0;
}

err_t LwipTcpLayer::lwip_tcp_event(void *arg, LwipTcpLayer::tcp_pcb *pcb,
        LwipTcpLayer::lwip_event event, struct pbuf *p, u16_t size, err_t err)
{
    return stackIf.lwip_tcp_event(arg, pcb, event, p, size, err);
}

void LwipTcpLayer::memp_free(memp_t type, void *ptr)
{
    if ( ptr != NULL && (
        (type == MEMP_TCP_PCB)
        || (type == MEMP_TCP_PCB_LISTEN)
    ))
    {
        stackIf.lwip_free_pcb_event((LwipTcpLayer::tcp_pcb*)ptr);
    }
    ::memp_free(type, ptr);
}
