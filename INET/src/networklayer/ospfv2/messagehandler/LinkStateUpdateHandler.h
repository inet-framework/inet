#ifndef __INET_LINKSTATEUPDATEHANDLER_H
#define __INET_LINKSTATEUPDATEHANDLER_H

#include "IMessageHandler.h"
#include "OSPFcommon.h"

namespace OSPF {

class LinkStateUpdateHandler : public IMessageHandler
{
private:
    struct AcknowledgementFlags {
        bool floodedBackOut;
        bool lsaIsNewer;
        bool lsaIsDuplicate;
        bool impliedAcknowledgement;
        bool lsaReachedMaxAge;
        bool noLSAInstanceInDatabase;
        bool anyNeighborInExchangeOrLoadingState;
    };

private:
    bool ValidateLSChecksum(OSPFLSA* lsa) { return true; }   // not implemented
    void AcknowledgeLSA(OSPFLSAHeader& lsaHeader, Interface* intf, AcknowledgementFlags acknowledgementFlags, RouterID lsaSource);

public:
    LinkStateUpdateHandler(Router* containingRouter);

    void    ProcessPacket(OSPFPacket* packet, Interface* intf, Neighbor* neighbor);
};

} // namespace OSPF

#endif // __INET_LINKSTATEUPDATEHANDLER_H

