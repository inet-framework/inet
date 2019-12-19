//
// Copyright (C) 2015 Irene Ruengeler
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef __INET_PACKETDRILLUTILS_H
#define __INET_PACKETDRILLUTILS_H

#include "inet/common/INETDefs.h"
#include "omnetpp/platdep/sockets.h"

#include "inet/networklayer/common/L3Address.h"
#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
 #include <sys/socket.h>
#endif
#include "inet/transportlayer/sctp/SctpAssociation.h"
#include "inet/transportlayer/sctp/SctpHeader.h"

namespace inet {

/* For tables mapping symbolic strace strings to the corresponding
 * integer values.
 */
struct int_symbol {
    int64 value;
    const char *name;
};

#ifndef IPPROTO_SCTP
#define IPPROTO_SCTP 132
#endif

/* On Windows we don't have these macros defined (values copyed from fcntl.h) */
#ifndef F_GETFL
#define F_GETFL 3
#endif
#ifndef F_SETFL
#define F_SETFL 4
#endif

/* TCP option numbers and lengths. */
#define TCPOPT_EOL                0
#define TCPOPT_NOP                1
#define TCPOPT_MAXSEG             2
#define TCPOLEN_MAXSEG            4
#define TCPOPT_WINDOW             3
#define TCPOLEN_WINDOW            3
#define TCPOPT_SACK_PERMITTED     4
#define TCPOLEN_SACK_PERMITTED    2
#define TCPOPT_SACK               5
#define TCPOPT_TIMESTAMP          8
#define TCPOLEN_TIMESTAMP         10
#define TCPOPT_EXP                254    /* Experimental */

#define SCTP_DATA_CHUNK_TYPE                0x00
#define SCTP_INIT_CHUNK_TYPE                0x01
#define SCTP_INIT_ACK_CHUNK_TYPE            0x02
#define SCTP_SACK_CHUNK_TYPE                0x03
#define SCTP_HEARTBEAT_CHUNK_TYPE           0x04
#define SCTP_HEARTBEAT_ACK_CHUNK_TYPE       0x05
#define SCTP_ABORT_CHUNK_TYPE               0x06
#define SCTP_SHUTDOWN_CHUNK_TYPE            0x07
#define SCTP_SHUTDOWN_ACK_CHUNK_TYPE        0x08
#define SCTP_ERROR_CHUNK_TYPE               0x09
#define SCTP_COOKIE_ECHO_CHUNK_TYPE         0x0a
#define SCTP_COOKIE_ACK_CHUNK_TYPE          0x0b
#define SCTP_SHUTDOWN_COMPLETE_CHUNK_TYPE   0x0e
#define SCTP_PAD_CHUNK_TYPE                 0x84
#define SCTP_RECONFIG_CHUNK_TYPE            0x82

#define SCTP_DATA_CHUNK_I_BIT               0x08
#define SCTP_DATA_CHUNK_U_BIT               0x04
#define SCTP_DATA_CHUNK_B_BIT               0x02
#define SCTP_DATA_CHUNK_E_BIT               0x01

#define SCTP_ABORT_CHUNK_T_BIT              0x01
#define SCTP_SHUTDOWN_COMPLETE_CHUNK_T_BIT  0x01

#define MAX_SCTP_CHUNK_BYTES                0xffff

#define FLAG_CHUNK_TYPE_NOCHECK                 0x00000001
#define FLAG_CHUNK_FLAGS_NOCHECK                0x00000002
#define FLAG_CHUNK_LENGTH_NOCHECK               0x00000004
#define FLAG_CHUNK_VALUE_NOCHECK                0x00000008

#define FLAG_INIT_CHUNK_TAG_NOCHECK             0x00000100
#define FLAG_INIT_CHUNK_A_RWND_NOCHECK          0x00000200
#define FLAG_INIT_CHUNK_OS_NOCHECK              0x00000400
#define FLAG_INIT_CHUNK_IS_NOCHECK              0x00000800
#define FLAG_INIT_CHUNK_TSN_NOCHECK             0x00001000
#define FLAG_INIT_CHUNK_OPT_PARAM_NOCHECK       0x00002000
#define FLAG_INIT_CHUNK_OPT_SUPPORTED_ADDRESS_TYPES_PARAM_NOCHECK 0x00004000

#define FLAG_INIT_ACK_CHUNK_TAG_NOCHECK         0x00000100
#define FLAG_INIT_ACK_CHUNK_A_RWND_NOCHECK      0x00000200
#define FLAG_INIT_ACK_CHUNK_OS_NOCHECK          0x00000400
#define FLAG_INIT_ACK_CHUNK_IS_NOCHECK          0x00000800
#define FLAG_INIT_ACK_CHUNK_TSN_NOCHECK         0x00001000
#define FLAG_INIT_ACK_CHUNK_OPT_PARAM_NOCHECK   0x00002000

#define FLAG_SHUTDOWN_CHUNK_CUM_TSN_NOCHECK     0x00000100

#define FLAG_DATA_CHUNK_TSN_NOCHECK             0x00000100
#define FLAG_DATA_CHUNK_SID_NOCHECK             0x00000200
#define FLAG_DATA_CHUNK_SSN_NOCHECK             0x00000400
#define FLAG_DATA_CHUNK_PPID_NOCHECK            0x00000800


#define FLAG_SACK_CHUNK_CUM_TSN_NOCHECK         0x00000100
#define FLAG_SACK_CHUNK_A_RWND_NOCHECK          0x00000200
#define FLAG_SACK_CHUNK_GAP_BLOCKS_NOCHECK      0x00000400
#define FLAG_SACK_CHUNK_DUP_TSNS_NOCHECK        0x00000800

#define FLAG_RECONFIG_RESULT_NOCHECK                            0x00000010
#define FLAG_RECONFIG_SENDER_NEXT_TSN_NOCHECK                   0x00000040
#define FLAG_RECONFIG_RECEIVER_NEXT_TSN_NOCHECK                 0x00000080
#define FLAG_RECONFIG_REQ_SN_NOCHECK                            0x00000010
#define FLAG_RECONFIG_RESP_SN_NOCHECK                           0x00000020
#define FLAG_RECONFIG_LAST_TSN_NOCHECK                          0x00000040
#define FLAG_RECONFIG_NUMBER_OF_NEW_STREAMS_NOCHECK             0x00000080

/* SCTP option names */
#define SCTP_INITMSG                0
#define SCTP_RTOINFO                1
#define SCTP_NODELAY                2
#define SCTP_MAXSEG                 3
#define SCTP_DELAYED_SACK           4
#define SCTP_MAX_BURST              5
#define SCTP_FRAGMENT_INTERLEAVE    6
#define SCTP_INTERLEAVING_SUPPORTED 7
#define SCTP_PADDR_PARAMS           8
#define SCTP_ASSOCINFO              9
#define SCTP_PEER_ADDR_PARAMS       10

#define SCTP_UNORDERED  1

#define SCTP_ENABLE_STREAM_RESET        0x00000900      /* struct sctp_assoc_value */
#define SCTP_RESET_STREAMS              0x00000901      /* struct sctp_reset_streams */
#define SCTP_RESET_ASSOC                0x00000902      /* sctp_assoc_t */
#define SCTP_ADD_STREAMS                0x00000903      /* struct sctp_add_streams */
#define SCTP_STATUS                     0x00000100

#define PD_O_RDWR                       0x002
#define SCTP_COOKIE_WAIT                0x0002

/* For enable stream reset */
#define SCTP_ENABLE_RESET_STREAM_REQ    0x00000001
#define SCTP_ENABLE_RESET_ASSOC_REQ     0x00000002
#define SCTP_ENABLE_CHANGE_ASSOC_REQ    0x00000004
#define SCTP_ENABLE_VALUE_MASK          0x00000007
/* For reset streams */
#define SCTP_STREAM_RESET_INCOMING      0x00000001
#define SCTP_STREAM_RESET_OUTGOING      0x00000002

#define SCTP_IPV4_ADDRESS_PARAMETER_TYPE    0x0005
#define SCTP_IPV6_ADDRESS_PARAMETER_TYPE    0x0006

#define SPP_HB_ENABLE           0x00000001
#define SPP_HB_DISABLE          0x00000002
#define SPP_HB_DEMAND           0x00000004
#define SPP_PMTUD_ENABLE        0x00000008
#define SPP_PMTUD_DISABLE       0x00000010
#define SPP_HB_TIME_IS_ZERO     0x00000080
#define SPP_IPV6_FLOWLABEL      0x00000100
#define SPP_DSCP                0x00000200
#define SPP_IPV4_TOS            SPP_DSCP

enum direction_t {
    DIRECTION_INVALID,
    DIRECTION_INBOUND,  /* packet coming into the kernel under test */
    DIRECTION_OUTBOUND, /* packet leaving the kernel under test */
};

/* A --name=value option in a script */
struct option_list {
    char *name;
    char *value;
    struct option_list *next;
};


#define NO_TIME_RANGE -1    /* time_usecs_end if no range */


/* The errno-related info from strace to summarize a system call error */
struct errno_spec {
    const char *errno_macro;    /* errno symbol (C macro name) */
    const char *strerror;       /* strerror translation of errno */
};


/* Return a pointer to a table of platform-specific string->int mappings. */
struct int_symbol *platform_symbols(void);

/* Convert microseconds to a floating-point seconds value. */
static inline double usecs_to_secs(int64 usecs) { return ((double)usecs) / 1.0e6; };

static inline bool is_valid_u8(int64 x) { return (x >= 0) && (x <= UCHAR_MAX); };

static inline bool is_valid_u16(int64 x) { return (x >= 0) && (x <= USHRT_MAX); };

static inline bool is_valid_u32(int64 x) { return (x >= 0) && (x <= UINT_MAX); };


#define ADDR_STR_LEN 66
#define TUN_DRIVER_DEFAULT_MTU 1500    /* default MTU for tun device */


/* Types of events in a script */
enum event_t {
    INVALID_EVENT = 0,
    PACKET_EVENT,
    SYSCALL_EVENT,
    COMMAND_EVENT,
    NUM_EVENT_TYPES,
};

/* Types of event times */
enum eventTime_t {
    ABSOLUTE_TIME = 0,
    RELATIVE_TIME,
    ANY_TIME,
    ABSOLUTE_RANGE_TIME,
    RELATIVE_RANGE_TIME,
    NUM_TIME_TYPES,
};

/* The types of expressions in a script */
enum expression_t {
    EXPR_NONE,
    EXPR_ELLIPSIS,        /* ... but no value */
    EXPR_INTEGER,         /* integer in 'num' */
    EXPR_LINGER,          /* struct linger for SO_LINGER */
    EXPR_WORD,            /* unquoted word in 'string' */
    EXPR_STRING,          /* double-quoted string in 'string' */
    EXPR_SOCKET_ADDRESS_IPV4, /* sockaddr_in in 'socket_address_ipv4' */
    EXPR_SOCKET_ADDRESS_IPV6, /* sockaddr_in6 in 'socket_address_ipv6' */
    EXPR_BINARY,          /* binary expression, 2 sub-expressions */
    EXPR_LIST,            /* list of expressions */
    EXPR_SCTP_RTOINFO, /* struct sctp_rtoinfo for SCTP_RTOINFO */
    EXPR_SCTP_INITMSG, /* struct sctp_initmsg for SCTP_INITMSG */
    EXPR_SCTP_ASSOCVAL, /* struct sctp_assoc_value */
    EXPR_SCTP_SACKINFO, /* struct sctp_sack_info for SCTP_DELAYED_SACK */
    EXPR_SCTP_PEER_ADDR_PARAMS, /* struct for sctp_paddrparams for SCTP_PEER_ADDR_PARAMS */
    EXPR_SCTP_SNDRCVINFO,
    EXPR_SCTP_RESET_STREAMS,
    EXPR_SCTP_STATUS,
    EXPR_SCTP_ASSOCPARAMS, /* struct sctp_assocparams for SCTP_ASSOCINFO */
    EXPR_SCTP_ADD_STREAMS,

