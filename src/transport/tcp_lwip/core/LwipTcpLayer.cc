

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/snmp.h"
#include "lwip/tcp.h"
#include "lwip/debug.h"
#include "lwip/stats.h"

#include <stddef.h>
#include <string.h>

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

    tcp_input(p, NULL/*interface*/);
}
