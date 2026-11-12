//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_GNBMAC_H
#define __INET_GNBMAC_H

#include "inet/prototype/cellular/CarrierGrid.h"
#include "inet/prototype/cellular/UeMac.h"

namespace inet {

namespace cellular {

class PanelAntenna;

/**
 * The dummy MAC entity of a base station sector, above one radio per component carrier. Each
 * slot of each carrier, it gives the PRBs to a few attached UEs round-robin, sends one
 * downlink allocation per UE, and grants uplink allocations for a later slot.
 */
class INET_API GnbMac : public SimpleModule
{
  protected:
    struct Carrier {
        IRadio *radio = nullptr;
        const CellularTransmitter *transmitter = nullptr;
        const PanelAntenna *panel = nullptr;
        CarrierGrid grid;
        bool isTdd = false;
        cMessage *slotTimer = nullptr;
        long slotNumber = 0;
        int nextDownlinkUe = 0;
        int nextUplinkUe = 0;
    };

    struct UeState {
        UeMac *ueMac = nullptr;
        // the last effective SNIR of each carrier in dB, for link adaptation
        std::vector<double> downlinkSnirs;
        std::vector<double> uplinkSnirs;
    };

    // parameters
    IdealControlChannel *controlChannel = nullptr;
    int scheduledUesPerSlot = -1;
    std::string tddPattern;
    int downlinkFirstSymbol = -1;
    int downlinkNumSymbols = -1;
    int uplinkNumSymbols = -1;
    int uplinkGrantDelay = -1;
    bool bitmapAllocation = false;
    int rbgSize = -1;
    double mcsMargin = NaN;

    // state
    std::vector<Carrier> carriers;
    std::vector<UeState> ues;
    std::map<int, int> ueIndexToUe;

    static simsignal_t transportBlockReceivedSignal;
    static simsignal_t transportBlockFailedSignal;
    static simsignal_t effectiveSnirMeasuredSignal;

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *message) override;

    // 'D', 'S' or 'U' of the TDD pattern, or 'F' for an FDD carrier
    virtual char getSlotType(const Carrier& carrier, long slotNumber) const;
    // the PRB sets of the UEs of one slot, contiguous blocks or interleaved RBGs
    virtual std::vector<std::vector<int>> dividePrbs(int numPrbs, int numUes, bool interleaved) const;

    virtual void handleSlot(int carrierIndex);
    virtual void scheduleDownlink(int carrierIndex);
    virtual void scheduleUplink(int carrierIndex, simtime_t startTime);
    virtual void handleLowerPacket(Packet *packet);

  public:
    virtual ~GnbMac();

    // how well a UE at the position hears this sector: the element gain of the panel of the
    // first carrier minus a distance term, in dB
    virtual double computeAttachmentMetric(const Coord& position) const;

    virtual void attach(UeMac *ueMac);
    virtual void reportDownlinkSnir(int ueIndex, int carrierIndex, double snir);
};

} // namespace cellular

} // namespace inet

#endif

