// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef __INET_DSDVTABLE_H
#define __INET_DSDVTABLE_H

#include <climits>
#include <map>
#include <ostream>
#include <set>
#include <vector>

#include "inet/routing/dsdv/DsdvHeader_m.h"

namespace inet {
namespace dsdv {

// The sole owner of DSDV freshness and dissemination state. The routing table
// is a projection; it never supplies sequence numbers or cached alternatives.
class INET_API DsdvTable
{
  public:
    static constexpr uint32_t INFINITY_METRIC = UINT32_MAX;
    static constexpr int HEADER_BYTES = 16;
    static constexpr int ENTRY_BYTES = 16;

    struct RouteEntry {
        DsdvEntry current;
        Ipv4Address successor;
        simtime_t refreshed;
        simtime_t changed;
        simtime_t firstAcceptance;
        simtime_t lastImprovement;
        simtime_t estimate;
        simtime_t deadline = -1;
        bool hasSample = false;
        bool hasAdvertised = false;
        DsdvEntry advertised;
    };

    struct Advertisement {
        bool full = false;
        std::vector<std::vector<DsdvEntry>> datagrams;
        std::vector<simtime_t> settlingDelays;
    };

  protected:
    Ipv4Address self;
    simtime_t cap;
    simtime_t minSettlingDelay;
    simtime_t neighborTimeout;
    simtime_t routeTimeout;
    std::map<Ipv4Address, RouteEntry> routeEntries;
    std::map<Ipv4Address, simtime_t> neighbors;
    std::map<Ipv4Address, DsdvEntry> baseline;
    std::set<Ipv4Address> mustSend;
    Ipv4Address optionalCursor;
    size_t significantChanges = 0;
    size_t changeLimit = 0;

    void makeUrgent(RouteEntry& record);
    void install(const DsdvEntry& entry, Ipv4Address sender, simtime_t now);
    void recordChange() { if (significantChanges < changeLimit) significantChanges++; }

  public:
    static bool isHost(Ipv4Address address);
    static bool isValidEntry(const DsdvEntry& entry);
    static bool isValidHeader(const DsdvHeader& header);
    static uint64_t advance(uint64_t sequence, uint64_t increment);
    static bool isFinite(const DsdvEntry& entry) { return entry.metric != INFINITY_METRIC; }
    static bool equal(const DsdvEntry& a, const DsdvEntry& b);

    void configure(Ipv4Address address, simtime_t settlingCap, simtime_t neighborAge, simtime_t routeAge,
            simtime_t minSettlingDelay = SimTime(10, SIMTIME_MS));
    void start(simtime_t now);
    void suspend(simtime_t now);
    void advanceSelf();
    void configureChangeHistory(size_t limit) { changeLimit = limit; significantChanges = 0; }
    size_t consumeSignificantChanges() { size_t result = significantChanges; significantChanges = 0; return result; }
    void receive(const DsdvHeader& header, simtime_t now);
    bool invalidate(Ipv4Address destination, simtime_t now);
    std::vector<Ipv4Address> invalidateNeighbor(Ipv4Address neighbor, simtime_t now);
    std::vector<Ipv4Address> expire(simtime_t now);
    simtime_t nextExpiry() const;
    simtime_t nextAdvertisement(simtime_t now) const;
    Advertisement prepare(bool full, size_t capacity, simtime_t now);
    const std::map<Ipv4Address, RouteEntry>& getRouteEntries() const { return routeEntries; }
    const std::map<Ipv4Address, simtime_t>& getNeighbors() const { return neighbors; }
    const std::map<Ipv4Address, DsdvEntry>& getBaseline() const { return baseline; }
    const std::set<Ipv4Address>& getMustSend() const { return mustSend; }
    const RouteEntry& getSelf() const { return routeEntries.at(self); }
};

INET_API std::ostream& operator<<(std::ostream& stream, const DsdvTable::RouteEntry& record);

} // namespace dsdv
} // namespace inet
#endif
