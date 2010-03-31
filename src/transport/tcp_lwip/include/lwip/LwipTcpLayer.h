/*
 * Author: Zoltan Bojthe
 */

#ifndef __INET_LWIPTCPLAYER
#define __INET_LWIPTCPLAYER

#include "lwip/opt.h"

#if LWIP_TCP /* don't build if not configured for use in lwipopts.h */

#if 0
class LwipTcpLayer
{
    #ifndef TCP_LOCAL_PORT_RANGE_START
    #define TCP_LOCAL_PORT_RANGE_START 4096
    #define TCP_LOCAL_PORT_RANGE_END   0x7fff
    #endif

  protected:
    /* Incremented every coarse grained timer shot (typically every 500 ms). */
    u32_t tcp_ticks;

    /* static global */
    u8_t tcp_timer;

    /* The TCP PCB lists. */

    /** List of all TCP PCBs bound but not yet (connected || listening) */
    struct tcp_pcb *tcp_bound_pcbs;
    /** List of all TCP PCBs in LISTEN state */
    union tcp_listen_pcbs_t tcp_listen_pcbs;
    /** List of all TCP PCBs that are in a state in which
     * they accept or send data. */
    struct tcp_pcb *tcp_active_pcbs;
    /** List of all TCP PCBs in TIME-WAIT state */
    struct tcp_pcb *tcp_tw_pcbs;

    /** is it never used???  */
    struct tcp_pcb *tcp_tmp_pcb;

    /* static for functions */
    /* u16_t tcp_new_port(void) */
    u16_t port;

    /* u32_t tcp_next_iss(void) */
    u32_t iss;

    struct tcp_seg inseg;
    struct tcp_hdr* tcphdr;
    struct ip_hdr* iphdr;
    u32_t seqno;
    u32_t ackno;
    u8_t flags;
    u16_t tcplen;

    u8_t recv_flags;
    struct pbuf* recv_data;

    struct tcp_pcb* tcp_input_pcb;

  public:
    /** Constructor */
    LwipTcpLayer();

  public:
    /**
     * Default accept callback if no accept callback is specified by the user.
     */
    err_t
    tcp_accept_null(void *arg, struct tcp_pcb *pcb, err_t err);

  protected:
    /**
     * A nastly hack featuring 'goto' statements that allocates a
     * new TCP local port.
     *
     * @return a new (free) local TCP port number
     */
    u16_t
    tcp_new_port();

  public:
    /**
     * Called periodically to dispatch TCP timers.
     *
     */
    void tcp_tmr(void);

    /**
     * Closes the connection held by the PCB.
     *
     * Listening pcbs are freed and may not be referenced any more.
     * Connection pcbs are freed if not yet connected and may not be referenced
     * any more. If a connection is established (at least SYN received or in
     * a closing state), the connection is closed, and put in a closing state.
     * The pcb is then automatically freed in tcp_slowtmr(). It is therefore
     * unsafe to reference it.
     *
     * @param pcb the tcp_pcb to close
     * @return ERR_OK if connection has been closed
     *         another err_t if closing failed and pcb is not freed
     */
    err_t
    tcp_close(struct tcp_pcb *pcb);

    /**
     * Abandons a connection and optionally sends a RST to the remote
     * host.  Deletes the local protocol control block. This is done when
     * a connection is killed because of shortage of memory.
     *
     * @param pcb the tcp_pcb to abort
     * @param reset boolean to indicate whether a reset should be sent
     */
    void
    tcp_abandon(struct tcp_pcb *pcb, int reset);

    /**
     * Binds the connection to a local portnumber and IP address. If the
     * IP address is not given (i.e., ipaddr == NULL), the IP address of
     * the outgoing network interface is used instead.
     *
     * @param pcb the tcp_pcb to bind (no check is done whether this pcb is
     *        already bound!)
     * @param ipaddr the local ip address to bind to (use IP_ADDR_ANY to bind
     *        to any local address
     * @param port the local port to bind to
     * @return ERR_USE if the port is already in use
     *         ERR_OK if bound
     */
    err_t
    tcp_bind(struct tcp_pcb *pcb, struct ip_addr *ipaddr, u16_t port);

    /**
     * Set the state of the connection to be LISTEN, which means that it
     * is able to accept incoming connections. The protocol control block
     * is reallocated in order to consume less memory. Setting the
     * connection to LISTEN is an irreversible process.
     *
     * @param pcb the original tcp_pcb
     * @param backlog the incoming connections queue limit
     * @return tcp_pcb used for listening, consumes less memory.
     *
     * @note The original tcp_pcb is freed. This function therefore has to be
     *       called like this:
     *             tpcb = tcp_listen(tpcb);
     */
    struct tcp_pcb *
    tcp_listen_with_backlog(struct tcp_pcb *pcb, u8_t backlog);

