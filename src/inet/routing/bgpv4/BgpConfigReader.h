//
// Copyright (C) 2010 Helene Lageber
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_BGPCONFIGREADER
#define __INET_BGPCONFIGREADER

#include "inet/common/INETDefs.h"
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
    cModule *bgpModule = nullptr;
    IInterfaceTable *ift = nullptr;
    BgpRouter *bgpRouter = nullptr;

  public:
    BgpConfigReader(cModule *bgpModule, IInterfaceTable *ift);
    virtual ~BgpConfigReader() {};

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

#endif // ifndef __INET_BGPCONFIGREADER

