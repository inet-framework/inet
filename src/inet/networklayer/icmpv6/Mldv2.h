//
// Copyright (C) 2026 OpenSim Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_MLDV2_H
#define __INET_MLDV2_H

#include <map>
#include <set>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/checksum/ChecksumMode_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/lifecycle/OperationalBase.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/stlutils.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/ipv6/Ipv6Address.h"
#include "inet/networklayer/icmpv6/Mldv2Message_m.h"

namespace inet {

class IInterfaceTable;
class Ipv6RoutingTable;
struct Ipv6MulticastSourceList;

// Self-message kinds (declared at namespace scope so Mldv2.ned's
// @selfMessageKinds(inet::Mldv2TimerKind) can name it for Qtenv).
enum Mldv2TimerKind {
    MLDV2_R_GENERAL_QUERY_TIMER,
    MLDV2_R_GROUP_TIMER,
    MLDV2_R_SOURCE_TIMER,
    MLDV2_R_REXMT_TIMER,
    MLDV2_R_OLDER_VERSION_TIMER, // Older Version Host Present timer (RFC 3810 8.3.2), per RouterGroupData
    MLDV2_H_GENERAL_QUERY_TIMER,
    MLDV2_H_GROUP_TIMER,
    MLDV2_H_STATE_CHANGE_TIMER,
    MLDV2_H_OLDER_VERSION_TIMER, // Older Version Querier Present timer (RFC 3810 8.2.1), per HostInterfaceData
    MLDV2_H_OLDER_VERSION_REPORT_TIMER, // report delay timer of the MLDv1 mode (RFC 2710 section 5), per HostGroupData
};

class INET_API Mldv2 : public OperationalBase, protected cListener
{
  protected:
    enum RouterState {
        MLDV2_RS_INITIAL,
        MLDV2_RS_QUERIER,
        MLDV2_RS_NON_QUERIER,
    };

    enum RouterGroupState {
        MLDV2_RGS_NO_MEMBERS_PRESENT,
        MLDV2_RGS_MEMBERS_PRESENT,
        MLDV2_RGS_CHECKING_MEMBERSHIP,
    };

    enum HostGroupState {
        MLDV2_HGS_NON_MEMBER,
        MLDV2_HGS_DELAYING_MEMBER,
        MLDV2_HGS_IDLE_MEMBER,
    };

    enum FilterMode {
        MLDV2_FM_INCLUDE,
        MLDV2_FM_EXCLUDE,
    };

    struct HostInterfaceData;

    struct HostGroupData {
        HostInterfaceData *parent;
        Ipv6Address groupAddr;
        FilterMode filter;
        Ipv6AddressVector sourceAddressList; // sorted
        HostGroupState state;
        cMessage *timer; // for scheduling responses to Multicast-Address-Specific and -and-Source-Specific Queries
        Ipv6AddressVector queriedSources; // saved from last Multicast-Address-Specific or -and-Source-Specific Query; sorted

        // State-Change Report retransmission (RFC 9777 section 6.1): a State-Change Report holds
        // a Filter-Mode-Change record while filterModeChangeCount is positive, else the
        // Source-List-Change records of the sources with a positive count. Every State-Change
        // Report that leaves decrements the counts.
        int filterModeChangeCount = 0; // State-Change Reports that must still hold a Filter-Mode-Change record
        std::map<Ipv6Address, int> sourceChangeCounts; // per source: State-Change Reports that must still hold it
        cMessage *retransmitTimer; // fires at uniform(0, unsolicitedReportInterval)

        // The MLDv1 mode (RFC 2710 section 5): the report delay timer runs in the Delaying
        // Listener state, and the flag tells that this node sent the last Report.
        cMessage *olderVersionReportTimer;
        bool lastReporter = false;

        HostGroupData(HostInterfaceData *parent, const Ipv6Address& group);
        virtual ~HostGroupData();
        std::string getStateInfo() const;
    };

    typedef std::map<Ipv6Address, HostGroupData *> GroupToHostDataMap;

    struct HostInterfaceData {
        Mldv2 *owner;
        NetworkInterface *ie;
        GroupToHostDataMap groups;
        cMessage *generalQueryTimer; // for scheduling responses to General Queries

        // Older Version Querier Present (RFC 3810 8.2.1): while olderVersionTimer is
        // scheduled, an MLDv1 querier is present on this interface and the host emits
        // MLDv1 Reports/Dones instead of MLDv2 reports.
        cMessage *olderVersionTimer; // fires at the Older Version Querier Present Interval
        bool olderVersionPresent = false;

