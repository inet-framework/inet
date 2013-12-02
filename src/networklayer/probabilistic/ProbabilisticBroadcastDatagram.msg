class noncobject Address;

cplusplus {{
   #include "Address.h"
}}

packet ProbabilisticBroadcastDatagram
{
    Address   destAddr;
    Address   srcAddr;
    long      id;    // Unique message ID ~ f(node address, unique number in node scope).
    simtime_t appTtl;   // Message time-to-live set by application.
    Address   initialSrcAddr;
    Address   finalDestAddr;
    int       nbHops;
    int       protocol = -1;
}
