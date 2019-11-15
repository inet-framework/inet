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

#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211Tag_m.h"
#include "inet/physicallayer/base/packetlevel/FlatTransmissionBase.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ScalarReceiverLoss.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ScalarTransmission.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ScalarTransmitter.h"


namespace inet {

namespace physicallayer {
std::map<int, Ieee80211ScalarReceiverLoss::Links> Ieee80211ScalarReceiverLoss::uniLinks;
std::map<int, Ieee80211ScalarReceiverLoss::Links> Ieee80211ScalarReceiverLoss::lossLinks;

Define_Module(Ieee80211ScalarReceiverLoss);

void Ieee80211ScalarReceiverLoss::initialize(int stage)
{
    Ieee80211ScalarReceiver::initialize(stage);
    if (stage == INITSTAGE_PHYSICAL_LAYER_NEIGHBOR_CACHE) {

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

        auto parent = check_and_cast<Radio *>(this->getParentModule());
        auto transmitter = check_and_cast<Ieee80211ScalarTransmitter *> (parent->getSubmodule("transmitter"));
        auto sens = this->sensitivity.get();
        auto pw = transmitter->getPower().get();
        auto fq = transmitter->getCenterFrequency().get();
        auto gr = parent->getAntenna()->getGain()->getMaxGain();
        auto distance = transmitter->getMaxCommunicationRange().get();
        double landa = 300000000.0/ fq;
        double loss = pw/sens;

        loss *= gr;
        loss = std::sqrt(loss);

        distance = (loss * landa)/(4 * PI);



        std::deque<std::pair<int,int>> links;
        std::deque<std::pair<int,int>> erased;
        int nodes = topo.getNumNodes();
        for (int i = 0; i < nodes; i++) {
            cTopology::Node *destNode = topo.getNode(i);
            cModule *host = destNode->getModule();
            auto  mod = check_and_cast<IMobility *>(host->getSubmodule("mobility"));
            auto cord1 = mod->getCurrentPosition();
            for (int j = i+1; j < nodes; j++) {
                cTopology::Node *destNodeAux = topo.getNode(j);
                cModule *host2 = destNodeAux->getModule();
                auto  mod2 = check_and_cast<IMobility *>(host2->getSubmodule("mobility"));
                auto cord2 = mod2->getCurrentPosition();
                double d = cord1.distance(cord2);
                if (d < distance) {
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
                Ieee80211ScalarReceiverLoss::Links l;
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
                Ieee80211ScalarReceiverLoss::Links l;
                l.push_back(link.second);
                lossLinks[link.first] = l;
            }
            else
                it->second.push_back(link.second);
            numLossLinks--;
        }
    }
}

Ieee80211ScalarReceiverLoss::Ieee80211ScalarReceiverLoss() :
        Ieee80211ScalarReceiver()
{
}

const IReceptionResult *Ieee80211ScalarReceiverLoss::computeReceptionResult(const IListening *listening, const IReception *reception, const IInterference *interference, const ISnir *snir, const std::vector<const IReceptionDecision *> *decisions) const
{

    auto receptionResult = Ieee80211ScalarReceiver::computeReceptionResult(listening, reception, interference, snir, decisions);

    auto txNode = findContainingNode(check_and_cast<Radio *>(const_cast<IRadio *>(reception->getTransmission()->getTransmitter())));
    auto rxNode = findContainingNode(check_and_cast<Radio *>(const_cast<IRadio *>(reception->getReceiver())));

    auto packet = const_cast<Packet *>(receptionResult->getPacket());

    int txId = txNode->getId();
    int rxId = rxNode->getId();

    auto cord1 = reception->getTransmission()->getStartPosition();
    auto cord2 = reception->getStartPosition();
    //double dist = cord1.distance(cord2);


    auto uniLinkIt = uniLinks.find(txId);
    auto lossLinkIt = lossLinks.find(txId);

    if (uniLinkIt == uniLinks.end() && lossLinkIt == lossLinks.end())
        return receptionResult;
    if (uniLinkIt != uniLinks.end()) {
        auto it = std::find(uniLinkIt->second.begin(), uniLinkIt->second.end(), rxId);
        if (it != uniLinkIt->second.end()) {
            packet->setBitError(true);
        }
    }

    if (lossLinkIt != lossLinks.end()) {
        auto it = std::find(lossLinkIt->second.begin(), lossLinkIt->second.end(), rxId);
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