    NUM_EXPR_TYPES,
};

/* Flavors of IP versions we support. */
enum ip_version_t {
    /* Native IPv4, with AF_INET sockets and IPv4 addresses. */
    IP_VERSION_4        = 0,

    /* IPv4-Mapped IPv6 addresses: (see RFC 4291 sec. 2.5.5.2) we
     * use AF_INET6 sockets but all connect(), bind(), and
     * accept() calls are for IPv4 addresses mapped into IPv6
     * address space. So all interface addresses and packets on
     * the wire are IPv4.
     */
    IP_VERSION_4_MAPPED_6   = 1,

    /* Native IPv6, with AF_INET6 sockets and IPv6 addresses. */
    IP_VERSION_6        = 2,
};

enum status_t {
    STATUS_OK  = 0,
    STATUS_ERR = -1,
    STATUS_WARN = -2,    /* a non-fatal error or warning */
};

class PacketDrillConfig;
class PacketDrillScript;
class PacketDrillExpression;
class PacketDrillStruct;
class PacketDrillOption;

/* A system call and its expected result. */
struct syscall_spec {
    const char *name;            /* name of system call */
    cQueue *arguments;           /* arguments to system call */
    PacketDrillExpression *result;        /* expected result from call */
    struct errno_spec *error;    /* errno symbol or NULL */
    char *note;                  /* extra note from strace */
    int64 end_usecs;             /* finish time, if it blocks */
};

/* A shell command line to execute using system(3) */
struct command_spec {
    const char *command_line;    /* executed with /bin/sh */
};

/* The public, top-level call to parse a test script. It first parses the
 * internal linear script buffer and then fills in the
 * 'script' object with the internal representation of the
 * script. Uses the given 'config' object to look up configuration
 * info needed during parsing.
 *Passes the given 'callback_invocation' when calling back to
 * parse_and_finalize_config() after parsing all in-script
 * options.
 *
 * Returns STATUS_OK on success; on failure returns STATUS_ERR. The
 * implementation for this function is in the bison parser file
 * parser.y.
 */
struct Invocation;

} // namespace inet

