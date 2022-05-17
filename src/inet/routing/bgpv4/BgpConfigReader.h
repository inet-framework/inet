//
// Copyright (C) 2010 Helene Lageber
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_BGPCONFIGREADER_H
#define __INET_BGPCONFIGREADER_H

#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/routing/bgpv4/BgpCommon.h"
#include "inet/routing/bgpv4/BgpRouter.h"
#include "inet/routing/bgpv4/bgpmessage/BgpHeader_m.h"
#include "inet/routing/bgpv4/bgpmessage/BgpUpdate.h"

namespace inet {
namespace bgp {

class INET_API BgpConfigReader
{
  private:
    enum { NB_TIMERS = 4 };
    cModule *bgpModule = nullptr;
    IInterfaceTable *ift = nullptr;
    BgpRouter *bgpRouter = nullptr;

  public:
    BgpConfigReader(cModule *bgpModule, IInterfaceTable *ift);
    virtual ~BgpConfigReader() {}

    void loadConfigFromXML(cXMLElement *bgpConfig, BgpRouter *bgpRouter);

  private:
    std::vector<const char *> findInternalPeers(cXMLElementList& ASConfig);
    void loadASConfig(cXMLElementList& ASConfig);
    void loadEbgpSessionConfig(cXMLElementList& ASConfig, cXMLElementList& sessionList, simtime_t *delayTab);
    AsId findMyAS(cXMLElementList& ASList, int& outRouterPosition);
    void loadTimerConfig(cXMLElementList& timerConfig, simtime_t *delayTab);
    int isInInterfaceTable(IInterfaceTable *ifTable, Ipv4Address addr);
    int isInInterfaceTable(IInterfaceTable *ifTable, std::string ifName);
    unsigned int calculateStartDelay(int rtListSize, unsigned char rtPosition, unsigned char rtPeerPosition);
    bool getBoolAttrOrPar(const cXMLElement& ifConfig, const char *name) const;
    int getIntAttrOrPar(const cXMLElement& ifConfig, const char *name) const;
    const char *getStrAttrOrPar(const cXMLElement& ifConfig, const char *name) const;
};

} // namespace bgp
} // namespace inet

#endif