    /**
     * Update the state that tracks the available window space to advertise.
     *
     * Returns how much extra window would be advertised if we sent an
     * update now.
     */
    u32_t
    tcp_update_rcv_ann_wnd(struct tcp_pcb *pcb);

    /**
     * This function should be called by the application when it has
     * processed the data. The purpose is to advertise a larger window
     * when the data has been processed.
     *
     * @param pcb the tcp_pcb for which data is read
     * @param len the amount of bytes that have been read by the application
     */
    void
    tcp_recved(struct tcp_pcb *pcb, u16_t len);

    /**
     * Connects to another host. The function given as the "connected"
     * argument will be called when the connection has been established.
     *
     * @param pcb the tcp_pcb used to establish the connection
     * @param ipaddr the remote ip address to connect to
     * @param port the remote tcp port to connect to
     * @param connected callback function to call when connected (or on error)
     * @return ERR_VAL if invalid arguments are given
     *         ERR_OK if connect request has been sent
     *         other err_t values if connect request couldn't be sent
     */
    err_t
    tcp_connect(struct tcp_pcb *pcb, struct ip_addr *ipaddr, u16_t port,
          err_t (* connected)(void *arg, struct tcp_pcb *tpcb, err_t err));

    /**
     * Called every 500 ms and implements the retransmission timer and the timer that
     * removes PCBs that have been in TIME-WAIT for enough time. It also increments
     * various timers such as the inactivity timer in each PCB.
     *
     * Automatically called from tcp_tmr().
     */
    void
    tcp_slowtmr(void);

    /**
     * Is called every TCP_FAST_INTERVAL (250 ms) and process data previously
     * "refused" by upper layer (application) and sends delayed ACKs.
     *
     * Automatically called from tcp_tmr().
     */
    void
    tcp_fasttmr(void);

    /**
     * Deallocates a list of TCP segments (tcp_seg structures).
     *
     * @param seg tcp_seg list of TCP segments to free
     * @return the number of pbufs that were deallocated
     */
    u8_t
    tcp_segs_free(struct tcp_seg *seg);

    /**
     * Frees a TCP segment (tcp_seg structure).
     *
     * @param seg single tcp_seg to free
     * @return the number of pbufs that were deallocated
     */
    u8_t
    tcp_seg_free(struct tcp_seg *seg);

    /**
     * Sets the priority of a connection.
     *
     * @param pcb the tcp_pcb to manipulate
     * @param prio new priority
     */
    void
    tcp_setprio(struct tcp_pcb *pcb, u8_t prio);

    /**
     * Returns a copy of the given TCP segment.
     * The pbuf and data are not copied, only the pointers
     *
     * @param seg the old tcp_seg
     * @return a copy of seg
     */
    struct tcp_seg *
    tcp_seg_copy(struct tcp_seg *seg);

    /**
     * Default receive callback that is called if the user didn't register
     * a recv callback for the pcb.
     */
    err_t
    tcp_recv_null(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err);

    /**
     * Kills the oldest active connection that has lower priority than prio.
     *
     * @param prio minimum priority
     */
    void
    tcp_kill_prio(u8_t prio);

    /**
     * Kills the oldest connection that is in TIME_WAIT state.
     * Called from tcp_alloc() if no more connections are available.
     */
    void
    tcp_kill_timewait(void);

    /**
     * Allocate a new tcp_pcb structure.
     *
     * @param prio priority for the new pcb
     * @return a new tcp_pcb that initially is in state CLOSED
     */
    struct tcp_pcb *
    tcp_alloc(u8_t prio);

    /**
     * Creates a new TCP protocol control block but doesn't place it on
     * any of the TCP PCB lists.
     * The pcb is not put on any list until binding using tcp_bind().
     *
     * @internal: Maybe there should be a idle TCP PCB list where these
     * PCBs are put on. Port reservation using tcp_bind() is implemented but
     * allocated pcbs that are not bound can't be killed automatically if wanting
     * to allocate a pcb with higher prio (@see tcp_kill_prio())
     *
     * @return a new tcp_pcb that initially is in state CLOSED
     */
    struct tcp_pcb *
    tcp_new(void);

