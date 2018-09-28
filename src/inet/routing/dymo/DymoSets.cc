//
// Copyright (C) 2016
// Author: Alfonso Ariza
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#include "DymoSets.h"
#include "inet/routing/dymo/Dymo_m.h"

namespace inet {

namespace dymo {

DymoSets::DymoSets()
{
}

bool DymoSets::check(const Ptr<const RteMsg>& rteMsg) {
    if (!active)
        return false;

    // Only check RREQ
    const auto & rreq = dynamicPtrCast<const Rreq>(rteMsg);
    if (rreq != nullptr)
        return checkRREQ(rreq);

    const auto & rerr = dynamicPtrCast<const Rerr>(rteMsg);
    if (rerr != nullptr)
        return checkRERR(rerr);

    return false;
}


bool DymoSets::checkRREQ(const Ptr<const Rreq>& rreq) {
    if (!active)
        return false;

    // Only check RREQ
    const AddressBlock& originatorNode = rreq->getOriginatorNode();
    const AddressBlock& targetNode = rreq->getTargetNode();

    // ignore self messages
    if (selfAddress == originatorNode.getAddress())
        return true;

    OriginatorAddressPair addr;
    addr.orPrefixLen = originatorNode.getPrefixLength();
    addr.prefixAddr = originatorNode.getAddress();
    auto it = multicastRouteSet.find(addr);
    if (it == multicastRouteSet.end()) {
        // include and return false
        MulticastRouteInfo info;
        info.hasMetricType = originatorNode.getHasMetric();
        info.mType = originatorNode.getMetricType();
        info.orSequenceNumber = originatorNode.getSequenceNumber();
        info.metric = originatorNode.getMetric();
        info.target = targetNode.getAddress();
        info.trSequenceNumber = targetNode.getSequenceNumber();
        info.timeStamp = simTime();
        info.removeTime = simTime() + lifetime;
        MulticastRouteVect v;
        v.push_back(info);
        multicastRouteSet.insert(std::make_pair(addr, v));
        return false;
    }
    else {
        for (auto itAux = it->second.begin(); itAux != it->second.end();) {
            if (itAux->removeTime < simTime()) {
                itAux = it->second.erase(itAux);
                continue;
            }
            if (itAux->mType != originatorNode.getMetricType()
                    || itAux->target != targetNode.getAddress()) {
                ++itAux;
                continue;
            }
            if (itAux->mType == originatorNode.getMetricType()
                    && itAux->target == targetNode.getAddress()) {
                // check sequence
                if (itAux->orSequenceNumber > originatorNode.getSequenceNumber()
                        || (itAux->orSequenceNumber
                                == originatorNode.getSequenceNumber()
                                && itAux->metric <= originatorNode.getMetric())) {

                    itAux->timeStamp = simTime();
                    itAux->removeTime = simTime() + lifetime;
                    EV_DETAIL << "Redundant RREQ: originator = "
                                     << originatorNode.getAddress()
                                     << ", target = " << targetNode.getAddress()
                                     << endl;
                    return true;
                }
                else {
                    // actualize and return false
                    itAux->orSequenceNumber =
                            originatorNode.getSequenceNumber();
                    itAux->metric = originatorNode.getMetric();
                    itAux->timeStamp = simTime();
                    itAux->removeTime = simTime() + lifetime;
                    return false;
                }
            }
            else
                throw cRuntimeError("Never ");
        }

        // include and return
        // include and return false
        MulticastRouteInfo info;
        info.hasMetricType = originatorNode.getHasMetric();
        info.mType = originatorNode.getMetricType();
        info.orSequenceNumber = originatorNode.getSequenceNumber();
        info.metric = originatorNode.getMetric();
        info.target = targetNode.getAddress();
        info.trSequenceNumber = targetNode.getSequenceNumber();
        info.timeStamp = simTime();
        info.removeTime = simTime() + lifetime;
        it->second.push_back(info);
        return false;
    }
}

bool DymoSets::checkRERR(const Ptr<const Rerr> &rerr) {

    if (!active)
        return false;

    ErrorRouteInfo info;
    // info.source = rerr->getPktSource();
    std::vector<AddressBlock> remove;
    for (unsigned int i = 0; i < rerr->getUnreachableNodeArraySize(); i++) {
        const AddressBlock& addressBlock = rerr->getUnreachableNode(i);
        info.unreachableAddress = addressBlock.getAddress();
        auto it = errorRouteSet.find(info);
        if (it == errorRouteSet.end() || it->timeout < simTime()) {
            info.timeout = simTime() + rerrTimeout;
            errorRouteSet.insert(info);
        }
        else {
            remove.push_back(addressBlock);
        }
    }
    if (remove.size() == rerr->getUnreachableNodeArraySize())
        return true;
    return false;
}


} // namespace dymo

} // namespace inet

