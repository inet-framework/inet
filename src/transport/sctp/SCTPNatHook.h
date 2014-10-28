#ifndef __INET_SCTPNATHOOK_H
#define __INET_SCTPNATHOOK_H

#include "INetfilter.h"
#include "SCTPNatTable.h"
#include "INETDefs.h"
#include "InterfaceTableAccess.h"

class IPv4;

class INET_API SCTPNatHook : public cSimpleModule, INetfilter::IHook
{
    protected:
        IPv4 *ipLayer;      // IPv4 module
        SCTPNatTable* natTable;
        IRoutingTable *rt;
        IInterfaceTable *ift;
        uint64 nattedPackets;
        void initialize();
        void finish();

    public:
      SCTPNatHook();
      virtual ~SCTPNatHook();
      IHook::Result datagramPreRoutingHook(IPv4Datagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, IPv4Address& nextHopAddr);
      IHook::Result datagramForwardHook(IPv4Datagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, IPv4Address& nextHopAddr);
      IHook::Result datagramPostRoutingHook(IPv4Datagram* datagram, const InterfaceEntry* inIE, const InterfaceEntry*& outIE, IPv4Address& nextHopAddr);
      IHook::Result datagramLocalInHook(IPv4Datagram* datagram, const InterfaceEntry* inIE);
      IHook::Result datagramLocalOutHook(IPv4Datagram* datagram, const InterfaceEntry*& outIE, IPv4Address& nextHopAddr);
      void sendBackError(IPv4Datagram* dgram);
};

#endif