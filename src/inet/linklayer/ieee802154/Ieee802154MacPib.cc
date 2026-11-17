// SPDX-License-Identifier: LGPL-3.0-or-later
//

#include "inet/linklayer/ieee802154/Ieee802154MacPib.h"

#include <limits>
#include <set>

namespace inet {

bool Ieee802154MacPib::isIndividualExtendedAddress(const Ieee802154Address& address)
{
    return address.isExtended() && !address.isExtendedGroup();
}

uint32_t Ieee802154MacPib::calculateUnitBackoffPeriod(uint32_t ccaDurationUs)
{
    if (ccaDurationUs < 1 || ccaDurationUs > 1000000)
        throw cRuntimeError("phyCcaDuration must be in the range 1..1000000 us");
    // 13.3.3 gives 62.5 ksymbol/s for 2450 MHz O-QPSK, i.e. 16 us/symbol.
    // Table 8-36 rounds CCA up to whole symbols before adding turnaround.
    return 12 + (ccaDurationUs + 15) / 16;
}

Ieee802154MacPib::State Ieee802154MacPib::makeDefaultState(const Ieee802154Address& deviceExtendedAddress, uint8_t dsn, uint32_t ccaDurationUs)
{
    State result;
    result.macExtendedAddress = deviceExtendedAddress;
    result.macCoordExtendedAddress = Ieee802154Address::NONE_ADDRESS;
    result.macDsn = dsn;
    result.macUnitBackoffPeriod = calculateUnitBackoffPeriod(ccaDurationUs);
    return result;
}

Ieee802154MacStatus Ieee802154MacPib::getValue(const State& state, const std::string& attribute, Ieee802154MacPibValue& value)
{
#define GET_ADDRESS(name) if (attribute == #name) { value = state.name; return Ieee802154MacStatus::SUCCESS; }
#define GET_INTEGER(name) if (attribute == #name) { value = static_cast<uint64_t>(state.name); return Ieee802154MacStatus::SUCCESS; }
#define GET_BOOLEAN(name) if (attribute == #name) { value = state.name; return Ieee802154MacStatus::SUCCESS; }
    GET_ADDRESS(macExtendedAddress)
    GET_ADDRESS(macCoordExtendedAddress)
    GET_INTEGER(macShortAddress)
    GET_INTEGER(macCoordShortAddress)
    GET_INTEGER(macPanId)
    GET_INTEGER(macDsn)
    GET_INTEGER(macMinBe)
    GET_INTEGER(macMaxBe)
    GET_INTEGER(macMaxCsmaBackoffs)
    GET_INTEGER(macMaxFrameRetries)
    GET_INTEGER(macSifsPeriod)
    GET_INTEGER(macLifsPeriod)
    GET_INTEGER(macUnitBackoffPeriod)
    GET_BOOLEAN(macRxOnWhenIdle)
    GET_BOOLEAN(macImplicitBroadcast)
    GET_BOOLEAN(macGroupRxMode)
    GET_BOOLEAN(macSecurityEnabled)
    GET_INTEGER(macBeaconOrder)
    GET_BOOLEAN(macTimestampSupported)
    GET_INTEGER(macSyncSymbolOffset)
    GET_BOOLEAN(macPromiscuousMode)
    GET_BOOLEAN(macDsmeCapable)
    GET_BOOLEAN(macDsmeEnabled)
    GET_BOOLEAN(macDaCapable)
    GET_BOOLEAN(macDaEnabled)
    GET_BOOLEAN(macExtendedDsmeCapable)
    GET_BOOLEAN(macExtendedDsmeEnabled)
    GET_BOOLEAN(macHoppingCapable)
    GET_BOOLEAN(macHoppingEnabled)
    GET_BOOLEAN(macLeCapable)
    GET_BOOLEAN(macLeEnabled)
    GET_BOOLEAN(macLeHsEnabled)
    GET_BOOLEAN(macMetricsCapable)
    GET_BOOLEAN(macMetricsEnabled)
    GET_BOOLEAN(macRccnCapable)
    GET_BOOLEAN(macRccnEnabled)
    GET_BOOLEAN(macSrmCapable)
    GET_BOOLEAN(macSrmEnabled)
    GET_BOOLEAN(macTrleCapable)
    GET_BOOLEAN(macTrleEnabled)
    GET_BOOLEAN(macTrleRelayingMode)
    GET_BOOLEAN(macTschCapable)
    GET_BOOLEAN(macTschEnabled)
#undef GET_ADDRESS
#undef GET_INTEGER
#undef GET_BOOLEAN
    return Ieee802154MacStatus::UNSUPPORTED_ATTRIBUTE;
}

static bool isReadOnlyAttribute(const std::string& attribute)
{
    return attribute == "macExtendedAddress" || attribute == "macSifsPeriod" ||
            attribute == "macLifsPeriod" || attribute == "macTimestampSupported" ||
            attribute == "macSyncSymbolOffset" || attribute == "macDsmeCapable" ||
            attribute == "macDaCapable" || attribute == "macExtendedDsmeCapable" ||
            attribute == "macHoppingCapable" || attribute == "macLeCapable" ||
            attribute == "macMetricsCapable" || attribute == "macRccnCapable" ||
            attribute == "macSrmCapable" || attribute == "macTrleCapable" ||
            attribute == "macTschCapable";
}

static bool getNonnegativeInteger(const Ieee802154MacPibValue& value, uint64_t& result)
{
    if (const auto *unsignedValue = std::get_if<uint64_t>(&value)) {
        result = *unsignedValue;
        return true;
    }
    if (const auto *signedValue = std::get_if<int64_t>(&value); signedValue != nullptr && *signedValue >= 0) {
        result = static_cast<uint64_t>(*signedValue);
        return true;
    }
    return false;
}

Ieee802154MacStatus Ieee802154MacPib::setValue(State& state, const std::string& attribute, const Ieee802154MacPibValue& value, bool validateCrossAttributes)
{
    Ieee802154MacPibValue currentValue;
    if (getValue(state, attribute, currentValue) != Ieee802154MacStatus::SUCCESS)
        return Ieee802154MacStatus::UNSUPPORTED_ATTRIBUTE;
    if (isReadOnlyAttribute(attribute))
        return Ieee802154MacStatus::READ_ONLY;

    if (attribute == "macCoordExtendedAddress") {
        const auto *address = std::get_if<Ieee802154Address>(&value);
        if (address == nullptr || !isIndividualExtendedAddress(*address))
            return Ieee802154MacStatus::INVALID_PARAMETER;
        state.macCoordExtendedAddress = *address;
        return Ieee802154MacStatus::SUCCESS;
    }

    uint64_t integerValue = 0;
    if (attribute == "macShortAddress" || attribute == "macCoordShortAddress" || attribute == "macPanId") {
        if (!getNonnegativeInteger(value, integerValue) || integerValue > 0xffff)
            return Ieee802154MacStatus::INVALID_PARAMETER;
        if (attribute == "macShortAddress") state.macShortAddress = static_cast<uint16_t>(integerValue);
        else if (attribute == "macCoordShortAddress") state.macCoordShortAddress = static_cast<uint16_t>(integerValue);
        else state.macPanId = static_cast<uint16_t>(integerValue);
        return Ieee802154MacStatus::SUCCESS;
    }
    if (attribute == "macDsn") {
        if (!getNonnegativeInteger(value, integerValue) || integerValue > 0xff)
            return Ieee802154MacStatus::INVALID_PARAMETER;
        state.macDsn = static_cast<uint8_t>(integerValue);
        return Ieee802154MacStatus::SUCCESS;
    }
    if (attribute == "macMinBe") {
        if (!getNonnegativeInteger(value, integerValue) || integerValue > 8 ||
                (validateCrossAttributes && integerValue > state.macMaxBe))
            return Ieee802154MacStatus::INVALID_PARAMETER;
        state.macMinBe = static_cast<uint8_t>(integerValue);
        return Ieee802154MacStatus::SUCCESS;
    }
    if (attribute == "macMaxBe") {
        if (!getNonnegativeInteger(value, integerValue) || integerValue < 3 || integerValue > 8 ||
                (validateCrossAttributes && state.macMinBe > integerValue))
            return Ieee802154MacStatus::INVALID_PARAMETER;
        state.macMaxBe = static_cast<uint8_t>(integerValue);
        return Ieee802154MacStatus::SUCCESS;
    }
    if (attribute == "macMaxCsmaBackoffs" || attribute == "macMaxFrameRetries") {
        const uint64_t maximum = attribute == "macMaxCsmaBackoffs" ? 5 : 7;
        if (!getNonnegativeInteger(value, integerValue) || integerValue > maximum)
            return Ieee802154MacStatus::INVALID_PARAMETER;
        if (attribute == "macMaxCsmaBackoffs") state.macMaxCsmaBackoffs = static_cast<uint8_t>(integerValue);
        else state.macMaxFrameRetries = static_cast<uint8_t>(integerValue);
        return Ieee802154MacStatus::SUCCESS;
    }
    if (attribute == "macUnitBackoffPeriod") {
        if (!getNonnegativeInteger(value, integerValue) || integerValue == 0 || integerValue > std::numeric_limits<uint32_t>::max())
            return Ieee802154MacStatus::INVALID_PARAMETER;
        state.macUnitBackoffPeriod = static_cast<uint32_t>(integerValue);
        return Ieee802154MacStatus::SUCCESS;
    }
    if (attribute == "macBeaconOrder") {
        if (!getNonnegativeInteger(value, integerValue) || integerValue != 15)
            return Ieee802154MacStatus::INVALID_PARAMETER;
        state.macBeaconOrder = static_cast<uint8_t>(integerValue);
        return Ieee802154MacStatus::SUCCESS;
    }
    if (attribute == "macRxOnWhenIdle" || attribute == "macImplicitBroadcast" ||
            attribute == "macGroupRxMode" || attribute == "macPromiscuousMode") {
        const auto *booleanValue = std::get_if<bool>(&value);
        if (booleanValue == nullptr)
            return Ieee802154MacStatus::INVALID_PARAMETER;
        if (attribute == "macRxOnWhenIdle") state.macRxOnWhenIdle = *booleanValue;
        else if (attribute == "macImplicitBroadcast") state.macImplicitBroadcast = *booleanValue;
        else if (attribute == "macGroupRxMode") state.macGroupRxMode = *booleanValue;
        else state.macPromiscuousMode = *booleanValue;
        return Ieee802154MacStatus::SUCCESS;
    }
    if (attribute == "macSecurityEnabled" || attribute == "macDsmeEnabled" || attribute == "macDaEnabled" ||
            attribute == "macExtendedDsmeEnabled" || attribute == "macHoppingEnabled" || attribute == "macLeEnabled" ||
            attribute == "macLeHsEnabled" || attribute == "macMetricsEnabled" || attribute == "macRccnEnabled" ||
            attribute == "macSrmEnabled" || attribute == "macTrleEnabled" || attribute == "macTrleRelayingMode" ||
            attribute == "macTschEnabled") {
        const auto *booleanValue = std::get_if<bool>(&value);
        if (booleanValue == nullptr || *booleanValue)
            return Ieee802154MacStatus::INVALID_PARAMETER;
        if (attribute == "macSecurityEnabled") state.macSecurityEnabled = false;
        else if (attribute == "macDsmeEnabled") state.macDsmeEnabled = false;
        else if (attribute == "macDaEnabled") state.macDaEnabled = false;
        else if (attribute == "macExtendedDsmeEnabled") state.macExtendedDsmeEnabled = false;
        else if (attribute == "macHoppingEnabled") state.macHoppingEnabled = false;
        else if (attribute == "macLeEnabled") state.macLeEnabled = false;
        else if (attribute == "macLeHsEnabled") state.macLeHsEnabled = false;
        else if (attribute == "macMetricsEnabled") state.macMetricsEnabled = false;
        else if (attribute == "macRccnEnabled") state.macRccnEnabled = false;
        else if (attribute == "macSrmEnabled") state.macSrmEnabled = false;
        else if (attribute == "macTrleEnabled") state.macTrleEnabled = false;
        else if (attribute == "macTrleRelayingMode") state.macTrleRelayingMode = false;
        else state.macTschEnabled = false;
        return Ieee802154MacStatus::SUCCESS;
    }

    // getValue() established that this is a selected name; all remaining
    // selected attributes are read-only and were handled above.
    return Ieee802154MacStatus::READ_ONLY;
}

Ieee802154MacPib::Ieee802154MacPib(Ieee802154Address deviceExtendedAddress, uint8_t injectedDsn,
        uint32_t ccaDurationUs, const std::vector<Ieee802154MacSetRequest>& startupOverrides) :
    state(makeDefaultState(deviceExtendedAddress, injectedDsn, ccaDurationUs)),
    ccaDurationUs(ccaDurationUs)
{
    if (!isIndividualExtendedAddress(deviceExtendedAddress))
        throw cRuntimeError("macExtendedAddress must be an individual extended address");

    std::set<std::string> names;
    for (const auto& overrideValue : startupOverrides) {
        if (!names.insert(overrideValue.attribute).second)
            throw cRuntimeError("Duplicate IEEE 802.15.4 MAC PIB startup override '%s'", overrideValue.attribute.c_str());
        const auto status = setValue(state, overrideValue.attribute, overrideValue.value, false);
        if (status != Ieee802154MacStatus::SUCCESS)
            throw cRuntimeError("Invalid IEEE 802.15.4 MAC PIB startup override '%s' (status %u)",
                    overrideValue.attribute.c_str(), static_cast<unsigned int>(status));
    }
    if (state.macMinBe > state.macMaxBe)
        throw cRuntimeError("macMinBe must not exceed macMaxBe after startup overrides");
}

Ieee802154MacGetConfirm Ieee802154MacPib::get(const std::string& attribute) const
{
    Ieee802154MacPibValue value;
    const auto status = getValue(state, attribute, value);
    Ieee802154MacGetConfirm result(attribute, status);
    if (status == Ieee802154MacStatus::SUCCESS)
        result.value = std::move(value);
    return result;
}

Ieee802154MacStatus Ieee802154MacPib::set(const std::string& attribute, const Ieee802154MacPibValue& value)
{
    State candidate = state;
    const auto status = setValue(candidate, attribute, value, true);
    if (status == Ieee802154MacStatus::SUCCESS)
        state = std::move(candidate);
    return status;
}

void Ieee802154MacPib::reset(bool setDefaultPib, uint8_t injectedDsn)
{
    if (setDefaultPib)
        state = makeDefaultState(state.macExtendedAddress, injectedDsn, ccaDurationUs);
}

} // namespace inet
