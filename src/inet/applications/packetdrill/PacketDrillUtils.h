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
#include "inet/networklayer/common/L3Address.h"
#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
 #include <sys/socket.h>
#endif

using namespace inet;

/* For tables mapping symbolic strace strings to the corresponding
 * integer values.
 */
struct int_symbol {
    int64 value;
    const char *name;
};

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
    EXPR_BINARY,          /* binary expression, 2 sub-expressions */
    EXPR_LIST,            /* list of expressions */
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


//namespace inet {

class PacketDrillConfig;
class PacketDrillScript;
class PacketDrillExpression;
class PacketDrillStruct;

/* A system call and its expected result. */
struct syscall_spec {
    const char *name;            /* name of system call */
    cQueue *arguments;           /* arguments to system call */
    PacketDrillExpression *result;        /* expected result from call */
    struct errno_spec *error;    /* errno symbol or NULL */
    char *note;                  /* extra note from strace */
    int64 end_usecs;             /* finish time, if it blocks */
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
int parse_script(PacketDrillConfig *config,
    PacketDrillScript *script,
    struct invocation *callback_invocation);

/* Top-level info about the invocation of a test script */
struct invocation {
    PacketDrillConfig *config;    /* run-time configuration */
    PacketDrillScript *script;    /* parse tree of the script to run */
};

/* Two expressions combined via a binary operator */
struct binary_expression {
    char *op;    /* binary operator */
    PacketDrillExpression *lhs;    /* left hand side expression */
    PacketDrillExpression *rhs;    /* right hand side expression */
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
        const char* getScriptPath() { return scriptPath; };
        void setScriptPath(const char* sPath) { scriptPath = (char*)strdup(sPath); };
        int getWireProtocol() { return wireProtocol; };
        int getSocketDomain() { return socketDomain; };
        int getToleranceUsecs() { return tolerance_usecs; };
};


class INET_API PacketDrillPacket
{
    public:
        PacketDrillPacket();
        ~PacketDrillPacket();

    private:
        cPacket* inetPacket;
        enum direction_t direction; /* direction packet is traveling */

    public:
        enum direction_t getDirection() { return direction; };
        void setDirection(enum direction_t dir) { direction = dir; };
        cPacket* getInetPacket() { return inetPacket; };
        void setInetPacket(cPacket *pkt) { inetPacket = pkt->dup(); delete pkt;};
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
        } event;    /* pointer to the event */

    public:
        void setLineNumber(int number) { lineNumber = number; };
        int getLineNumber() { return lineNumber; };
        void setEventNumber(int number) { eventNumber = number; };
        int getEventNumber() { return eventNumber; };
        void setEventTime(int64 usecs) { eventTime = SimTime(usecs, SIMTIME_US); };
        void setEventTime(simtime_t time) { eventTime = time; };
        simtime_t getEventTime() { return eventTime; };
        void setEventTimeEnd(int64 usecs) { eventTimeEnd = SimTime(usecs, SIMTIME_US); };
        void setEventTimeEnd(simtime_t time) { eventTimeEnd = time; };
        simtime_t getEventTimeEnd() { return eventTimeEnd; };
        void setEventOffset(int64 usecs) { eventOffset = SimTime(usecs, SIMTIME_US); };
        void setEventOffset(simtime_t time) { eventOffset = time; };
        simtime_t getEventOffset() { return eventOffset; };
        void setTimeType(enum eventTime_t ttype) { timeType = ttype; };
        enum eventTime_t getTimeType() { return timeType; };
        void setType(enum event_t tt) { type = tt; };
        enum event_t getType() { return type; };
        PacketDrillPacket* getPacket() { return event.packet; };
        void setPacket(PacketDrillPacket* packet) { event.packet = packet; };
        void setSyscall(struct syscall_spec *syscall) { event.syscall = syscall; };
        struct syscall_spec *getSyscall() { return event.syscall; };
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
            cQueue *list;
        } value;
        const char *format; /* the printf format for printing the value */

    public:
        void setType(enum expression_t t) { type = t; };
        enum expression_t getType() { return type; };
        void setNum(int64 n) { value.num = n; };
        int64 getNum() { return value.num; };
        void setString(char* str) { value.string = strdup(str); };
        char* getString() { return strdup(value.string); };
        void setFormat(const char* format_) { format = format_; };
        const char* getFormat() { return format; };
        cQueue* getList() { return value.list; };
        void setList(cQueue* queue) { value.list = queue; };
        struct binary_expression* getBinary() { return value.binary; };
        void setBinary(struct binary_expression* bin) { value.binary = bin; };

        int unescapeCstringExpression(const char *input_string, char **error);
        int getS32(int32 *value, char **error);
        int symbolToInt(const char *input_symbol, int64 *output_integer, char **error);
        bool lookupIntSymbol(const char *input_symbol, int64 *output_integer, struct int_symbol *symbols);
};


class INET_API PacketDrillScript
{
    public:
        PacketDrillScript(const char* file);
        ~PacketDrillScript();

    private:
        struct option_list *optionList;
        cQueue *eventList;
        char *buffer;
        int length;
        const char *scriptPath;

    public:
        void readScript();
        int parseScriptAndSetConfig(PacketDrillConfig *config, const char *script_buffer);

        char *getBuffer() { return buffer; };
        int getLength() { return length; };
        const char *getScriptPath() { return scriptPath; };
        cQueue *getEventList() { return eventList; };
        struct option_list *getOptionList() { return optionList; };
        void setOptionList(struct option_list *optL) { optionList = optL;};
        void addEvent(PacketDrillEvent *evt) { eventList->insert(evt); };
};

class INET_API PacketDrillStruct: public cObject
{
    public:
        PacketDrillStruct();
        PacketDrillStruct(uint32 v1, uint32 v2);

        uint32 getValue1() { return value1; };
        void setValue1(uint32 value) { value1 = value; };
        uint32 getValue2() { return value2; };
        void setValue2(uint32 value) { value2 = value; };
        virtual PacketDrillStruct *dup() const { return new PacketDrillStruct(*this); };

    private:
        uint32 value1;
        uint32 value2;
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

#endif