        HostInterfaceData(Mldv2 *owner, NetworkInterface *ie);
        virtual ~HostInterfaceData();
        HostGroupData *getOrCreateGroupData(const Ipv6Address& group);
        void deleteGroupData(const Ipv6Address& group);
        friend inline std::ostream& operator<<(std::ostream& out, const Mldv2::HostInterfaceData& entry)
        {
            for (auto& g : entry.groups) {
                out << "(groupAddress: " << g.second->groupAddr << " ";
                out << "hostGroupState: " << Mldv2::getHostGroupStateString(g.second->state) << " ";
                out << "groupTimer: " << g.second->timer->getArrivalTime() << " ";
                out << "queriedSources: ";
                for (auto& entry : g.second->queriedSources)
                    out << entry << ", ";
                out << "sourceAddressList: ";
                for (auto& entry : g.second->sourceAddressList)
                    out << entry << ", ";
                out << "filter: " << Mldv2::getFilterModeString(g.second->filter) << ") ";
            }

            return out;
        }
    };

    struct RouterInterfaceData;
    struct RouterGroupData;

    struct SourceRecord {
        RouterGroupData *parent;
        Ipv6Address sourceAddr;
        cMessage *sourceTimer;

        SourceRecord(RouterGroupData *parent, const Ipv6Address& source);
        virtual ~SourceRecord();
    };

    typedef std::map<Ipv6Address, SourceRecord *> SourceToSourceRecordMap;

    struct RouterGroupData {
        RouterInterfaceData *parent;
        Ipv6Address groupAddr;
        FilterMode filter;
        RouterGroupState state;
        cMessage *timer;
        SourceToSourceRecordMap sources;

        // Query retransmission (RFC 9777 section 7.6.3): a Multicast Address Specific Query is
        // sent [Last Listener Query Count] times, and a source of a Multicast Address and
        // Source Specific Query is in [Last Listener Query Count] of them,
        // lastMemberQueryInterval apart. A Report changes only the timers, and so the S flags.
        cMessage *rexmtTimer; // fires at lastMemberQueryInterval
        int groupRexmtCount = 0; // Multicast Address Specific Queries still to send
        std::map<Ipv6Address, int> sourceRexmtCounts; // per source: Multicast Address and Source Specific Queries still to hold it

        // Older Version Host Present (RFC 3810 8.3.2): while olderVersionTimer is
        // scheduled, an MLDv1 host is present for this group. The group is forwarded as
        // EXCLUDE{} (any-source) and MLDv2 per-source processing is bypassed.
        cMessage *olderVersionTimer; // fires at olderVersionHostPresentInterval
        bool olderVersionPresent = false;

        RouterGroupData(RouterInterfaceData *parent, const Ipv6Address& group);
        virtual ~RouterGroupData();
        bool hasSourceRecord(const Ipv6Address& source) { return containsKey(sources, source); }
        SourceRecord *createSourceRecord(const Ipv6Address& source);
        SourceRecord *getOrCreateSourceRecord(const Ipv6Address& source);
        void deleteSourceRecord(const Ipv6Address& source);
        std::string getStateInfo() const;
        void collectForwardedSources(Ipv6MulticastSourceList& result) const;

      private:
        void printSourceList(std::ostream& out, bool withRunningTimer) const;
    };

    typedef std::map<Ipv6Address, RouterGroupData *> GroupToRouterDataMap;

    struct RouterInterfaceData {
        Mldv2 *owner;
        NetworkInterface *ie;
        GroupToRouterDataMap groups;
        RouterState state;
        bool versionWarned = false; // the warning about a Query of another version is given once
        cMessage *generalQueryTimer;
        bool waitsForLinkLocalAddress = false; // the querier starts when the interface has a tested link-local address

        RouterInterfaceData(Mldv2 *owner, NetworkInterface *ie);
        virtual ~RouterInterfaceData();
        RouterGroupData *getOrCreateGroupData(const Ipv6Address& group);
        void deleteGroupData(const Ipv6Address& group);
        friend inline std::ostream& operator<<(std::ostream& out, const Mldv2::RouterInterfaceData& entry)
        {
            out << "routerState: " << Mldv2::getRouterStateString(entry.state) << " ";
            out << "queryTimer: " << entry.generalQueryTimer->getArrivalTime() << " ";
            if (entry.groups.empty())
                out << "(empty)";
            else {
                for (auto& g : entry.groups) {
                    out << "(groupAddress: " << g.second->groupAddr << " ";
                    out << "routerGroupState: " << Mldv2::getRouterGroupStateString(g.second->state) << " ";
                    out << "timer: " << g.second->timer->getArrivalTime() << " ";
                    out << "filter: " << Mldv2::getFilterModeString(g.second->filter) << ") ";
                }
            }

            return out;
        }
    };