int parse_script(inet::PacketDrillConfig *config,
        inet::PacketDrillScript *script,
        inet::Invocation *callback_invocation);
void parse_and_finalize_config(inet::Invocation *invocation);

namespace inet {

/* Top-level info about the invocation of a test script */
struct Invocation {
    PacketDrillConfig *config;    /* run-time configuration */
    PacketDrillScript *script;    /* parse tree of the script to run */
};

/* Two expressions combined via a binary operator */
struct binary_expression {
    char *op;    /* binary operator */
    PacketDrillExpression *lhs;    /* left hand side expression */
    PacketDrillExpression *rhs;    /* right hand side expression */
};

struct sctp_rtoinfo_expr {
    PacketDrillExpression *srto_assoc_id;
    PacketDrillExpression *srto_initial;
    PacketDrillExpression *srto_max;
    PacketDrillExpression *srto_min;
};

/* Parse tree for a sctp_initmsg struct in a [gs]etsockopt syscall. */
struct sctp_initmsg_expr {
    PacketDrillExpression *sinit_num_ostreams;
    PacketDrillExpression *sinit_max_instreams;
    PacketDrillExpression *sinit_max_attempts;
    PacketDrillExpression *sinit_max_init_timeo;
};


/* Parse tree for a sctp_assoc_value struct in a [gs]etsockopt syscall. */
struct sctp_assoc_value_expr {
    PacketDrillExpression *assoc_id;
    PacketDrillExpression *assoc_value;
};

/* Parse tree for sctp_assocparams struct in [gs]etsockopt syscall. */
struct sctp_assocparams_expr {
    PacketDrillExpression *sasoc_assoc_id;
    PacketDrillExpression *sasoc_asocmaxrxt;
    PacketDrillExpression *sasoc_number_peer_destinations;
    PacketDrillExpression *sasoc_peer_rwnd;
    PacketDrillExpression *sasoc_local_rwnd;
    PacketDrillExpression *sasoc_cookie_life;
};

/* Parse tree for a sctp_sack_info struct in a [gs]etsockopt syscall. */
struct sctp_sack_info_expr {
    PacketDrillExpression *sack_assoc_id;
    PacketDrillExpression *sack_delay;
    PacketDrillExpression *sack_freq;
};

/* Parse tree for a sctp_paddrparams struct in a [gs]etsockopt syscall. */
struct sctp_paddrparams_expr {
    PacketDrillExpression *spp_assoc_id;
    PacketDrillExpression *spp_address;
    PacketDrillExpression *spp_hbinterval;
    PacketDrillExpression *spp_pathmaxrxt;
    PacketDrillExpression *spp_pathmtu;
    PacketDrillExpression *spp_flags;
    PacketDrillExpression *spp_ipv6_flowlabel;
    PacketDrillExpression *spp_dscp;
};

/* Parse tree for sctp_sndrcvinfo in sctp_recvmsg syscall. */
struct sctp_sndrcvinfo_expr {
    PacketDrillExpression *sinfo_stream;
    PacketDrillExpression *sinfo_ssn;
    PacketDrillExpression *sinfo_flags;
    PacketDrillExpression *sinfo_ppid;
    PacketDrillExpression *sinfo_context;
    PacketDrillExpression *sinfo_timetolive;
    PacketDrillExpression *sinfo_tsn;
    PacketDrillExpression *sinfo_cumtsn;
    PacketDrillExpression *sinfo_assoc_id;
};

struct sctp_reset_streams_expr {
    PacketDrillExpression *srs_assoc_id;
    PacketDrillExpression *srs_flags;
    PacketDrillExpression *srs_number_streams;
    PacketDrillExpression *srs_stream_list;
};

/* Parse tree for a sctp_status struct in a [gs]etsockopt syscall. */
struct sctp_status_expr {
    PacketDrillExpression *sstat_assoc_id;
    PacketDrillExpression *sstat_state;
    PacketDrillExpression *sstat_rwnd;
    PacketDrillExpression *sstat_unackdata;
    PacketDrillExpression *sstat_penddata;
    PacketDrillExpression *sstat_instrms;
    PacketDrillExpression *sstat_outstrms;
    PacketDrillExpression *sstat_fragmentation_point;
    PacketDrillExpression *sstat_primary;
};

/* Parse tree for sctp_add_stream struct for setsockopt. */
struct sctp_add_streams_expr {
    PacketDrillExpression *sas_assoc_id;
    PacketDrillExpression *sas_instrms;
    PacketDrillExpression *sas_outstrms;
};

class INET_API PacketDrillConfig
{
    public:
        PacketDrillConfig();
        ~PacketDrillConfig();

