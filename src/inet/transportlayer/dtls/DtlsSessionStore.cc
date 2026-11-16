// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/DtlsSessionStore.h"

#include <algorithm>

namespace inet::dtls {

void DtlsSessionStore::configure(size_t limit)
{
    clear();
    capacity = limit;
}

bool DtlsSessionStore::insert(const Entry& entry, simtime_t now)
{
    entries.erase(std::remove_if(entries.begin(), entries.end(), [now](const auto& value) { return value.expiresAt <= now; }), entries.end());
    if (capacity == 0 || entry.ticket.empty() || entry.ticket.size() > 65535 || entry.expiresAt <= now)
        return false;
    // A retransmitted ticket must not refresh its original expiry or admission state.
    if (find(entry.ticket, entry.serverSide, now))
        return false;
    if (entries.size() == capacity)
        entries.erase(entries.begin());
    entries.push_back(entry);
    return true;
}

std::optional<DtlsSessionStore::Entry> DtlsSessionStore::find(const std::vector<uint8_t>& ticket, bool serverSide, simtime_t now) const
{
    for (const auto& entry : entries)
        if (entry.ticket == ticket && entry.serverSide == serverSide && entry.expiresAt > now)
            return entry;
    return {};
}

std::optional<DtlsSessionStore::Entry> DtlsSessionStore::select(const L3Address& peer, int port, const std::string& identity, uint16_t version, simtime_t now) const
{
    for (auto it = entries.rbegin(); it != entries.rend(); ++it)
        if (!it->serverSide && it->peerAddress == peer && it->peerPort == port && it->peerIdentity == identity &&
            it->version == version && it->expiresAt > now)
            return *it;
    return {};
}

} // namespace inet::dtls
