//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
/**
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#include "inet/routing/eigrp/tables/EigrpInterfaceTable.h"

#include <algorithm>

#include "inet/common/stlutils.h"

namespace inet {
namespace eigrp {

Define_Module(EigrpInterfaceTable);

std::ostream& operator<<(std::ostream& out, const EigrpInterface& iface)
{
    out << iface.getInterfaceName() << "(" << iface.getInterfaceId() << ")";
    out << "  Peers:" << iface.getNumOfNeighbors();
    out << "  Passive:";
    if (iface.isPassive()) out << "enabled";
    else out << "disabled";
//    out << "  stubs:" << iface.getNumOfStubs();
    out << "  HelloInt:" << iface.getHelloInt();
    out << "  HoldInt:" << iface.getHoldInt();
    out << "  SplitHorizon:";
    if (iface.isSplitHorizonEn()) out << "enabled";
    else out << "disabled";
    /*out << "  bw:" << iface.getBandwidth();
    out << "  dly:" << iface.getDelay();
    out << "  rel:" << iface.getReliability() << "/255";
    out << "  rLoad:" << iface.getLoad() << "/255";
    out << "  pendingMsgs:" << iface.getPendingMsgs();*/
    return out;
}

EigrpInterface::~EigrpInterface()
{
    hellot = nullptr;
}

EigrpInterface::EigrpInterface(NetworkInterface *iface, int networkId, bool enabled) :
               interfaceId(iface->getInterfaceId()), networkId(networkId), enabled(enabled)
{
    hellot = nullptr;
    neighborCount = 0;
    stubCount = 0;
    splitHorizon = true;
    passive = false;
    mtu = iface->getMtu();
    this->setInterfaceDatarate(iface->getDatarate());
    load = 1;
    reliability = 255;
    interfaceName = iface->getInterfaceName();
    relMsgs = 0;
    pendingMsgs = 0;

    if (!iface->isMulticast() && bandwidth <= 1544) { // Non-broadcast Multi Access interface (no multicast) with bandwidth equal or lower than T1 link
        helloInt = 60;
        holdInt = 180;
    }
    else {
        helloInt = 5;
        holdInt = 15;
    }
}

bool EigrpInterface::isMulticastAllowedOnIface(NetworkInterface *iface)
{
    if (iface->isMulticast())
        if (getNumOfNeighbors() > 1)
            return true;

    return false;
}

void EigrpInterface::setInterfaceDatarate(double datarate) {

    interfaceDatarate = datarate;

    switch ((long)datarate) {
        case 64000: // 56k modem
            delay = 20000;
            break;
        case 56000: // 56k modem
            delay = 20000;
            break;
        case 1544000: // T1
            delay = 20000;
            break;
        case 10000000: // Eth10
            delay = 1000;
            break;
        default: // >Eth10
            delay = 100;
            break;
    }
    bandwidth = datarate / 1000;
}

EigrpInterfaceTable::~EigrpInterfaceTable()
{
    int cnt = eigrpInterfaces.size();
    EigrpInterface *iface;

    for (int i = 0; i < cnt; i++) {

        iface = eigrpInterfaces[i];
        eigrpInterfaces[i] = nullptr;
        delete iface;
    }
    eigrpInterfaces.clear();
}

void EigrpInterfaceTable::initialize(int stage)
{
    cSimpleModule::initialize(stage);
    if (stage == INITSTAGE_ROUTING_PROTOCOLS) {
        WATCH_PTRVECTOR(eigrpInterfaces);
    }
}

void EigrpInterfaceTable::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}

void EigrpInterfaceTable::addInterface(EigrpInterface *interface)
{
    // TODO check duplicity
    eigrpInterfaces.push_back(interface);
}

EigrpInterface *EigrpInterfaceTable::removeInterface(EigrpInterface *iface)
{
    InterfaceVector::iterator it;
    it = find(eigrpInterfaces, iface);
    if (it != eigrpInterfaces.end()) {
        eigrpInterfaces.erase(it);
        return iface;
    }

    return nullptr;
}

EigrpInterface *EigrpInterfaceTable::findInterfaceById(int ifaceId)
{
    InterfaceVector::iterator it;
    EigrpInterface *iface;

    for (it = eigrpInterfaces.begin(); it != eigrpInterfaces.end(); it++) {
        iface = *it;
        if (iface->getInterfaceId() == ifaceId) {
            return iface;
        }
    }

    return nullptr;
}

} // namespace eigrp
} // namespace inet