    private:
        enum ip_version_t ip_version;    /* v4, v4-mapped-v6, v6 */
        int socketDomain;    /* AF_INET or AF_INET6 */
        int wireProtocol;    /* AF_INET or AF_INET6 */

        int tolerance_usecs;    /* tolerance for time divergence */
        int mtu;    /* MTU of tun device */
        char *scriptPath;    /* pathname of script file */

    public:
        const char* getScriptPath() const { return scriptPath; };
        void setScriptPath(const char* sPath) { scriptPath = (char*)sPath; };
        int getWireProtocol() const { return wireProtocol; };
        int getSocketDomain() const { return socketDomain; };
        int getToleranceUsecs() const { return tolerance_usecs; };
        void parseScriptOptions(cQueue *options);
};


class INET_API PacketDrillPacket
{
    public:
        PacketDrillPacket();
        ~PacketDrillPacket();

    private:
        Packet* inetPacket;
        enum direction_t direction; /* direction packet is traveling */

    public:
        enum direction_t getDirection() const { return direction; };
        void setDirection(enum direction_t dir) { direction = dir; };
        Packet* getInetPacket() { return inetPacket; };
        void setInetPacket(Packet *pkt) { inetPacket = pkt->dup(); delete pkt;};
};

class INET_API PacketDrillEvent : public cObject
{
    public:
        PacketDrillEvent(enum event_t type_);
        ~PacketDrillEvent();

