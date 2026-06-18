//
// Copyright (C) 2012 - 2013 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//


/**
 * @file Igmpv3.h
 * @author Adam Malik(towdie13@gmail.com), Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @date 12.5.2013
 */

#ifndef __INET_IGMPV3_H
#define __INET_IGMPV3_H

#include "inet/common/SimpleModule.h"
#include <set>

#include "inet/common/ModuleRefByPar.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/stlutils.h"
#include "inet/networklayer/common/NetworkInterface.h"
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/networklayer/ipv4/IgmpMessage.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"

namespace inet {

class IInterfaceTable;
class IRoutingTable;

class INET_API Igmpv3 : public SimpleModule, protected cListener
{
  protected:
    enum RouterState {
        IGMPV3_RS_INITIAL,
        IGMPV3_RS_QUERIER,
        IGMPV3_RS_NON_QUERIER,
    };

    enum RouterGroupState {
        IGMPV3_RGS_NO_MEMBERS_PRESENT,
        IGMPV3_RGS_MEMBERS_PRESENT,
        IGMPV3_RGS_CHECKING_MEMBERSHIP,
    };

    enum HostGroupState {
        IGMPV3_HGS_NON_MEMBER,
        IGMPV3_HGS_DELAYING_MEMBER,
        IGMPV3_HGS_IDLE_MEMBER,
    };

    enum FilterMode {
        IGMPV3_FM_INCLUDE,
        IGMPV3_FM_EXCLUDE,
    };

    enum ReportType {
        IGMPV3_RT_IS_IN = 1,
        IGMPV3_RT_IS_EX = 2,
        IGMPV3_RT_TO_IN = 3,
        IGMPV3_RT_TO_EX = 4,
        IGMPV3_RT_ALLOW = 5,
        IGMPV3_RT_BLOCK = 6,
    };

    enum IgmpTimerKind {
        IGMPV3_R_GENERAL_QUERY_TIMER,
        IGMPV3_R_GROUP_TIMER,
        IGMPV3_R_SOURCE_TIMER,
        IGMPV3_R_REXMT_TIMER,
        IGMPV3_R_OLDER_VERSION_TIMER, // Older Version Host Present timer (RFC 3376 7.3.2), per RouterGroupData
        IGMPV3_H_GENERAL_QUERY_TIMER,
        IGMPV3_H_GROUP_TIMER,
        IGMPV3_H_STATE_CHANGE_TIMER,
        IGMPV3_H_OLDER_VERSION_TIMER, // Older Version Querier Present timer (RFC 3376 7.2.1), per HostInterfaceData
    };

    // Older-version compatibility level (RFC 3376 7.2/7.3). NONE means native v3.
    enum CompatVersion {
        IGMP_COMPAT_NONE = 0,
        IGMP_COMPAT_V1 = 1,
        IGMP_COMPAT_V2 = 2,
    };

    struct HostInterfaceData;

    struct HostGroupData {
        HostInterfaceData *parent;
        Ipv4Address groupAddr;
        FilterMode filter;
        Ipv4AddressVector sourceAddressList; // sorted
        HostGroupState state;
        cMessage *timer; // for scheduling responses to Group-Specific and Group-and-Source-Specific Queries
        Ipv4AddressVector queriedSources; // saved from last Group-Specific or Group-and-Source-Specific Query; sorted

        // State-Change Report retransmission (RFC 3376 6.1): the last State-Change
        // Report is (re)transmitted [Robustness Variable] times in total.
        std::vector<GroupRecord> pendingRecords; // records of the pending State-Change Report
        int retransmitCount = 0; // remaining retransmissions (0 = nothing pending)
        cMessage *retransmitTimer; // fires at uniform(0, unsolicitedReportInterval)

        HostGroupData(HostInterfaceData *parent, Ipv4Address group);
        virtual ~HostGroupData();
        std::string getStateInfo() const;
    };

    typedef std::map<Ipv4Address, HostGroupData *> GroupToHostDataMap;

    struct HostInterfaceData {
        Igmpv3 *owner;
        NetworkInterface *ie;
//        int multicastRouterVersion;
        GroupToHostDataMap groups;
        cMessage *generalQueryTimer; // for scheduling responses to General Queries

        // Older Version Querier Present (RFC 3376 7.2.1): while olderVersionTimer is
        // scheduled, an older-version querier is present on this interface and the host
        // emits older-version (v1/v2) Reports/Leaves instead of v3 reports.
        cMessage *olderVersionTimer; // fires at otherQuerierPresentInterval
        CompatVersion compatVersion = IGMP_COMPAT_NONE;

