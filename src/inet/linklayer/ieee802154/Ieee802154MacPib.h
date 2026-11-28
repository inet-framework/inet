// SPDX-License-Identifier: LGPL-3.0-or-later
//

#ifndef __INET_IEEE802154MACPIB_H
#define __INET_IEEE802154MACPIB_H

#include <cstdint>
#include <string>
#include <vector>

#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee802154/Ieee802154MacGetRequest.h"

namespace inet {

/**
 * Synchronous storage and validation for the selected IEEE 802.15.4 MAC PIB.
 * The future service provider owns admission, request IDs, notifications, and
 * applying accepted values to radio or access procedures; this class has no
 * module, PHY, RNG, callback, or operational state.
 */
class INET_API Ieee802154MacPib
{
  private:
    struct State {
        Ieee802154Address macExtendedAddress;
        Ieee802154Address macCoordExtendedAddress; // NONE is the local unspecified-context representation.
        uint16_t macShortAddress = 0xffff;
        uint16_t macCoordShortAddress = 0xffff;
        uint16_t macPanId = 0xffff;
        uint8_t macDsn = 0;
        uint8_t macMinBe = 3;
        uint8_t macMaxBe = 5;
        uint8_t macMaxCsmaBackoffs = 4;
        uint8_t macMaxFrameRetries = 3;
        uint32_t macSifsPeriod = 12;
        uint32_t macLifsPeriod = 40;
        uint32_t macUnitBackoffPeriod = 0;
        bool macRxOnWhenIdle = false;
        bool macImplicitBroadcast = false;
        bool macGroupRxMode = false;
        bool macSecurityEnabled = false;
        uint8_t macBeaconOrder = 15;
        bool macTimestampSupported = false;
        uint16_t macSyncSymbolOffset = 0;
        bool macPromiscuousMode = false;
        bool macDsmeCapable = false;
        bool macDsmeEnabled = false;
        bool macDaCapable = false;
        bool macDaEnabled = false;
        bool macExtendedDsmeCapable = false;
        bool macExtendedDsmeEnabled = false;
        bool macHoppingCapable = false;
        bool macHoppingEnabled = false;
        bool macLeCapable = false;
        bool macLeEnabled = false;
        bool macLeHsEnabled = false;
        bool macMetricsCapable = false;
        bool macMetricsEnabled = false;
        bool macRccnCapable = false;
        bool macRccnEnabled = false;
        bool macSrmCapable = false;
        bool macSrmEnabled = false;
        bool macTrleCapable = false;
        bool macTrleEnabled = false;
        bool macTrleRelayingMode = false;
        bool macTschCapable = false;
        bool macTschEnabled = false;
    };

    State state;
    const uint32_t ccaDurationUs;

    static bool isIndividualExtendedAddress(const Ieee802154Address& address);
    static uint32_t calculateUnitBackoffPeriod(uint32_t ccaDurationUs);
    static Ieee802154MacStatus getValue(const State& state, const std::string& attribute, Ieee802154MacPibValue& value);
    static Ieee802154MacStatus setValue(State& state, const std::string& attribute, const Ieee802154MacPibValue& value, bool validateCrossAttributes);
    static State makeDefaultState(const Ieee802154Address& deviceExtendedAddress, uint8_t dsn, uint32_t ccaDurationUs);

  public:
    /**
     * Creates the selected M1 MAC PIB. The device identity must be an
     * individual extended address. DSN is supplied by the owner; this class
     * never draws randomness. The selected O-QPSK profile uses 16 us per
     * symbol, 12 turnaround symbols, and a default CCA duration of 128 us.
     * Startup overrides use exact Table 8-36/8-37 names and the same value
     * rules as SET; the final BE pair is checked after all overrides so their
     * order is immaterial. Duplicate or invalid overrides throw before the
     * object is constructed.
     */
    Ieee802154MacPib(Ieee802154Address deviceExtendedAddress, uint8_t injectedDsn,
            uint32_t ccaDurationUs = 128, const std::vector<Ieee802154MacSetRequest>& startupOverrides = {});

    /** GET echoes the exact name; an unsupported name returns UNSUPPORTED_ATTRIBUTE with no value. */
    Ieee802154MacGetConfirm get(const std::string& attribute) const;

    /**
     * Sets one selected attribute atomically. Unknown names return
     * UNSUPPORTED_ATTRIBUTE; read-only names return READ_ONLY before value
     * validation; invalid type, domain, profile, or BE invariant returns
     * INVALID_PARAMETER without changing any stored value.
     */
    Ieee802154MacStatus set(const std::string& attribute, const Ieee802154MacPibValue& value);

    /**
     * Applies only the PIB portion of MLME-RESET (8.2.6.2): false retains all
     * values; true restores selected defaults and uses the caller-supplied
     * DSN. PHY reset and operation abortion belong to the service provider.
     */
    void reset(bool setDefaultPib, uint8_t injectedDsn);
};

} // namespace inet

#endif