    private:
        int lineNumber;    /* location in test script file */
        int eventNumber;
        simtime_t eventTime;    /* event time in microseconds */
        simtime_t eventTimeEnd;    /* event time range end (or NO_TIME_RANGE) */
        simtime_t eventOffset;    /* relative event time offset from script start (or NO_TIME_RANGE) */
        enum eventTime_t timeType;    /* type of time */
        enum event_t type;    /* type of the event */
        union {
            PacketDrillPacket *packet;
            struct syscall_spec *syscall;
            struct command_spec *command;
        } eventKind;    /* pointer to the event */

    public:
        void setLineNumber(int number) { lineNumber = number; };
        int getLineNumber() const { return lineNumber; };
        void setEventNumber(int number) { eventNumber = number; };
        int getEventNumber() const { return eventNumber; };
        void setEventTime(int64 usecs) { eventTime = SimTime(usecs, SIMTIME_US); };
        void setEventTime(simtime_t time) { eventTime = time; };
        simtime_t getEventTime() const { return eventTime; };
        void setEventTimeEnd(int64 usecs) { eventTimeEnd = SimTime(usecs, SIMTIME_US); };
        void setEventTimeEnd(simtime_t time) { eventTimeEnd = time; };
        simtime_t getEventTimeEnd() const { return eventTimeEnd; };
        void setEventOffset(int64 usecs) { eventOffset = SimTime(usecs, SIMTIME_US); };
        void setEventOffset(simtime_t time) { eventOffset = time; };
        simtime_t getEventOffset() const { return eventOffset; };
        void setTimeType(enum eventTime_t ttype) { timeType = ttype; };
        enum eventTime_t getTimeType() const { return timeType; };
        void setType(enum event_t tt) { type = tt; };
        enum event_t getType() const { return type; };
        PacketDrillPacket* getPacket() { return eventKind.packet; };
        void setPacket(PacketDrillPacket* packet) { eventKind.packet = packet; };
        void setSyscall(struct syscall_spec *syscall) { eventKind.syscall = syscall; };
        struct syscall_spec *getSyscall() { return eventKind.syscall; };
        void setCommand(struct command_spec *command) { eventKind.command = command; };
        struct command_spec *getCommand() { return eventKind.command; };
};


class INET_API PacketDrillExpression : public cObject
{
    public:
        PacketDrillExpression(enum expression_t type_);
        ~PacketDrillExpression();

