class noncobject ModuleIdAddress;

cplusplus {{
   #include "ModuleIdAddress.h"
}}

packet FloodDatagram
{
    @customize(true);

    ModuleIdAddress srcAddress;
    ModuleIdAddress destAddress;
    int             ttl = 1;    // time to live field
    unsigned long   seqNum = 0; // sequence number
}
