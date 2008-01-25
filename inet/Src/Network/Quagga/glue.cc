
#include "glue.h"

enum IPProtocolId inet_protocol(u_int8_t prot)
{
    switch(prot)
    {
        case IPPROTO_OSPFIGP:
            return IP_PROT_OSPF;

        default:
            ASSERT(false);
    }
}