        HostInterfaceData(Igmpv3 *owner, NetworkInterface *ie);
        virtual ~HostInterfaceData();
        HostGroupData *getOrCreateGroupData(Ipv4Address group);
        void deleteGroupData(Ipv4Address group);
        friend inline std::ostream& operator<<(std::ostream& out, const Igmpv3::HostInterfaceData& entry)
        {
            for (auto& g : entry.groups) {
                out << "(groupAddress: " << g.second->groupAddr << " ";
                out << "hostGroupState: " << Igmpv3::getHostGroupStateString(g.second->state) << " ";
                out << "groupTimer: " << g.second->timer->getArrivalTime() << " ";
                out << "queriedSources: ";
                for (auto& entry : g.second->queriedSources)
                    out << entry << ", ";
                out << "sourceAddressList: ";
                for (auto& entry : g.second->sourceAddressList)
                    out << entry << ", ";
                out << "filter: " << Igmpv3::getFilterModeString(g.second->filter) << ") ";
            }

            return out;
        }
    };

    struct RouterInterfaceData;
    struct RouterGroupData;

    struct SourceRecord {
        RouterGroupData *parent;
        Ipv4Address sourceAddr;
        cMessage *sourceTimer;

        SourceRecord(RouterGroupData *parent, Ipv4Address source);
        virtual ~SourceRecord();
    };

    typedef std::map<Ipv4Address, SourceRecord *> SourceToSourceRecordMap;

    struct RouterGroupData {
        RouterInterfaceData *parent;
        Ipv4Address groupAddr;
        FilterMode filter;
        RouterGroupState state;
        cMessage *timer;
        SourceToSourceRecordMap sources; // TODO should map source addresses to source timers
                                         // i.e. map<Ipv4Address,cMessage*>

        // Last-Member/Group-Specific Query retransmission (RFC 3376 6.4.2): a
        // Group-Specific or Group-and-Source-Specific Query is sent [Last Member
        // Query Count] times in total, lastMemberQueryInterval apart.
        cMessage *rexmtTimer; // fires at lastMemberQueryInterval
        int rexmtCount = 0; // remaining retransmissions (0 = nothing pending)
        bool rexmtGroupAndSource = false; // false=Group-Specific, true=Group-and-Source-Specific
        Ipv4AddressVector rexmtSources; // for the group-and-source case: sources to resend; sorted

        // Older Version Host Present (RFC 3376 7.3.2): while olderVersionTimer is
        // scheduled, an older-version (v1/v2) host is present for this group. The group
        // is forwarded as EXCLUDE{} (any-source) and v3 per-source processing is bypassed.
        cMessage *olderVersionTimer; // fires at groupMembershipInterval
        CompatVersion olderVersionCompat = IGMP_COMPAT_NONE;

        RouterGroupData(RouterInterfaceData *parent, Ipv4Address group);
        virtual ~RouterGroupData();
        bool hasSourceRecord(Ipv4Address source) { return containsKey(sources, source); }
        SourceRecord *createSourceRecord(Ipv4Address source);
        SourceRecord *getOrCreateSourceRecord(Ipv4Address source);
        void deleteSourceRecord(Ipv4Address source);
        std::string getStateInfo() const;
        void collectForwardedSources(Ipv4MulticastSourceList& result) const;

      private:
        void printSourceList(std::ostream& out, bool withRunningTimer) const;
    };

    typedef std::map<Ipv4Address, RouterGroupData *> GroupToRouterDataMap;

    struct RouterInterfaceData {
        Igmpv3 *owner;
        NetworkInterface *ie;
        GroupToRouterDataMap groups;
        RouterState state;
        cMessage *generalQueryTimer;

        RouterInterfaceData(Igmpv3 *owner, NetworkInterface *ie);
        virtual ~RouterInterfaceData();
        RouterGroupData *getOrCreateGroupData(Ipv4Address group);
        void deleteGroupData(Ipv4Address group);
        friend inline std::ostream& operator<<(std::ostream& out, const Igmpv3::RouterInterfaceData& entry)
        {
            out << "routerState: " << Igmpv3::getRouterStateString(entry.state) << " ";
            out << "queryTimer: " << entry.generalQueryTimer->getArrivalTime() << " ";
            if (entry.groups.empty())
                out << "(empty)";
            else {
                for (auto& g : entry.groups) {
                    out << "(groupAddress: " << g.second->groupAddr << " ";
                    out << "routerGroupState: " << Igmpv3::getRouterGroupStateString(g.second->state) << " ";
                    out << "timer: " << g.second->timer->getArrivalTime() << " ";
                    out << "filter: " << Igmpv3::getFilterModeString(g.second->filter) << ") ";
                }
            }

            return out;
        }
    };

  protected:
    ModuleRefByPar<IRoutingTable> rt;
    ModuleRefByPar<IInterfaceTable> ift;

    bool enabled;
    int robustnessVariable; // RFC 3376: a State-Change Report is (re)transmitted this many times
    double queryInterval; // TODO these should probably be simtime_t
    double queryResponseInterval;
    double groupMembershipInterval;
    double otherQuerierPresentInterval;
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
    static Ipv4AddressVector set_complement(const Ipv4AddressVector& first, const Ipv4AddressVector& second);
    static Ipv4AddressVector set_intersection(const Ipv4AddressVector& first, const Ipv4AddressVector& second);
    static Ipv4AddressVector set_union(const Ipv4AddressVector& first, const Ipv4AddressVector& second);