    /**
     * Used to specify the argument that should be passed callback
     * functions.
     *
     * @param pcb tcp_pcb to set the callback argument
     * @param arg void pointer argument to pass to callback functions
     */
    void
    tcp_arg(struct tcp_pcb *pcb, void *arg);

    /**
     * Used to specify the function that should be called when a TCP
     * connection receives data.
     *
     * @param pcb tcp_pcb to set the recv callback
     * @param recv callback function to call for this pcb when data is received
     */
    void
    tcp_recv(struct tcp_pcb *pcb,
       err_t (* recv)(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err));

    /**
     * Used to specify the function that should be called when TCP data
     * has been successfully delivered to the remote host.
     *
     * @param pcb tcp_pcb to set the sent callback
     * @param sent callback function to call for this pcb when data is successfully sent
     */
    void
    tcp_sent(struct tcp_pcb *pcb,
       err_t (* sent)(void *arg, struct tcp_pcb *tpcb, u16_t len));

    /**
     * Used to specify the function that should be called when a fatal error
     * has occured on the connection.
     *
     * @param pcb tcp_pcb to set the err callback
     * @param errf callback function to call for this pcb when a fatal error
     *        has occured on the connection
     */
    void
    tcp_err(struct tcp_pcb *pcb,
       void (* errf)(void *arg, err_t err));

    /**
     * Used for specifying the function that should be called when a
     * LISTENing connection has been connected to another host.
     *
     * @param pcb tcp_pcb to set the accept callback
     * @param accept callback function to call for this pcb when LISTENing
     *        connection has been connected to another host
     */
    void
    tcp_accept(struct tcp_pcb *pcb,
         err_t (* accept)(void *arg, struct tcp_pcb *newpcb, err_t err));

    /**
     * Used to specify the function that should be called periodically
     * from TCP. The interval is specified in terms of the TCP coarse
     * timer interval, which is called twice a second.
     *
     */
    void
    tcp_poll(struct tcp_pcb *pcb,
       err_t (* poll)(void *arg, struct tcp_pcb *tpcb), u8_t interval);

    /**
     * Purges a TCP PCB. Removes any buffered data and frees the buffer memory
     * (pcb->ooseq, pcb->unsent and pcb->unacked are freed).
     *
     * @param pcb tcp_pcb to purge. The pcb itself is not deallocated!
     */
    void
    tcp_pcb_purge(struct tcp_pcb *pcb);

    /**
     * Purges the PCB and removes it from a PCB list. Any delayed ACKs are sent first.
     *
     * @param pcblist PCB list to purge.
     * @param pcb tcp_pcb to purge. The pcb itself is NOT deallocated!
     */
    void
    tcp_pcb_remove(struct tcp_pcb **pcblist, struct tcp_pcb *pcb);

    /**
     * Calculates a new initial sequence number for new connections.
     *
     * @return u32_t pseudo random sequence number
     */
    u32_t
    tcp_next_iss(void);

    /**
     * Calcluates the effective send mss that can be used for a specific IP address
     * by using ip_route to determin the netif used to send to the address and
     * calculating the minimum of TCP_MSS and that netif's mtu (if set).
     */
    u16_t
    tcp_eff_send_mss(u16_t sendmss, struct ip_addr *addr);

    const char*
    tcp_debug_state_str(enum tcp_state s);

#if TCP_DEBUG || TCP_INPUT_DEBUG || TCP_OUTPUT_DEBUG
    /**
     * Print a tcp header for debugging purposes.
     *
     * @param tcphdr pointer to a struct tcp_hdr
     */
    void
    tcp_debug_print(struct tcp_hdr *tcphdr);

    /**
     * Print a tcp state for debugging purposes.
     *
     * @param s enum tcp_state to print
     */
    void
    tcp_debug_print_state(enum tcp_state s);

    /**
     * Print tcp flags for debugging purposes.
     *
     * @param flags tcp flags, all active flags are printed
     */
    void
    tcp_debug_print_flags(u8_t flags);

    /**
     * Print all tcp_pcbs in every list for debugging purposes.
     */
    void
    tcp_debug_print_pcbs();

    /**
     * Check state consistency of the tcp_pcb lists.
     */
    s16_t
    tcp_pcbs_sane();
#endif

    // from tcp_in:

  public:
    /**
     * The initial input processing of TCP. It verifies the TCP header, demultiplexes
     * the segment between the PCBs and passes it on to tcp_process(), which implements
     * the TCP finite state machine. This function is called by the IP layer (in
     * ip_input()).
     *
     * @param p received TCP segment to process (p->payload pointing to the IP header)
     * @param inp network interface on which this segment was received
     */
    void
    tcp_input(struct pbuf *p, struct netif *inp);