    private:
        enum expression_t type;
        union {
            int64 num;
            char *string;
            struct binary_expression *binary;
            struct sctp_rtoinfo_expr *sctp_rtoinfo;
            struct sctp_initmsg_expr *sctp_initmsg;
            struct sctp_assoc_value_expr *sctp_assoc_value;
            struct sctp_assocparams_expr *sctp_assocparams;
            struct sctp_sack_info_expr *sctp_sack_info;
            struct sctp_paddrparams_expr *sctp_paddr_params;
            struct sctp_sndrcvinfo_expr *sctp_sndrcvinfo;
            struct sctp_reset_streams_expr *sctp_resetstreams;
            struct sctp_add_streams_expr *sctp_addstreams;
            struct sctp_status_expr *sctp_status;
            L3Address *ip_address;
        } value;
        cQueue *list;
        const char *format; /* the printf format for printing the value */

    public:
        void setType(enum expression_t t) { type = t; };
        enum expression_t getType() const { return type; };
        void setNum(int64 n) { value.num = n; };
        int64 getNum() const { return value.num; };
        void setString(char* str) { value.string = str; };
        const char* getString() const { return value.string; };
        void setFormat(const char* format_) { format = format_; };
        const char* getFormat() const { return format; };
        cQueue* getList() { return list; };
        void setList(cQueue* queue) { list = queue; };
        struct binary_expression* getBinary() { return value.binary; };
        void setBinary(struct binary_expression* bin) { value.binary = bin; };
        void setRtoinfo(struct sctp_rtoinfo_expr *exp) { value.sctp_rtoinfo = exp; };
        struct sctp_rtoinfo_expr *getRtoinfo() { return value.sctp_rtoinfo; };
        void setInitmsg(struct sctp_initmsg_expr *exp) { value.sctp_initmsg = exp; };
        struct sctp_initmsg_expr *getInitmsg() { return value.sctp_initmsg; };
        void setAssocParams(struct sctp_assocparams_expr *exp) { value.sctp_assocparams = exp; };
        struct sctp_assocparams_expr *getAssocParams() { return value.sctp_assocparams; };
        void setAssocval(struct sctp_assoc_value_expr *exp) { value.sctp_assoc_value = exp; };
        struct sctp_assoc_value_expr *getAssocval() { return value.sctp_assoc_value; };
        void setSackinfo(struct sctp_sack_info_expr *exp) { value.sctp_sack_info = exp; };
        struct sctp_sack_info_expr *getSackinfo() { return value.sctp_sack_info; };
        void setPaddrParams(struct sctp_paddrparams_expr *exp) {value.sctp_paddr_params = exp; };
        struct sctp_paddrparams_expr *getPaddrParams() { return value.sctp_paddr_params; };
        void setSndRcvInfo(struct sctp_sndrcvinfo_expr *exp) {value.sctp_sndrcvinfo = exp; };
        struct sctp_sndrcvinfo_expr *getSndRcvInfo() { return value.sctp_sndrcvinfo; };
        void setResetStreams(struct sctp_reset_streams_expr *exp) {value.sctp_resetstreams = exp; };
        struct sctp_reset_streams_expr *getResetStreams() { return value.sctp_resetstreams; };
        void setIp(L3Address *exp) {value.ip_address = exp; };
        void setStatus(struct sctp_status_expr *exp) {value.sctp_status = exp; };
        struct sctp_status_expr *getStatus() { return value.sctp_status; };
        void setAddStreams(struct sctp_add_streams_expr *exp) {value.sctp_addstreams = exp; };
        struct sctp_add_streams_expr *getAddStreams() { return value.sctp_addstreams; };