    static const std::string getRouterStateString(RouterState rs);
    static const std::string getRouterGroupStateString(RouterGroupState rgs);
    static const std::string getHostGroupStateString(HostGroupState hgs);
    static const std::string getFilterModeString(FilterMode fm);

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual ~Igmpv3();

  protected:
    void addWatches();

    virtual HostInterfaceData *createHostInterfaceData(NetworkInterface *ie);
    virtual RouterInterfaceData *createRouterInterfaceData(NetworkInterface *ie);
    virtual HostInterfaceData *getHostInterfaceData(NetworkInterface *ie);
    virtual RouterInterfaceData *getRouterInterfaceData(NetworkInterface *ie);
    virtual RouterGroupData *getRouterGroupData(NetworkInterface *ie, Ipv4Address group); // nullptr if absent
    virtual void deleteHostInterfaceData(int interfaceId);
    virtual void deleteRouterInterfaceData(int interfaceId);

    virtual void configureInterface(NetworkInterface *ie);

    virtual void startTimer(cMessage *timer, double interval);

    virtual void sendGeneralQuery(RouterInterfaceData *interface, double maxRespTime);
    virtual void sendGroupSpecificQuery(RouterGroupData *group);
    virtual void sendGroupAndSourceSpecificQuery(RouterGroupData *group, const Ipv4AddressVector& sources);
    virtual void sendGroupReport(NetworkInterface *ie, const std::vector<GroupRecord>& records);
    virtual void sendQueryToIP(Packet *msg, NetworkInterface *ie, Ipv4Address dest);
    virtual void sendReportToIP(Packet *msg, NetworkInterface *ie, Ipv4Address dest);

    virtual void processHostGeneralQueryTimer(cMessage *msg);
    virtual void processHostGroupQueryTimer(cMessage *msg);
    virtual void processHostStateChangeTimer(cMessage *msg);
    virtual void processHostOlderVersionTimer(cMessage *msg);
    virtual void processRouterGeneralQueryTimer(cMessage *msg);
    virtual void processRouterGroupTimer(cMessage *msg);
    virtual void processRouterSourceTimer(cMessage *msg);
    virtual void processRexmtTimer(cMessage *msg);
    virtual void processRouterOlderVersionTimer(cMessage *msg);

    virtual void processIgmpMessage(Packet *msg);
    virtual void processQuery(Packet *msg);
    virtual void processReport(Packet *msg);

    // --- Older-version interop (RFC 3376 7.2/7.3) ---
    // Host side: enter older-version compatibility on an older-version General Query, and
    // emit older-version messages while it is active.
    virtual void processOlderVersionQuery(NetworkInterface *ie, Packet *packet, CompatVersion version);
    virtual void sendOlderVersionReport(NetworkInterface *ie, Ipv4Address group, CompatVersion version);
    virtual void sendOlderVersionLeave(NetworkInterface *ie, Ipv4Address group);
    // Router side: a received older-version Report/Leave puts the group into
    // older-version-host-present mode (forward as EXCLUDE{}).
    virtual void processOlderVersionReport(NetworkInterface *ie, Packet *packet, CompatVersion version);
    virtual void processOlderVersionLeave(NetworkInterface *ie, Packet *packet);
    virtual void enterRouterOlderVersionCompat(NetworkInterface *ie, RouterGroupData *groupData, CompatVersion version);

    virtual void multicastSourceListChanged(NetworkInterface *ie, Ipv4Address group, const Ipv4MulticastSourceList& sourceList);

  public:
    static void insertChecksum(ChecksumMode checksumMode, const Ptr<IgmpMessage>& igmpMsg, Packet *payload);
    void insertChecksum(const Ptr<IgmpMessage>& igmpMsg, Packet *payload) { insertChecksum(checksumMode, igmpMsg, payload); }
    bool verifyChecksum(const Packet *packet);

  public:
    /**
     * Function for computing the time value in seconds from an encoded value.
     * Codes in the [1,127] interval are the number of 1/10 seconds,
     * codes above 127 are contain a 3-bit exponent and a four bit mantissa
     * and represents the (mantissa + 16) * 2^(3+exp) number of 1/10 seconds.
     */
    static uint16_t decodeTime(uint8_t code);
    static uint8_t codeTime(uint16_t timevalue);
};

inline std::ostream& operator<<(std::ostream& out, const Ipv4AddressVector addresses)
{
    out << "(";
    for (size_t i = 0; i < addresses.size(); i++)
        out << (i > 0 ? "," : "") << addresses[i];
    out << ")";
    return out;
}

} // namespace inet

#endif

