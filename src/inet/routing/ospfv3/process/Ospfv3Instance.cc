
#include "inet/routing/ospfv3/interface/Ospfv3Interface.h"
#include "inet/routing/ospfv3/process/Ospfv3Instance.h"

namespace inet {
namespace ospfv3 {

Ospfv3Instance::Ospfv3Instance(int instanceId, Ospfv3Process* parentProcess, int addressFamily)
{
    this->instanceID = instanceId;
    this->containingProcess=parentProcess;
    this->addressFamily = addressFamily;
    this->containingModule=findContainingNode(this->containingProcess);
    this->ift = check_and_cast<IInterfaceTable *>(containingModule->getSubmodule("interfaceTable"));
}

Ospfv3Instance::~Ospfv3Instance()
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        delete areas[i];
    }
}

bool Ospfv3Instance::hasArea(Ipv4Address areaId)
{
    std::map<Ipv4Address, Ospfv3Area*>::iterator areaIt = this->areasById.find(areaId);
    if (areaIt == this->areasById.end())
        return false;

    return true;
}//hasArea

void Ospfv3Instance::addArea(Ospfv3Area* newArea)
{
    this->areas.push_back(newArea);
    this->areasById[newArea->getAreaID()]=newArea;
}//addArea

Ospfv3Area* Ospfv3Instance::getAreaById(Ipv4Address areaId)
{
    std::map<Ipv4Address, Ospfv3Area*>::iterator areaIt = this->areasById.find(areaId);
    if (areaIt == this->areasById.end())
        return nullptr;

    return areaIt->second;
}//getAreaById

void Ospfv3Instance::processPacket(Packet* pk)
{
    const auto& packet = pk->peekAtFront<Ospfv3Packet>();
    EV_INFO << "Process " << this->containingProcess->getProcessID() << " received packet: (" << packet->getClassName() << ")" << packet->getName() << "\n";
    if (packet->getVersion()!=3) {
        delete pk;
        return;
    }

    int intfId = pk->getTag<InterfaceInd>()->getInterfaceId();
    Ipv4Address areaId = packet->getAreaID();
    Ospfv3Area* area = this->getAreaById(packet->getAreaID());
    if (area!=nullptr) {
        Ospfv3Interface *intf = area->getInterfaceById(intfId);

        if (intf == nullptr) {
            EV_DEBUG <<"Interface is null in instance::processPacket\n";
            //it must be the backbone area and...
            if (areaId == Ipv4Address::UNSPECIFIED_ADDRESS) {
                if (this->getAreaCount() > 1) {
                    // it must be a virtual link and the source router's router ID must be the endpoint of this virtual link and...
                    intf = area->findVirtualLink(packet->getRouterID());

                    if (intf != nullptr) {
                        Ospfv3Area *virtualLinkTransitArea = this->getAreaById(intf->getTransitAreaID());
                        if (virtualLinkTransitArea != nullptr) {
//                           // the receiving interface must attach to the virtual link's configured transit area
                            Ospfv3Interface *virtualLinkInterface = virtualLinkTransitArea->getInterfaceById(intfId);
//
                            if (virtualLinkInterface == nullptr) {
                                intf = nullptr;
                            }
                        }
                        else {
                            intf = nullptr;
                        }
                    }
                }
            }
        }
        if (intf != nullptr) {
            Ipv6Address destinationAddress =  pk->getTag<L3AddressInd>()->getDestAddress().toIpv6();
            Ipv6Address allDRouters = Ipv6Address::ALL_OSPF_DESIGNATED_ROUTERS_MCAST;
            Ospfv3Interface::Ospfv3InterfaceFaState interfaceState = intf->getState();

            // if destination address is ALL_D_ROUTERS the receiving interface must be in DesignatedRouter or Backup state
            if (
                    ((destinationAddress == allDRouters) &&
                            (
                                    (interfaceState == Ospfv3Interface::INTERFACE_STATE_DESIGNATED) ||
                                    (interfaceState == Ospfv3Interface::INTERFACE_STATE_BACKUP)
                            )
                    ) ||
                    (destinationAddress != allDRouters)
            )
            {
                // packet authentication
                ospf::OspfPacketType packetType = static_cast<ospf::OspfPacketType>(packet->getType());
                Ospfv3Neighbor* neighbor = nullptr;

                // all packets except HelloPackets are sent only along adjacencies, so a Neighbor must exist
                if (packetType != ospf::OspfPacketType::HELLO_PACKET)
                    neighbor = intf->getNeighborById(packet->getRouterID());

                switch (packetType) {
                case ospf::OspfPacketType::HELLO_PACKET:
                    intf->processHelloPacket(pk);
                    break;

                case ospf::OspfPacketType::DATABASE_DESCRIPTION_PACKET:
                    if (neighbor != nullptr) {
                        EV_DEBUG << "Instance is sending packet to interface\n";
                        intf->processDDPacket(pk);
                    }
                    break;

                case ospf::OspfPacketType::LINKSTATE_REQUEST_PACKET:
                    if (neighbor != nullptr) {
                        intf->processLSR(pk, neighbor);
                    }
                    break;

                case ospf::OspfPacketType::LINKSTATE_UPDATE_PACKET:
                    if (neighbor != nullptr) {
                        intf->processLSU(pk, neighbor);
                    }
                    break;

                case ospf::OspfPacketType::LINKSTATE_ACKNOWLEDGEMENT_PACKET:
                    if (neighbor != nullptr) {
                        intf->processLSAck(pk, neighbor);
                    }
                    break;

                default:
                    break;
                }
            }
            else {
                delete(pk);
            }
        }
    }
}//processPacket