    /**
     * Called by tcp_input() when a segment arrives for a listening
     * connection (from tcp_input()).
     *
     * @param pcb the tcp_pcb_listen for which a segment arrived
     * @return ERR_OK if the segment was processed
     *         another err_t on error
     *
     * @note the return value is not (yet?) used in tcp_input()
     * @note the segment which arrived is saved in global variables, therefore only the pcb
     *       involved is passed as a parameter to this function
     */
  protected:
    err_t
    tcp_listen_input(struct tcp_pcb_listen *pcb);

    /**
     * Called by tcp_input() when a segment arrives for a connection in
     * TIME_WAIT.
     *
     * @param pcb the tcp_pcb for which a segment arrived
     *
     * @note the segment which arrived is saved in global variables, therefore only the pcb
     *       involved is passed as a parameter to this function
     */
  protected:
    err_t
    tcp_timewait_input(struct tcp_pcb *pcb);

    /**
     * Implements the TCP state machine. Called by tcp_input. In some
     * states tcp_receive() is called to receive data. The tcp_seg
     * argument will be freed by the caller (tcp_input()) unless the
     * recv_data pointer in the pcb is set.
     *
     * @param pcb the tcp_pcb for which a segment arrived
     *
     * @note the segment which arrived is saved in global variables, therefore only the pcb
     *       involved is passed as a parameter to this function
     */
  protected:
    err_t
    tcp_process(struct tcp_pcb *pcb);

    /**
     * Insert segment into the list (segments covered with new one will be deleted)
     *
     * Called from tcp_receive()
     */
  protected:
    void
    tcp_oos_insert_segment(struct tcp_seg *cseg, struct tcp_seg *next);

    /**
     * Called by tcp_process. Checks if the given segment is an ACK for outstanding
     * data, and if so frees the memory of the buffered data. Next, is places the
     * segment on any of the receive queues (pcb->recved or pcb->ooseq). If the segment
     * is buffered, the pbuf is referenced by pbuf_ref so that it will not be freed until
     * i it has been removed from the buffer.
     *
     * If the incoming segment constitutes an ACK for a segment that was used for RTT
     * estimation, the RTT is estimated here as well.
     *
     * Called from tcp_process().
     */
  protected:
    void
    tcp_receive(struct tcp_pcb *pcb);

    /**
     * Parses the options contained in the incoming segment.
     *
     * Called from tcp_listen_input() and tcp_process().
     * Currently, only the MSS option is supported!
     *
     * @param pcb the tcp_pcb for which a segment arrived
     */
  protected:
    void
    tcp_parseopt(struct tcp_pcb *pcb);

    // from tcp_out:

  protected:
    struct tcp_hdr *
    tcp_output_set_header(struct tcp_pcb *pcb, struct pbuf *p, int optlen,
                          u32_t seqno_be /* already in network byte order */);


    /**
     * Called by tcp_close() to send a segment including flags but not data.
     *
     * @param pcb the tcp_pcb over which to send a segment
     * @param flags the flags to set in the segment header
     * @return ERR_OK if sent, another err_t otherwise
     */
  public:
    err_t
    tcp_send_ctrl(struct tcp_pcb *pcb, u8_t flags);

    /**
     * Write data for sending (but does not send it immediately).
     *
     * It waits in the expectation of more data being sent soon (as
     * it can send them more efficiently by combining them together).
     * To prompt the system to send data now, call tcp_output() after
     * calling tcp_write().
     *
     * @param pcb Protocol control block of the TCP connection to enqueue data for.
     * @param data pointer to the data to send
     * @param len length (in bytes) of the data to send
     * @param apiflags combination of following flags :
     * - TCP_WRITE_FLAG_COPY (0x01) data will be copied into memory belonging to the stack
     * - TCP_WRITE_FLAG_MORE (0x02) for TCP connection, PSH flag will be set on last segment sent,
     * @return ERR_OK if enqueued, another err_t on error
     *
     * @see tcp_write()
     */
  public:
    err_t
    tcp_write(struct tcp_pcb *pcb, const void *data, u16_t len, u8_t apiflags);

