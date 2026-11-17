// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_DTLSPATHMTUSTATE_H
#define INET_DTLSPATHMTUSTATE_H

#include "inet/common/INETDefs.h"

namespace inet::dtls {

/** Finite size-search policy; the association owns scheduling and probe recovery. */
class INET_API DtlsPathMtuState
{
  public:
    enum class Phase { BASE = 1, SEARCHING = 2, SEARCH_COMPLETE = 3, ERROR = 4, UNSUPPORTED = 5 };

  protected:
    size_t base;
    size_t ceiling;
    size_t searchCeiling;
    size_t confirmed;
    Phase phase = Phase::BASE;

  public:
    DtlsPathMtuState(size_t base, size_t ceiling);
    [[nodiscard]] Phase getPhase() const { return phase; }
    // BASE and ERROR retain the provisional base; availability is separate.
    [[nodiscard]] size_t getUdpBudget() const { return confirmed; }
    [[nodiscard]] size_t getProbeSize() const;
    [[nodiscard]] bool isAvailable() const { return phase != Phase::ERROR; }
    void reset();
    void unsupported();
    void raise();
    void succeeded();
    // One failure means the shared probe exhausted its three actual attempts.
    void failed();
    bool packetTooBig(size_t udpBytes);
};

} // namespace inet::dtls
#endif