        int unescapeCstringExpression(const char *input_string, char **error);
        int getS32(int32 *value, char **error);
        int getU16(uint16 *value, char **error);
        int getU32(uint32 *value, char **error);
        int symbolToInt(const char *input_symbol, int64 *output_integer, char **error);
        bool lookupIntSymbol(const char *input_symbol, int64 *output_integer, struct int_symbol *symbols);
};


class INET_API PacketDrillScript
{
    public:
        PacketDrillScript(const char* file);
        ~PacketDrillScript();

    private:
        cQueue *optionList;
        cQueue *eventList;
        char *buffer;
        int length;
        const char *scriptPath;

    public:
        void readScript();
        int parseScriptAndSetConfig(PacketDrillConfig *config, const char *script_buffer);

        char *getBuffer() { return buffer; }
        int getLength() const { return length; }
        const char *getScriptPath() { return scriptPath; }
        cQueue *getEventList() { return eventList; }
        cQueue *getOptionList() { return optionList; }
        void addEvent(PacketDrillEvent *evt) { eventList->insert(evt); }
        void addOption(PacketDrillOption *opt) { optionList->insert((cObject *)opt); }  //FIXME Why needed the cast to cObject?
};

class INET_API PacketDrillStruct: public cObject
{
    public:
        PacketDrillStruct();
        PacketDrillStruct(int64 v1, int64 v2);
        PacketDrillStruct(int64 v1, int64 v2, int32 v3, int32 v4, cQueue *streams);

