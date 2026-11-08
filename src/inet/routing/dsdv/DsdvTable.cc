// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/routing/dsdv/DsdvTable.h"

#include <algorithm>
#include <limits>

namespace inet {
namespace dsdv {

std::ostream& operator<<(std::ostream& stream, const DsdvTable::RouteEntry& record)
{
    stream << "destination=" << record.current.destination << " successor=" << record.successor
           << " sequence=" << record.current.sequence << " metric=" << record.current.metric
           << " refreshed=" << record.refreshed << " changed=" << record.changed
           << " firstAcceptance=" << record.firstAcceptance << " lastImprovement=" << record.lastImprovement
           << " estimate=" << record.estimate << " deadline=" << record.deadline
           << " hasSample=" << record.hasSample << " hasAdvertised=" << record.hasAdvertised;
    if (record.hasAdvertised)
        stream << " advertisedSequence=" << record.advertised.sequence << " advertisedMetric=" << record.advertised.metric;
    return stream;
}

bool DsdvTable::isHost(Ipv4Address address)
{
    uint32_t value = address.getInt();
    // Profile: ordinary IPv4 unicast, excluding 0/8, loopback and class D/E.
    return (value >> 24) != 0 && (value >> 24) != 127 && value < 0xe0000000U;
}

bool DsdvTable::isValidEntry(const DsdvEntry& entry)
{
    return isHost(entry.destination) && (isFinite(entry) ?
            entry.sequence % 2 == 0 && entry.metric <= INT_MAX : entry.sequence % 2 == 1);
}

bool DsdvTable::isValidHeader(const DsdvHeader& header)
{
    if (!header.isCorrect() || header.getVersion() != 1 || (header.getKind() != 1 && header.getKind() != 2) ||
            !isHost(header.getSender()) || header.getSelfSequence() % 2 || header.getEntriesArraySize() > UINT16_MAX ||
            header.getChunkLength() != B(HEADER_BYTES + ENTRY_BYTES * header.getEntriesArraySize()))
        return false;
    std::set<Ipv4Address> seen;
    for (size_t i = 0; i < header.getEntriesArraySize(); i++) {
        const auto& entry = header.getEntries(i);
        if (!isValidEntry(entry) || !seen.insert(entry.destination).second ||
                (isFinite(entry) && entry.metric == INT_MAX))
            return false;
        if (entry.destination == header.getSender() &&
                (entry.metric != 0 || entry.sequence != header.getSelfSequence()))
            return false;
        if (entry.metric == 0 && entry.destination != header.getSender())
            return false;
    }
    return true;
}

uint64_t DsdvTable::advance(uint64_t sequence, uint64_t increment)
{
    if (sequence > std::numeric_limits<uint64_t>::max() - increment)
        throw cRuntimeError("DSDV sequence exhausted; wraparound is outside this finite-run profile");
    return sequence + increment;
}

bool DsdvTable::equal(const DsdvEntry& a, const DsdvEntry& b)
{
    return a.destination == b.destination && a.sequence == b.sequence && a.metric == b.metric;
}

void DsdvTable::configure(Ipv4Address address, simtime_t settlingCap, simtime_t neighborAge, simtime_t routeAge)
{
    if (!isHost(address) || settlingCap <= SIMTIME_ZERO || neighborAge <= SIMTIME_ZERO || routeAge <= SIMTIME_ZERO)
        throw cRuntimeError("DSDV requires a unicast IPv4 address and positive settling/expiry intervals");
    if (!self.isUnspecified() && self != address)
        throw cRuntimeError("DSDV address changes require persistent state migration and are unsupported");
    self = address;
    cap = settlingCap;
    neighborTimeout = neighborAge;
    routeTimeout = routeAge;
}

void DsdvTable::makeUrgent(RouteEntry& record)
{
    record.deadline = -1;
    record.advertised = record.current;
    record.hasAdvertised = true;
    mustSend.insert(record.current.destination);
}

void DsdvTable::start(simtime_t now)
{
    bool restarting = routeEntries.count(self);
    suspend(now);
    if (!restarting) {
        auto& record = routeEntries[self];
        record.current.destination = self;
        record.current.metric = 0;
        record.current.sequence = 0;
    }
    else
        advanceSelf();
    for (auto& item : routeEntries) {
        auto& record = item.second;
        record.hasSample = false;
        record.estimate = cap / 2;
        record.changed = now;
        record.refreshed = now;
        record.deadline = -1;
        record.hasAdvertised = true;
        record.advertised = record.current;
    }
}

void DsdvTable::suspend(simtime_t now)
{
    for (auto& item : routeEntries) {
        if (item.first != self)
            invalidate(item.first, now);
        item.second.deadline = -1;
        item.second.hasAdvertised = false;
        item.second.hasSample = false;
    }
    mustSend.clear();
    baseline.clear();
    neighbors.clear();
}

void DsdvTable::advanceSelf()
{
    auto& entry = routeEntries.at(self).current;
    entry.sequence = advance(entry.sequence, 2);
}

void DsdvTable::install(const DsdvEntry& entry, Ipv4Address sender, simtime_t now)
{
    auto it = routeEntries.find(entry.destination);
    bool known = it != routeEntries.end();
    auto& record = routeEntries[entry.destination];
    bool wasFinite = known && isFinite(record.current);
    bool finite = isFinite(entry);
    bool newer = !known || entry.sequence > record.current.sequence;
    bool pathChanged = !known || record.current.metric != entry.metric || record.successor != sender;
    bool urgent = mustSend.count(entry.destination) || !finite || !wasFinite ||
            (pathChanged && now - record.changed >= cap);
    if (!known)
        record.estimate = cap / 2;
    // Model smoothing law; PB94 §4 specifies weighted history but no coefficients.
    if (finite && newer) {
        if (record.hasSample) {
            simtime_t sample = std::min(cap, record.lastImprovement - record.firstAcceptance);
            record.estimate = sample * 0.75 + record.estimate * 0.25;
        }
        record.firstAcceptance = record.lastImprovement = now;
        record.hasSample = true;
    }
    else if (finite && entry.metric < record.current.metric)
        record.lastImprovement = now;
    if (!finite)
        record.hasSample = false;
    record.current = entry;
    record.successor = finite ? sender : Ipv4Address();
    record.refreshed = now;
    if (pathChanged)
        record.changed = now;
    if (urgent)
        makeUrgent(record);
    else if (pathChanged && record.deadline < SIMTIME_ZERO)
        record.deadline = now + std::min(cap, record.estimate * 2);
}

void DsdvTable::receive(const DsdvHeader& header, simtime_t now)
{
    if (!isValidHeader(header))
        throw cRuntimeError("DSDV table received an unvalidated datagram");
    Ipv4Address sender = header.getSender();
    if (sender == self)
        return;
    neighbors[sender] = now;
    // Sender identity is liveness, not an implicit route refresh. Only entries
    // below refresh destination clocks; omission never means withdrawal.
    for (size_t i = 0; i < header.getEntriesArraySize(); i++) {
        DsdvEntry entry = header.getEntries(i);
        if (entry.destination == self) {
            auto& record = routeEntries.at(self);
            if (!isFinite(entry) && entry.sequence > record.current.sequence) {
                record.current.sequence = advance(entry.sequence, 1);
                makeUrgent(record);
            }
            continue;
        }
        if (isFinite(entry))
            entry.metric++;
        auto it = routeEntries.find(entry.destination);
        if (it == routeEntries.end()) {
            install(entry, sender, now);
            continue;
        }
        auto& record = it->second;
        // Perkins/Bhagwat 1994 §3, §4: sequence freshness, then lower metric.
        if (entry.sequence > record.current.sequence ||
                (entry.sequence == record.current.sequence && entry.metric < record.current.metric))
            install(entry, sender, now);
        else if (entry.sequence < record.current.sequence) {
            if (!isFinite(entry) && isFinite(record.current))
                makeUrgent(record);
        }
        else if (isFinite(entry) && entry.metric == record.current.metric && sender == record.successor)
            record.refreshed = now;
    }
}

bool DsdvTable::invalidate(Ipv4Address destination, simtime_t now)
{
    auto& record = routeEntries.at(destination);
    if (destination == self || !isFinite(record.current))
        return false;
    // Perkins/Bhagwat 1994 §3: failed-successor withdrawals have odd sequences.
    record.current.sequence = advance(record.current.sequence, 1);
    record.current.metric = INFINITY_METRIC;
    record.successor = Ipv4Address();
    record.changed = now;
    record.hasSample = false;
    makeUrgent(record);
    return true;
}

std::vector<Ipv4Address> DsdvTable::expire(simtime_t now)
{
    std::set<Ipv4Address> lost;
    for (auto it = neighbors.begin(); it != neighbors.end(); ) {
        if (it->second + neighborTimeout <= now) {
            lost.insert(it->first);
            it = neighbors.erase(it);
        }
        else
            ++it;
    }
    std::vector<Ipv4Address> invalidated;
    for (auto& item : routeEntries) {
        const auto& record = item.second;
        if (item.first != self && isFinite(record.current) &&
                (lost.count(record.successor) || record.refreshed + routeTimeout <= now)) {
            invalidate(item.first, now);
            invalidated.push_back(item.first);
        }
    }
    return invalidated;
}

simtime_t DsdvTable::nextExpiry() const
{
    simtime_t result = SimTime::getMaxTime();
    for (const auto& item : neighbors)
        result = std::min(result, item.second + neighborTimeout);
    for (const auto& item : routeEntries)
        if (item.first != self && isFinite(item.second.current))
            result = std::min(result, item.second.refreshed + routeTimeout);
    return result;
}

simtime_t DsdvTable::nextAdvertisement(simtime_t now) const
{
    if (!mustSend.empty())
        return now;
    simtime_t result = SimTime::getMaxTime();
    for (const auto& item : routeEntries)
        if (item.second.deadline >= SIMTIME_ZERO)
            result = std::min(result, std::max(now, item.second.deadline));
    return result;
}

DsdvTable::Advertisement DsdvTable::prepare(bool full, size_t capacity, simtime_t now)
{
    if (capacity == 0)
        throw cRuntimeError("DSDV payload budget cannot hold one entry");
    Advertisement result;
    std::map<Ipv4Address, DsdvEntry> view;
    for (auto& item : routeEntries) {
        auto& record = item.second;
        if (record.deadline >= SIMTIME_ZERO && record.deadline <= now) {
            result.settlingDelays.push_back(now - record.changed);
            makeUrgent(record);
        }
        if (record.deadline < SIMTIME_ZERO) {
            record.advertised = record.current;
            record.hasAdvertised = true;
        }
        if (record.hasAdvertised)
            view[item.first] = record.advertised;
    }
    std::vector<DsdvEntry> required;
    std::vector<DsdvEntry> optional;
    for (const auto& item : view) {
        auto old = baseline.find(item.first);
        if (mustSend.count(item.first) || old == baseline.end() || old->second.metric != item.second.metric)
            required.push_back(item.second);
        else if (!equal(old->second, item.second))
            optional.push_back(item.second);
    }
    result.full = full || required.size() > capacity;
    std::vector<DsdvEntry> entries;
    if (result.full) {
        for (const auto& item : view)
            entries.push_back(item.second);
    }
    else {
        entries = required;
        for (const auto& entry : optional) {
            if (entries.size() == capacity)
                break;
            entries.push_back(entry);
        }
    }
    for (size_t offset = 0; offset < entries.size(); offset += capacity) {
        size_t end = std::min(entries.size(), offset + capacity);
        result.datagrams.emplace_back(entries.begin() + offset, entries.begin() + end);
    }
    // Submission is synchronous in the adapter. The adapter commits this result
    // only by submitting every returned datagram in this same callback.
    for (const auto& entry : entries)
        mustSend.erase(entry.destination);
    if (result.full)
        baseline = view;
    return result;
}

} // namespace dsdv
} // namespace inet
