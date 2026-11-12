//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_UEMAC_H
#define __INET_UEMAC_H

#include "inet/common/SimpleModule.h"
#include "inet/common/packet/Packet.h"
#include "inet/physicallayer/wireless/common/contract/packetlevel/IRadio.h"

namespace inet {

namespace cellular {

using namespace inet::physicallayer;

class CellularTransmitter;
class GnbMac;
class IdealControlChannel;

/**
 * An uplink allocation that a GnbMac gives to a UeMac.
 */
struct UplinkGrant
{
    int carrierIndex = -1;
    simtime_t startTime;
    std::vector<int> prbs;
    int numSymbols = 0;
    int mcs = 0;
    int intendedReceiverId = -1;
};

/**
 * The dummy MAC entity of a UE, above one radio per component carrier. It sends the uplink
 * transport blocks that the serving GnbMac grants, sharing its maximum transmission power among
 * the carriers, and reports the effective SNIR of each downlink transport block.
 */
class INET_API UeMac : public SimpleModule
{
  protected:
    struct Carrier {
        IRadio *radio = nullptr;
        const CellularTransmitter *transmitter = nullptr;
    };

    IdealControlChannel *controlChannel = nullptr;
    int ueIndex = -1;
    W maxTransmissionPower = W(NaN);
    std::vector<Carrier> carriers;
    GnbMac *servingGnbMac = nullptr;
    // the grants of each future transmission time, one timer per time
    std::map<simtime_t, std::vector<UplinkGrant>> pendingGrants;

    static simsignal_t transportBlockReceivedSignal;
    static simsignal_t transportBlockFailedSignal;
    static simsignal_t effectiveSnirMeasuredSignal;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    virtual void attachToBestCell();
    virtual void transmitGrants(const std::vector<UplinkGrant>& grants);
    virtual void handleLowerPacket(Packet *packet);

  public:
    int getUeIndex() const { return ueIndex; }
    int getNumCarriers() const { return carriers.size(); }
    IRadio *getRadio(int carrierIndex) const { return carriers.at(carrierIndex).radio; }

    virtual void grantUplink(const UplinkGrant& grant);
};

} // namespace cellular

} // namespace inet

#endif