  protected:
    ModuleRefByPar<Ipv6RoutingTable> rt;
    ModuleRefByPar<IInterfaceTable> ift;

    bool enabled = true;
    int robustnessVariable; // RFC 3810: a State-Change Report is (re)transmitted this many times
    double queryInterval;
    double queryResponseInterval;
    double groupMembershipInterval;
    double otherQuerierPresentInterval;
    double olderVersionHostPresentInterval;
    int routerVersion;
    double startupQueryInterval;
    int startupQueryCount;
    double lastMemberQueryInterval;
    int lastMemberQueryCount;
    double lastMemberQueryTime;
    double unsolicitedReportInterval;

    // checksumMode
    ChecksumMode checksumMode = CHECKSUM_MODE_UNDEFINED;

    // group counters
    int numGroups = 0;
    int numHostGroups = 0;
    int numRouterGroups = 0;

    // message counters
    int numQueriesSent = 0;
    int numQueriesRecv = 0;
    int numGeneralQueriesSent = 0;
    int numGeneralQueriesRecv = 0;
    int numGroupSpecificQueriesSent = 0;
    int numGroupSpecificQueriesRecv = 0;
    int numGroupAndSourceSpecificQueriesSent = 0;
    int numGroupAndSourceSpecificQueriesRecv = 0;
    int numReportsSent = 0;
    int numReportsRecv = 0;

    typedef std::map<int, HostInterfaceData *> InterfaceToHostDataMap;
    typedef std::map<int, RouterInterfaceData *> InterfaceToRouterDataMap;

    // state variables per interface
    InterfaceToHostDataMap hostData;
    InterfaceToRouterDataMap routerData;

  public:
    static Ipv6AddressVector set_complement(const Ipv6AddressVector& first, const Ipv6AddressVector& second);
    static Ipv6AddressVector set_intersection(const Ipv6AddressVector& first, const Ipv6AddressVector& second);
    static Ipv6AddressVector set_union(const Ipv6AddressVector& first, const Ipv6AddressVector& second);

    static const std::string getRouterStateString(RouterState rs);
    static const std::string getRouterGroupStateString(RouterGroupState rgs);
    static const std::string getHostGroupStateString(HostGroupState hgs);
    static const std::string getFilterModeString(FilterMode fm);

  protected:
    virtual bool isInitializeStage(int stage) const override { return stage == INITSTAGE_NETWORK_LAYER; }
    virtual bool isModuleStartStage(int stage) const override { return stage == ModuleStartOperation::STAGE_NETWORK_LAYER; }
    virtual bool isModuleStopStage(int stage) const override { return stage == ModuleStopOperation::STAGE_NETWORK_LAYER; }
    virtual void handleStartOperation(LifecycleOperation *operation) override {}
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    // the Reports and Dones that wait for a tested link-local address of their interface
    std::map<int, std::vector<Packet *>> heldMessages;
    virtual void sendHeldMessages(NetworkInterface *ie);
    virtual void deleteHeldMessages(int interfaceId);
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual ~Mldv2();

  protected:
    void addWatches();

    virtual HostInterfaceData *createHostInterfaceData(NetworkInterface *ie);
    virtual RouterInterfaceData *createRouterInterfaceData(NetworkInterface *ie);
    virtual HostInterfaceData *getHostInterfaceData(NetworkInterface *ie);
    virtual RouterInterfaceData *getRouterInterfaceData(NetworkInterface *ie);
    virtual void deleteHostInterfaceData(int interfaceId);
    virtual void deleteRouterInterfaceData(int interfaceId);

    virtual void configureInterface(NetworkInterface *ie);

    virtual void startTimer(cMessage *timer, double interval);

