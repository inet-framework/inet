/* -*- mode:c++ -*- ********************************************************
 * file:        Ieee802154MacLoss.cc
 *
 * author:      Jerome Rousselot, Marcel Steine, Amre El-Hoiydi,
 *              Marc Loebbers, Yosia Hadisusanto, Andreas Koepke, Alfonso Ariza
 *
  *copyright:   (C) 2019 Universidad de Málaga
 *              (C) 2007-2009 CSEM SA
 *              (C) 2009 T.U. Eindhoven
 *              (C) 2004,2005,2006
 *              Telecommunication Networks Group (TKN) at Technische
 *              Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 *
 * Funding: This work was partially financed by the European Commission under the
 * Framework 6 IST Project "Wirelessly Accessible Sensor Populations"
 * (WASP) under contract IST-034963.
 ***************************************************************************
 * part of:    Modifications to the MF-2 framework by CSEM
 **************************************************************************/
#include <cassert>

#include "inet/common/FindModule.h"
#include "inet/common/INETMath.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"
#include "inet/linklayer/ieee802154Loss/Ieee802154MacLoss.h"
#include "inet/linklayer/ieee802154/Ieee802154MacHeader_m.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/common/WirelessGetNeig.h"
#include "inet/physicallayer/base/packetlevel/NarrowbandTransmitterBase.h"
#include "inet/physicallayer/contract/packetlevel/IRadioMedium.h"

namespace inet {

using namespace physicallayer;

Define_Module(Ieee802154MacLoss);

void Ieee802154MacLoss::initialize(int stage)
{
    Ieee802154Mac::initialize(stage);
    if (stage == INITSTAGE_NETWORK_LAYER) {
        // Read topology and set probabilities.
        WirelessGetNeig *neig = new WirelessGetNeig();
        auto transmitter = radio->getTransmitter();
        auto narrow = dynamic_cast<const NarrowbandTransmitterBase *>(transmitter);
        auto carrierFrequency = narrow->getCarrierFrequency();

        auto power = narrow->getMaxPower();
        auto sens = radio->getReceiver()->getMinReceptionPower();
        // compute distance using free space
        auto radioMedium = radio->getMedium();
        auto pathLoss = radioMedium->getPathLoss();
        double loss = unit(sens / power).get();
        auto dist = pathLoss->computeRange(radioMedium->getPropagation()->getPropagationSpeed(), carrierFrequency, loss);

        std::vector<L3Address> list;
        neig->getNeighbours(L3Address(interfaceEntry->getMacAddress()), list, dist.get());
        double probabilityCompleteFailureLink = par("probabilityCompleteFailureLink"); // (between 0..1) percentage of links that will fail in every node
        double probabilityLink = par("probabilityLink"); // (between 0..1) percentage of links that will lost packet in every node
        double probabilityFailurePacket = par("probabilityFailurePacket"); // (between 0..1) probability of loss in link with loss.
        for (auto elem: list) {
            if (probabilityCompleteFailureLink != 0 && dblrand() <  probabilityCompleteFailureLink)
                probability[elem] = 1.0;
            else if (probabilityLink != 0 && dblrand() <  probabilityLink)
                probability[elem] = probabilityFailurePacket;
        }
        delete neig;
    }
}

bool Ieee802154MacLoss::discard(const L3Address &addr) {

    auto it = probability.find(addr);
    if (it == probability.end())
        return false;
    if (it->second >= 1.0)
        return true;
    if (it->second > dblrand())
        return true;
    return false;
}

Ieee802154MacLoss::~Ieee802154MacLoss()
{
}



void Ieee802154MacLoss::sendUp(cMessage *message)
{
    auto packet = check_and_cast<Packet *> (message);
    auto  src = packet->getTag<MacAddressInd>()->getSrcAddress();
    // check if the packet must be discarded

    if (discard(src))
        delete message;
    else
        MacProtocolBase::sendUp(message);
}

} // namespace inet