    /**
     * Enqueue data and/or TCP options for transmission
     *
     * Called by tcp_connect(), tcp_listen_input(), tcp_send_ctrl() and tcp_write().
     *
     * @param pcb Protocol control block for the TCP connection to enqueue data for.
     * @param arg Pointer to the data to be enqueued for sending.
     * @param len Data length in bytes
     * @param flags tcp header flags to set in the outgoing segment
     * @param apiflags combination of following flags :
     * - TCP_WRITE_FLAG_COPY (0x01) data will be copied into memory belonging to the stack
     * - TCP_WRITE_FLAG_MORE (0x02) for TCP connection, PSH flag will be set on last segment sent,
     * @param optflags options to include in segment later on (see definition of struct tcp_seg)
     */
  public:
    err_t
    tcp_enqueue(struct tcp_pcb *pcb, void *arg, u16_t len,
                u8_t flags, u8_t apiflags, u8_t optflags);

#if LWIP_TCP_TIMESTAMPS
    /* Build a timestamp option (12 bytes long) at the specified options pointer)
     *
     * @param pcb tcp_pcb
     * @param opts option pointer where to store the timestamp option
     */
  protected:
    void
    tcp_build_timestamp_option(struct tcp_pcb *pcb, u32_t *opts);
#endif

    /** Send an ACK without data.
     *
     * @param pcb Protocol control block for the TCP connection to send the ACK
     */
  public:
    err_t
    tcp_send_empty_ack(struct tcp_pcb *pcb);


    /**
     * Find out what we can send and send it
     *
     * @param pcb Protocol control block for the TCP connection to send data
     * @return ERR_OK if data has been sent or nothing to send
     *         another err_t on error
     */
  public:
    err_t
    tcp_output(struct tcp_pcb *pcb);

    /**
     * Called by tcp_output() to actually send a TCP segment over IP.
     *
     * @param seg the tcp_seg to send
     * @param pcb the tcp_pcb for the TCP connection used to send the segment
     */
  protected:
    void
    tcp_output_segment(struct tcp_seg *seg, struct tcp_pcb *pcb);

    /**
     * Send a TCP RESET packet (empty segment with RST flag set) either to
     * abort a connection or to show that there is no matching local connection
     * for a received segment.
     *
     * Called by tcp_abort() (to abort a local connection), tcp_input() (if no
     * matching local pcb was found), tcp_listen_input() (if incoming segment
     * has ACK flag set) and tcp_process() (received segment in the wrong state)
     *
     * Since a RST segment is in most cases not sent for an active connection,
     * tcp_rst() has a number of arguments that are taken from a tcp_pcb for
     * most other segment output functions.
     *
     * @param seqno the sequence number to use for the outgoing segment
     * @param ackno the acknowledge number to use for the outgoing segment
     * @param local_ip the local IP address to send the segment from
     * @param remote_ip the remote IP address to send the segment to
     * @param local_port the local TCP port to send the segment from
     * @param remote_port the remote TCP port to send the segment to
     */
  public:
    void
    tcp_rst(u32_t seqno, u32_t ackno,
      struct ip_addr *local_ip, struct ip_addr *remote_ip,
      u16_t local_port, u16_t remote_port);

    /**
     * Requeue all unacked segments for retransmission
     *
     * Called by tcp_slowtmr() for slow retransmission.
     *
     * @param pcb the tcp_pcb for which to re-enqueue all unacked segments
     */
  public:
    void
    tcp_rexmit_rto(struct tcp_pcb *pcb);

    /**
     * Requeue the first unacked segment for retransmission
     *
     * Called by tcp_receive() for fast retramsmit.
     *
     * @param pcb the tcp_pcb for which to retransmit the first unacked segment
     */
  public:
    void
    tcp_rexmit(struct tcp_pcb *pcb);

    /**
     * Handle retransmission after three dupacks received
     *
     * @param pcb the tcp_pcb for which to retransmit the first unacked segment
     */
  public:
    void
    tcp_rexmit_fast(struct tcp_pcb *pcb);

    /**
     * Send keepalive packets to keep a connection active although
     * no data is sent over it.
     *
     * Called by tcp_slowtmr()
     *
     * @param pcb the tcp_pcb for which to send a keepalive packet
     */
  public:
    void
    tcp_keepalive(struct tcp_pcb *pcb);

    /**
     * Send persist timer zero-window probes to keep a connection active
     * when a window update is lost.
     *
     * Called by tcp_slowtmr()
     *
     * @param pcb the tcp_pcb for which to send a zero-window probe packet
     */
  public:
    void
    tcp_zero_window_probe(struct tcp_pcb *pcb);
};

#endif /* 0 */

#endif /* LWIP_TCP */

#endif /* __INET_LWIPTCPLAYER */