    virtual void sendGeneralQuery(RouterInterfaceData *interface, double maxRespTime);
    // the QRV and the QQIC of every Query: the Robustness Variable and the Query Interval of the
    // querier (RFC 9777 sections 5.1.8 and 5.1.9)
    virtual void setQuerierFields(const Ptr<Mldv2Query>& query);
    virtual void sendGroupSpecificQuery(RouterGroupData *group);
    virtual void sendGroupAndSourceSpecificQuery(RouterGroupData *group, const Ipv6AddressVector& sources);
    virtual void sendSourceSpecificQueries(RouterGroupData *group);
    virtual void sendOlderVersionQuery(RouterInterfaceData *interfaceData, const Ipv6Address& groupAddr, double maxRespDelay);
    virtual void warnAboutQueryVersion(RouterInterfaceData *interfaceData, int queryVersion);
    virtual void scheduleQueryRetransmission(RouterGroupData *group);
    virtual void sendGroupReport(NetworkInterface *ie, const std::vector<Mldv2MulticastAddressRecord>& records);
    virtual void sendStateChangeReport(HostGroupData *group);
    virtual std::vector<Mldv2MulticastAddressRecord> buildStateChangeRecords(HostGroupData *group);
    virtual void sendQueryToIPv6(Packet *msg, NetworkInterface *ie, const Ipv6Address& dest);
    virtual void sendReportToIPv6(Packet *msg, NetworkInterface *ie, const Ipv6Address& dest);
    virtual void sendToIPv6(Packet *msg, NetworkInterface *ie, const Ipv6Address& dest);

    virtual void processHostGeneralQueryTimer(cMessage *msg);
    virtual void processHostGroupQueryTimer(cMessage *msg);
    virtual void processHostStateChangeTimer(cMessage *msg);
    virtual void processHostOlderVersionTimer(cMessage *msg);
    virtual void processHostOlderVersionReportTimer(cMessage *msg);
    virtual void startOlderVersionReportTimer(HostGroupData *group, double maxResponseDelay);
    virtual void cancelHostTimers(HostInterfaceData *interfaceData, bool mldv1Mode);
    virtual void processRouterGeneralQueryTimer(cMessage *msg);
    virtual void processRouterGroupTimer(cMessage *msg);
    virtual void processRouterSourceTimer(cMessage *msg);
    virtual void processRexmtTimer(cMessage *msg);
    virtual void processRouterOlderVersionTimer(cMessage *msg);

    virtual void processMldMessage(Packet *msg);
    virtual void processQuery(Packet *msg);
    virtual void processReport(Packet *msg);

    // --- Older-version (MLDv1) interop (RFC 3810 8.2/8.3) ---
    // Host side: enter MLDv1 compatibility on an MLDv1 General Query, and emit MLDv1
    // Reports/Dones while it is active.
    virtual void processOlderVersionQuery(NetworkInterface *ie, Packet *packet);
    virtual void sendOlderVersionReport(NetworkInterface *ie, const Ipv6Address& group);
    virtual void sendOlderVersionDone(NetworkInterface *ie, const Ipv6Address& group);
    // Router side: a received MLDv1 Report/Done puts the group into
    // older-version-host-present mode (forward as EXCLUDE{}).
    virtual void processOlderVersionReport(NetworkInterface *ie, Packet *packet);
    virtual void processOlderVersionDone(NetworkInterface *ie, Packet *packet);
    virtual void enterRouterOlderVersionCompat(NetworkInterface *ie, RouterGroupData *groupData);
    virtual RouterGroupData *getRouterGroupData(NetworkInterface *ie, const Ipv6Address& group); // nullptr if absent

    virtual void multicastSourceListChanged(NetworkInterface *ie, const Ipv6Address& group, const Ipv6MulticastSourceList& sourceList);

  public:
    /**
     * MLDv2 Maximum Response Code / QQIC codec (RFC 3810 §5.1.3, 16-bit).
     * If code < 0x8000 the value is the literal code, otherwise the code is the
     * floating-point form '1 eee mmmm mmmm mmmm' and decodes to
     * (0x1000 | mant) << (exp + 3), where mant is the low 12 bits and exp is the
     * next 3 bits. Units are milliseconds for the Maximum Response Code.
     */
    static uint16_t decodeMaxRespCode(uint16_t code);
    static uint16_t codeMaxRespCode(uint16_t value);
    static uint8_t codeQqic(uint16_t value); // the 8-bit code of the Querier's Query Interval
};

inline std::ostream& operator<<(std::ostream& out, const Ipv6AddressVector addresses)
{
    out << "(";
    for (size_t i = 0; i < addresses.size(); i++)
        out << (i > 0 ? "," : "") << addresses[i];
    out << ")";
    return out;
}

} // namespace inet

#endif