void Ospfv3Instance::removeFromAllRetransmissionLists(LSAKeyType lsaKey)
{
    long areaCount = areas.size();
    for (long i = 0; i < areaCount; i++) {
        areas.at(i)->removeFromAllRetransmissionLists(lsaKey);
    }
}

void Ospfv3Instance::init()
{
    for (auto it = this->areas.begin(); it!=this->areas.end(); it++)
        (*it)->init();

    WATCH_PTRVECTOR(this->areas);
}//init

void Ospfv3Instance::debugDump()
{
    EV_DEBUG << "Instance " << this->getInstanceID() << "\n";
    for (auto it=this->areas.begin();it!=this->areas.end(); it++) {
        EV_DEBUG << "\tArea id " << (*it)->getAreaID() << " has these interfaces:\n";
        (*it)->debugDump();
    }
}//debugDump

Ipv4Address Ospfv3Instance::getNewInterAreaPrefixLinkStateID()
{
    Ipv4Address currIP = this->interAreaPrefixLsID;
    int newIP = currIP.getInt()+1;
    this->interAreaPrefixLsID = Ipv4Address(newIP);
    return currIP;
}

void Ospfv3Instance::subtractInterAreaPrefixLinkStateID()
{
    Ipv4Address currIP = this->interAreaPrefixLsID;
    int newIP = currIP.getInt()-1;
    this->interAreaPrefixLsID = Ipv4Address(newIP);
}

std::string Ospfv3Instance::detailedInfo() const
{
    std::stringstream out;
    int processID = this->getProcess()->getProcessID();
    Ipv4Address routerID = this->getProcess()->getRouterID();
    out << "Ospfv3 " << processID << " address-family ";

    if (this->addressFamily == IPV4INSTANCE)
        out << "IPv4 (router-id " << routerID << ")\n\n";
    else
        out << "IPv6 (router-id " << routerID << ")\n\n";

    out << "Neighbor ID\tPri\tState\t\tDead Time\tInterface ID\tInterface\n";
    for (auto it=this->areas.begin(); it!=this->areas.end(); it++) {
        int intfCount = (*it)->getInterfaceCount();
        for (int i=0; i<intfCount; i++) {
            Ospfv3Interface* intf = (*it)->getInterface(i);
            int neiCount = intf->getNeighborCount();
            for (int n=0; n<neiCount; n++) {
                Ospfv3Neighbor* neighbor = intf->getNeighbor(n);
                out << neighbor->getNeighborID() << "\t";
                out << neighbor->getNeighborPriority() << "\t";
                switch(neighbor->getState()) {
                    case Ospfv3Neighbor::DOWN_STATE:
                        out << "DOWN\t\t";
                        break;

                    case Ospfv3Neighbor::ATTEMPT_STATE:
                        out << "ATTEMPT\t\t";
                        break;

                    case Ospfv3Neighbor::INIT_STATE:
                        out << "INIT\t\t";
                        break;

                    case Ospfv3Neighbor::TWOWAY_STATE:
                        if (intf->getDesignatedID() == Ipv4Address::UNSPECIFIED_ADDRESS)
                            out << "2WAY\t\t";
                        else
                            out << "2WAY/DROTHER\t";
                        break;

                    case Ospfv3Neighbor::EXCHANGE_START_STATE:
                        out << "EXSTART\t\t";
                        break;

                    case Ospfv3Neighbor::EXCHANGE_STATE:
                        out << "EXCHANGE\t\t";
                        break;

                    case Ospfv3Neighbor::LOADING_STATE:
                        out << "LOADING\t\t";
                        break;

                    case Ospfv3Neighbor::FULL_STATE:
                        if (neighbor->getNeighborID() == intf->getDesignatedID())
                            out << "FULL/DR\t\t";
                        else if (neighbor->getNeighborID() == intf->getBackupID())
                            out << "FULL/BDR\t\t";
                        else
                            out << "FULL/DROTHER\t";
                        break;
                }

                int dead = intf->getDeadInterval() - ((int)simTime().dbl() - neighbor->getLastHelloTime());
                if (dead < 0)
                    dead = 0;
                out << dead << "\t\t";//"00:00:40\t\t";
                out << neighbor->getNeighborInterfaceID() << "\t\t";
                out << intf->getIntName() << "\n";
            }
        }
    }

    return out.str();
}

} // namespace ospfv3
}//namespace inet