        int64 getValue1() const { return value1; };
        void setValue1(uint64 value) { value1 = value; };
        int64 getValue2() const { return value2; };
        void setValue2(uint64 value) { value2 = value; };
        int32 getValue3() const { return value3; }
        int32 getValue4() const { return value4; }
        cQueue *getStreams() { return streamNumbers; };
        virtual PacketDrillStruct *dup() const { return new PacketDrillStruct(*this); };

    private:
        int64 value1;
        int64 value2;
        int32 value3;
        int32 value4;
        cQueue *streamNumbers = nullptr;
};

class INET_API PacketDrillOption: public cObject
{
    public:
        PacketDrillOption(char *name, char *value);

        const char *getName() const { return name; }
        void setName(char *name_) { free(name); name = strdup(name_); }
        const char *getValue() const { return value; }
        void setValue(char *value_) { free(value); value = strdup(value_); }

    private:
        char *name;
        char *value;
};

typedef std::vector<uint8_t> ByteVector;

class INET_API PacketDrillBytes: public cObject
{
    public:
        PacketDrillBytes();
        PacketDrillBytes(uint8 byte);

        void appendByte(uint8 byte);
        uint32 getListLength() const { return listLength; };
        ByteVector* getByteList() { return &byteList; };

    private:
        std::vector<uint8_t> byteList;
        uint32 listLength;
};

class INET_API PacketDrillTcpOption : public cObject
{
    public:
        PacketDrillTcpOption(uint16 kind_, uint16 length_);

    private:
        uint16 kind;
        uint16 length;
        uint16 mss; /* in network order */
        struct
        {
                uint32 val; /* in network order */
                uint32 ecr; /* in network order */
        } timeStamp;
        cQueue *blockList;
        uint8 windowScale;
        uint16 blockCount;

    public:
        uint16 getKind() { return kind; };
        uint16 getLength() { return length; };
        void setLength(uint16 len) {length = len;};
        uint16 getMss() { return mss; };
        void setMss(uint16 mss_) { mss = mss_; };
        uint16 getWindowScale() { return windowScale; };
        void setWindowScale(uint16 ws_) { windowScale = ws_; };
        uint32 getVal() { return timeStamp.val; };
        void setVal(uint32 val_) { timeStamp.val = val_; };
        uint32 getEcr() { return timeStamp.ecr; };
        void setEcr(uint32 ecr_) { timeStamp.ecr = ecr_; };
        cQueue *getBlockList() { return blockList; };
        void setBlockList(cQueue *bList) { blockList = bList; };
        uint16 getBlockCount() { return blockCount; };
        void increaseBlockCount() { blockCount++; };
};

class INET_API PacketDrillSctpChunk : public cObject
{
    public:
        PacketDrillSctpChunk(uint8 type_, sctp::SctpChunk *sctpChunk);

    private:
        uint8 type;
        sctp::SctpChunk *chunk;

    public:
        uint8 getType() { return type; };
        sctp::SctpChunk *getChunk() { return chunk; };
};

class INET_API PacketDrillSctpParameter : public cObject
{
    public:
        PacketDrillSctpParameter(uint16 type_, int16 len_, void* content_);
        ~PacketDrillSctpParameter();

    private:
        int32 parameterValue;
        cQueue* parameterList;
        int16 parameterLength;
        ByteVector *bytearray;
        uint32 flags;
        uint16 type;
        void *content;

    public:
        int32 getValue() const { return parameterValue; };
        cQueue* getList() { return parameterList; };
        uint32 getFlags() const { return flags; };
        void setFlags(uint32 flgs_) { flags = flgs_; };
        int16 getLength() const { return parameterLength; };
        uint16 getType() const { return type; };
        ByteVector *getByteList() { return bytearray; };
        void setByteArrayPointer(ByteVector *ptr) { bytearray = ptr; };
        void *getContent() { return content; };
};

}

#endif
