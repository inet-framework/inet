#include "lwip/opt.h"
#include "lwip/tcp.h"

// inet:
class IPvXAddress;

//lwip:
struct pbuf;

class LwipTcpStackIf
{
  public:
    /**
     * TCP layer send a packet to IP layer
     * @param pcb:    the lwip pcb or NULL (tipically when send a RESET )
     * @param src:    the source IP addr
     * @param dest:   the destination IP addr
     * @param tcpseg: pointer to TCP segment (message)
     * @param len:    length of tcpseg
     */
    virtual void ip_output(LwipTcpLayer::tcp_pcb *pcb,
            IPvXAddress const& src, IPvXAddress const& dest, void *tcpseg, int len) = 0;

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
    virtual void lwip_free_pcb_event(LwipTcpLayer::tcp_pcb* pcb) = 0;

    /**
     * Get the network interface
     */
    virtual netif* ip_route(IPvXAddress const & ipAddr) = 0;
};
