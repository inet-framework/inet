#ifndef __INET_OSPFV3SPLITTER_H
#define __INET_OSPFV3SPLITTER_H

#include <string>

#include "inet/common/ModuleAccess.h"
#include "inet/common/ModuleRefByPar.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6InterfaceData.h"
#include "inet/networklayer/ipv6/Ipv6RoutingTable.h"
#include "inet/routing/ospfv3/Ospfv3Common.h"
#include "inet/routing/ospfv3/Ospfv3Packet_m.h"
#include "inet/routing/ospfv3/Ospfv3Timers.h"
#include "inet/routing/ospfv3/process/Ospfv3Process.h"

namespace inet {
namespace ospfv3 {

int ospfv3InterfaceIndex = 1;

class INET_API Ospfv3Splitter : protected cListener, public cSimpleModule
{
  public:
    Ospfv3Splitter();
    virtual ~Ospfv3Splitter();
    ModuleRefByPar<IInterfaceTable> ift;

  private:
    void parseConfig(cXMLElement *, cXMLElement *);
    void parseRouting(cXMLElement *);
    void parseInterfaces(cXMLElement *);
    void addNewProcess(cXMLElement *, cXMLElement *, int);

  private:
    cModule *containingModule = nullptr;
    cModule *routingModule = nullptr;
    std::vector<Ospfv3Process *> processesModules;
    std::map<std::string, int> processInVector; // processID is mapped to its position in vector of processes in Splitter
    std::map<std::string, std::pair<int, int>> interfaceToProcess; // name of interface to position in vector - "eth0" : 0
    std::map<char *, char *> processToInterface; // "process101":"eth0", "process100":"eth0"

  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
};

} // namespace ospfv3
} // namespace inet

#endif

