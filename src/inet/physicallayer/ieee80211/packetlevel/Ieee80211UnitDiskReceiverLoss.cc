//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/common/ModuleAccess.h"
#include "inet/physicallayer/common/packetlevel/Radio.h"

#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211UnitDiskReceiverLoss.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211UnitDiskTransmitter.h"
#include "inet/physicallayer/unitdisk/UnitDiskTransmitter.h"


namespace inet {
namespace physicallayer {

std::map<int, Ieee80211UnitDiskReceiverLoss::Links> Ieee80211UnitDiskReceiverLoss::uniLinks;
std::map<int, Ieee80211UnitDiskReceiverLoss::Links> Ieee80211UnitDiskReceiverLoss::lossLinks;
std::map<int, IMobility *> Ieee80211UnitDiskReceiverLoss::nodes;


Define_Module(Ieee80211UnitDiskReceiverLoss);

Ieee80211UnitDiskReceiverLoss::Ieee80211UnitDiskReceiverLoss() :
        Ieee80211UnitDiskReceiver()
{
}

Ieee80211UnitDiskReceiverLoss::~Ieee80211UnitDiskReceiverLoss()
{
    if (!nodes.empty())
        nodes.clear();
    if (!uniLinks.empty())
        uniLinks.clear();
    if (!lossLinks.empty())
        lossLinks.clear();
}

void Ieee80211UnitDiskReceiverLoss::initialize(int stage)
{
    Ieee80211UnitDiskReceiver::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        if (!nodes.empty())
            nodes.clear();
        if (!uniLinks.empty())
            uniLinks.clear();
        if (!lossLinks.empty())
            lossLinks.clear();
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER_NEIGHBOR_CACHE) {

        auto parent = this->getParentModule();
        auto node = getContainingNode(this);
        mobility = check_and_cast<IMobility *>(node->getSubmodule("mobility"));
        hostId = node->getId();
        nodes.insert(std::make_pair(hostId, mobility));

        double numUniLink = par("perUniLinks").doubleValue();
        double numLossLinks = par("perLosLinks").doubleValue();
        if (numUniLink == 0 && numLossLinks == 0)
            return;
        if (numUniLink > 100|| numLossLinks > 100)
            throw cRuntimeError("perUniLinks %f or perLosLinks %f too big", numUniLink, numLossLinks);
        if (par("forceUni"))
            if (numUniLink > 50)
              throw cRuntimeError("perUniLinks %f with forceUni too big", numUniLink);
        if (!uniLinks.empty() || !lossLinks.empty())
            return;
        // Extract the connection list
        cTopology topo("topo");
        topo.extractByProperty("networkNode");

        auto transmitter = check_and_cast<Ieee80211UnitDiskTransmitter *> (parent->getSubmodule("transmitter"));
        auto distance = transmitter->getMaxCommunicationRange();
        communicationRange = distance;

        std::deque<std::pair<int,int>> links;
        std::deque<std::pair<int,int>> erased;
        for (int i = 0; i < topo.getNumNodes(); i++) {
            cTopology::Node *destNode = topo.getNode(i);
            cModule *host = destNode->getModule();
            auto  mod = check_and_cast<IMobility *>(host->getSubmodule("mobility"));
            auto cord1 = mod->getCurrentPosition();
            for (int j = i+1; j < topo.getNumNodes(); j++) {
                cTopology::Node *destNodeAux = topo.getNode(j);
                cModule *host2 = destNodeAux->getModule();
                auto  mod2 = check_and_cast<IMobility *>(host2->getSubmodule("mobility"));
                auto cord2 = mod2->getCurrentPosition();
                if (cord1.distance(cord2) < distance.get()) {
                    links.push_back(std::make_pair(host->getId(), host2->getId()));
                    links.push_back(std::make_pair(host2->getId(), host->getId()));
                }
             }
        }
        numUniLink /=100;
        numUniLink *= links.size(); //
        numUniLink = std::floor(numUniLink);

        while (numUniLink > 0) {
            int index = intuniform(0,links.size()-1);
            auto link = links[index];
            if (par("forceUni")) {
                auto itAux = std::find(erased.begin(), erased.end(), std::make_pair(link.second,link.first));
                if (itAux != erased.end()) continue;
                erased.push_back(link);
            }


            links.erase(links.begin()+index);
            auto it = uniLinks.find(link.first);
            if (it == uniLinks.end()) {
                Ieee80211UnitDiskReceiverLoss::Links l;
                l.push_back(link.second);
                uniLinks[link.first] = l;
            }
            else
                it->second.push_back(link.second);
            numUniLink--;
        }
        numLossLinks /=100;
        numLossLinks *= links.size(); //
        numLossLinks = std::floor(numLossLinks);

        while (numLossLinks > 0) {
            int index = intuniform(0,links.size()-1);
            auto link = links[index];
            links.erase(links.begin()+index);
            auto it = lossLinks.find(link.first);
            if (it == lossLinks.end()) {
                Ieee80211UnitDiskReceiverLoss::Links l;
                l.push_back(link.second);
                lossLinks[link.first] = l;
            }
            else
                it->second.push_back(link.second);
            numLossLinks--;
        }
    }
    else if (stage == INITSTAGE_PHYSICAL_LAYER_NEIGHBOR_CACHE) {
        // register the neighbors nodes
        auto cord1 = mobility->getCurrentPosition();
        for (const auto &elem : nodes) {
            if (elem.first == hostId)
                continue;
            auto cord2 = elem.second->getCurrentPosition();
            if (cord1.distance(cord2) < communicationRange.get()) {
                neigbors.push_back(elem.first);
            }
        }
    }
}

void Ieee80211UnitDiskReceiverLoss::checkNeigChange() const{
    std::vector<int> neig;
    auto cord1 = mobility->getCurrentPosition();
    for (const auto &elem : nodes) {
        if (elem.first == hostId)
            continue;
        auto cord2 = elem.second->getCurrentPosition();
        if (cord1.distance(cord2) < communicationRange.get()) {
            neig.push_back(elem.first);
        }
    }

    if (neig != neigbors) {
        auto temp1 = neigbors;
        auto temp2 = neig;
        // recompute list
        // first find the difference.
        for (auto it = temp1.begin(); it != temp1.end(); ) {
            auto it2 = std::find(temp2.begin(), temp2.end(), *it);
            if (it2 != temp2.end()) {// no change, erase both form the list
                it = temp1.erase(it);
                temp2.erase(it2);
                continue;
            }
            ++it;
        }
        // erase links from tem1, and include links from tem2
        auto uniLinkHostIt = uniLinks.find(hostId);
        auto lossLinkHostIt = lossLinks.find(hostId);
        for (const auto &elem : temp1) {
            auto uniLinkIt = uniLinks.find(elem);
            auto lossLinkIt = lossLinks.find(elem);
            if (uniLinkIt != uniLinks.end()) {
                auto it = std::find(uniLinkIt->second.begin(), uniLinkIt->second.end(), hostId);
                if (it != uniLinkIt->second.end()) {
                    uniLinkIt->second.erase(it);
                }
            }
            if (lossLinkIt != lossLinks.end()) {
                auto it = std::find(lossLinkIt->second.begin(), lossLinkIt->second.end(), hostId);
                if (it != lossLinkIt->second.end()) {
                    lossLinkIt->second.erase(it);
                }
            }
            if (uniLinkHostIt != uniLinks.end()) {
                auto it = std::find(uniLinkHostIt->second.begin(), uniLinkHostIt->second.end(), hostId);
                if (it != uniLinkHostIt->second.end()) {
                    uniLinkHostIt->second.erase(it);
                }
                if (uniLinkHostIt->second.empty()) {
                    uniLinks.erase(uniLinkHostIt);
                    uniLinkHostIt = uniLinks.end();
                }
            }
            if (lossLinkHostIt != lossLinks.end()) {
                auto it = std::find(lossLinkHostIt->second.begin(), lossLinkHostIt->second.end(), hostId);
                if (it != lossLinkHostIt->second.end()) {
                    lossLinkHostIt->second.erase(it);
                }
                if (lossLinkHostIt->second.empty()) {
                    lossLinks.erase(uniLinkHostIt);
                    lossLinkHostIt = lossLinks.end();
                }
            }
            if (uniLinkIt->second.empty())
                uniLinks.erase(uniLinkIt);
            if (lossLinkIt->second.empty())
                lossLinks.erase(lossLinkIt);
        }

        double numUniLink = par("perUniLinks").doubleValue();
        double numLossLinks = par("perLosLinks").doubleValue();

        for (const auto &elem : temp2) {
            int val1 = intuniform(0,1);
            int val2 = uniform(0, 100);
            if (val2 < numUniLink) {
                if (val1) {
                    uniLinks[hostId].push_back(elem);
                }
                else {
                    uniLinks[elem].push_back(hostId);
                }
                continue;
            }
            if (val2 < numLossLinks) {
                if (val1) {
                    lossLinks[hostId].push_back(elem);
                }
                else {
                    lossLinks[elem].push_back(hostId);
                }
                continue;
            }
        }
        auto ptr = const_cast<std::vector<int> *> (&neigbors);
        *ptr = neig;
    }
}

const IReceptionResult *Ieee80211UnitDiskReceiverLoss::computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const
{
    checkNeigChange();

    auto receptionResult = Ieee80211UnitDiskReceiver::computeReceptionResult(listening, reception, interference, snir, decisions);

    auto txNode = findContainingNode(check_and_cast<Radio *>(const_cast<IRadio *>(reception->getTransmission()->getTransmitter())));
    auto rxNode = findContainingNode(check_and_cast<Radio *>(const_cast<IRadio *>(reception->getReceiver())));

    auto packet = const_cast<Packet *>(receptionResult->getPacket());

    auto uniLinkIt = uniLinks.find(txNode->getId());
    auto lossLinkIt = lossLinks.find(txNode->getId());

    if (uniLinkIt == uniLinks.end() && lossLinkIt == lossLinks.end())
        return receptionResult;
    if (uniLinkIt != uniLinks.end()) {
        auto it = std::find(uniLinkIt->second.begin(), uniLinkIt->second.end(), rxNode->getId());
        if (it != uniLinkIt->second.end()) {
            packet->setBitError(true);
        }
    }

    if (lossLinkIt != lossLinks.end()) {
        auto it = std::find(lossLinkIt->second.begin(), lossLinkIt->second.end(), rxNode->getId());
        if (it != lossLinkIt->second.end()) {
            double pr = dblrand();
            if (par("errorProb").doubleValue() > pr)
                packet->setBitError(true);
        }
    }
    return receptionResult;
}

} // namespace physicallayer
} // namespace inet

