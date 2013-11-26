class noncobject Address;

cplusplus {{
   #include "Address.h"
}}

packet WiseRouteDatagram
{
    Address srcAddr;
    Address destAddr;
    unsigned long seqNum = 0;
	int     isFlood;
	Address finalDestAddr;
	Address initialSrcAddr;
	int     nbHops;
}
