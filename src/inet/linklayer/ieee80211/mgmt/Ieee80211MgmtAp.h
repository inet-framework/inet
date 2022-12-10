//
// Copyright (C) 2006 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


#ifndef __INET_IEEE80211MGMTAP_H
#define __INET_IEEE80211MGMTAP_H

#include <map>

#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtApBase.h"

namespace inet {

namespace ieee80211 {

/**
 * Used in 802.11 infrastructure mode: handles management frames for
 * an access point (AP). See corresponding NED file for a detailed description.
 */
class INET_API Ieee80211MgmtAp : public Ieee80211MgmtApBase, protected cListener
{
  public:
    /** Describes a STA */
    struct StaInfo {
        MacAddress address;
        int authSeqExpected; // when NOT_AUTHENTICATED: transaction sequence number of next expected auth frame
//        int consecFailedTrans; // TODO
//        double expiry; // TODO association should expire after a while if STA is silent?
    };

    class INET_API NotificationInfoSta : public cObject {
        MacAddress apAddress;
        MacAddress staAddress;

      public:
        void setApAddress(const MacAddress& a) { apAddress = a; }
        void setStaAddress(const MacAddress& a) { staAddress = a; }
        const MacAddress& getApAddress() const { return apAddress; }
        const MacAddress& getStaAddress() const { return staAddress; }
    };

    class INET_API MacCompare {
      public:
        bool operator()(const MacAddress& u1, const MacAddress& u2) const { return u1.compareTo(u2) < 0; }
    };
    typedef std::map<MacAddress, StaInfo, MacCompare> StaList;

  protected:
    // configuration
    std::string ssid;
    int channelNumber = -1;
    simtime_t beaconInterval;
    int numAuthSteps = 0;
    Ieee80211SupportedRatesElement supportedRates;

    // state
    StaList staList; ///< list of STAs
    cMessage *beaconTimer = nullptr;

  public:
    Ieee80211MgmtAp() {}
    virtual ~Ieee80211MgmtAp();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int) override;

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleTimer(cMessage *msg) override;

    /** Implements abstract Ieee80211MgmtBase method -- throws an error (no commands supported) */
    virtual void handleCommand(int msgkind, cObject *ctrl) override;

    /** Called by the signal handler whenever a change occurs we're interested in */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, intval_t value, cObject *details) override;

    /** Utility function: return sender STA's entry from our STA list, or nullptr if not in there */
    virtual StaInfo *lookupSenderSTA(const Ptr<const Ieee80211MgmtHeader>& header);

    /** Utility function: set fields in the given frame and send it out to the address */
    virtual void sendManagementFrame(const char *name, const Ptr<Ieee80211MgmtFrame>& body, int subtype, const MacAddress& destAddr);

    /** Utility function: creates and sends a beacon frame */
    virtual void sendBeacon();

    /** @name Processing of different frame types */
    //@{
    virtual void handleAuthenticationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleDeauthenticationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleAssociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleAssociationResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleReassociationRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleReassociationResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleDisassociationFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleBeaconFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleProbeRequestFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    virtual void handleProbeResponseFrame(Packet *packet, const Ptr<const Ieee80211MgmtHeader>& header) override;
    //@}

    void sendAssocNotification(const MacAddress& addr);

    void sendDisAssocNotification(const MacAddress& addr);

    /** lifecycle support */
    //@{

  protected:
    virtual void start() override;
    virtual void stop() override;
    //@}
};

} // namespace ieee80211

} // namespace inet

#endif

