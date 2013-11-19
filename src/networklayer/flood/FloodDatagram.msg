class noncobject Address;

cplusplus {{
   #include "Address.h"
}}

packet FloodDatagram
{
    @customize(true);

    Address       srcAddress;
    Address       destAddress;
    int           ttl = 1;    // time to live field
    unsigned long seqNum = 0; // sequence number
}
