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

class INET_API Igmpv3 : public cSimpleModule, protected cListener
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
        IGMPV3_H_GENERAL_QUERY_TIMER,
        IGMPV3_H_GROUP_TIMER,
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
    int robustness;
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

    // crcMode
    CrcMode crcMode = CRC_MODE_UNDEFINED;

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
    virtual void processRouterGeneralQueryTimer(cMessage *msg);
    virtual void processRouterGroupTimer(cMessage *msg);
    virtual void processRouterSourceTimer(cMessage *msg);

    virtual void processIgmpMessage(Packet *msg);
    virtual void processQuery(Packet *msg);
    virtual void processReport(Packet *msg);

    virtual void multicastSourceListChanged(NetworkInterface *ie, Ipv4Address group, const Ipv4MulticastSourceList& sourceList);

  public:
    static void insertCrc(CrcMode crcMode, const Ptr<IgmpMessage>& igmpMsg, Packet *payload);
    void insertCrc(const Ptr<IgmpMessage>& igmpMsg, Packet *payload) { insertCrc(crcMode, igmpMsg, payload); }
    bool verifyCrc(const Packet *packet);

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

