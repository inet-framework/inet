// SPDX-License-Identifier: LGPL-3.0-or-later
#include "inet/transportlayer/dtls/DtlsSessionStore.h"

#include <algorithm>

namespace inet::dtls {

void DtlsSessionStore::configure(size_t limit)
{
    clear();
    capacity = limit;
}

bool DtlsSessionStore::consumeEarly(const std::vector<uint8_t>& ticket, const std::vector<uint8_t>& clientRandom, uint64_t associationId, simtime_t now)
{
    for (auto& entry : entries)
        if (entry.version == 0xfefc && entry.serverSide && entry.ticket == ticket && entry.expiresAt > now && entry.earlyDataBytes != 0 && associationId != 0) {
            if (entry.earlyConsumed)
                return entry.earlyAssociationId == associationId && entry.earlyClientRandom == clientRandom;
            entry.earlyConsumed = true;
            entry.earlyAssociationId = associationId;
            entry.earlyClientRandom = clientRandom;
            return true;
        }
    return false;
}

bool DtlsSessionStore::insert(const Entry& entry, simtime_t now)
{
    entries.erase(std::remove_if(entries.begin(), entries.end(), [now](const auto& value) { return value.expiresAt <= now; }), entries.end());
    if (capacity == 0 || (entry.ticket.empty() && entry.sessionId.empty()) || entry.ticket.size() > 65535 ||
        entry.sessionId.size() > 32 || (!entry.sessionId.empty() && entry.version != 0xfefd) || entry.expiresAt <= now)
        return false;
    // A retransmitted ticket must not refresh its original expiry or admission state.
    if (find(entry.ticket, entry.serverSide, now, entry.version) || findSessionId(entry.sessionId, entry.serverSide, now))
        return false;
    if (entries.size() == capacity)
        entries.erase(entries.begin());
    entries.push_back(entry);
    return true;
}

std::optional<DtlsSessionStore::Entry> DtlsSessionStore::find(const std::vector<uint8_t>& ticket, bool serverSide, simtime_t now, uint16_t version) const
{
    for (const auto& entry : entries)
        if (!ticket.empty() && entry.version == version && entry.ticket == ticket && entry.serverSide == serverSide && entry.expiresAt > now)
            return entry;
    return {};
}

std::optional<DtlsSessionStore::Entry> DtlsSessionStore::findSessionId(const std::vector<uint8_t>& sessionId, bool serverSide, simtime_t now) const
{
    for (const auto& entry : entries)
        if (!sessionId.empty() && entry.version == 0xfefd && entry.sessionId == sessionId && entry.serverSide == serverSide && entry.expiresAt > now)
            return entry;
    return {};
}

void DtlsSessionStore::remove(const Entry& entry)
{
    entries.erase(std::remove_if(entries.begin(), entries.end(), [&](const auto& value) {
        return value.version == entry.version && value.serverSide == entry.serverSide &&
               ((!entry.ticket.empty() && value.ticket == entry.ticket) || (!entry.sessionId.empty() && value.sessionId == entry.sessionId));
    }), entries.end());
}

std::optional<DtlsSessionStore::Entry> DtlsSessionStore::select(const L3Address& peer, int port, const std::string& identity, uint16_t version, simtime_t now,
                                                            const std::vector<uint8_t>& allowedHashes) const
{
    for (auto it = entries.rbegin(); it != entries.rend(); ++it)
        if (!it->serverSide && it->peerAddress == peer && it->peerPort == port && it->peerIdentity == identity &&
            it->version == version && it->expiresAt > now && (allowedHashes.empty() || std::find(allowedHashes.begin(), allowedHashes.end(), it->hashBytes) != allowedHashes.end()))
            return *it;
    return {};
}

} // namespace inet::dtls
