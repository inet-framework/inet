// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_IEEE80211CAPABILITYELEMENTS_H
#define INET_IEEE80211CAPABILITYELEMENTS_H
#include <functional>
#include "inet/common/MemoryInputStream.h"
#include "inet/common/MemoryOutputStream.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MeshFrame_m.h"
namespace inet::ieee80211 {
inline bool getBit(const std::vector<uint8_t>& bytes, int bit)
{
    return (bytes[bit / 8] & (1 << (bit % 8))) != 0;
}

inline void setBit(std::vector<uint8_t>& bytes, int bit)
{
    bytes[bit / 8] |= 1 << (bit % 8);
}

inline void writeHtCapabilitiesElement(MemoryOutputStream& stream, const Ieee80211HtCapabilitiesElement& capabilities)
{
    // IEEE Std 802.11-2024, 9.4.2.54 and Tables 9-224 to 9-226.
    if (capabilities.maxAmpduLengthExponent < 0 || capabilities.maxAmpduLengthExponent > 3)
        throw cRuntimeError("Malformed Maximum A-MPDU Length Exponent: %d", capabilities.maxAmpduLengthExponent);
    if (capabilities.smPowerSave < 0 || capabilities.smPowerSave > 3 || capabilities.rxStbc < 0 || capabilities.rxStbc > 3)
        throw cRuntimeError("Malformed HT Capability Information: SM Power Save %d, Rx STBC %d", capabilities.smPowerSave, capabilities.rxStbc);
    if (capabilities.minimumMpduStartSpacing < 0 || capabilities.minimumMpduStartSpacing > 7 || capabilities.ampduParametersReserved > 7)
        throw cRuntimeError("Malformed A-MPDU Parameters: Minimum MPDU Start Spacing %d, reserved bits %d",
                capabilities.minimumMpduStartSpacing, capabilities.ampduParametersReserved);
    if (capabilities.rxHighestSupportedDataRate < 0 || capabilities.rxHighestSupportedDataRate > 1023)
        throw cRuntimeError("Malformed Rx Highest Supported Data Rate: %d", capabilities.rxHighestSupportedDataRate);
    if (!capabilities.txMcsSetDefined && (capabilities.txRxMcsSetNotEqual || capabilities.txMaxNss != 0 || capabilities.txUnequalModulation))
        throw cRuntimeError("Malformed undefined HT Tx MCS Set");
    if (capabilities.txMcsSetDefined && !capabilities.txRxMcsSetNotEqual &&
            (capabilities.txMaxNss != 0 || capabilities.txUnequalModulation))
        throw cRuntimeError("Malformed equal HT Tx/Rx MCS Set");
    if (capabilities.txMcsSetDefined && capabilities.txRxMcsSetNotEqual &&
            (capabilities.txMaxNss < 1 || capabilities.txMaxNss > 4))
        throw cRuntimeError("Malformed HT Tx Maximum Number of Spatial Streams: %d", capabilities.txMaxNss);

    stream.writeByte(45);
    stream.writeByte(26);
    uint16_t information = (capabilities.ldpc ? 1 : 0) |
            (capabilities.supportedChannelWidth40Mhz ? 1 << 1 : 0) |
            (capabilities.smPowerSave << 2) |
            (capabilities.greenfield ? 1 << 4 : 0) |
            (capabilities.shortGi20 ? 1 << 5 : 0) |
            (capabilities.shortGi40 ? 1 << 6 : 0) |
            (capabilities.txStbc ? 1 << 7 : 0) |
            (capabilities.rxStbc << 8) |
            (capabilities.htDelayedBlockAck ? 1 << 10 : 0) |
            (capabilities.maxAmsduLength7935 ? 1 << 11 : 0) |
            (capabilities.dsssCckModeIn40Mhz ? 1 << 12 : 0) |
            (capabilities.fortyMhzIntolerant ? 1 << 14 : 0) |
            (capabilities.lsigTxopProtectionSupport ? 1 << 15 : 0);
    stream.writeUint16Le(information);
    stream.writeByte(capabilities.maxAmpduLengthExponent | (capabilities.minimumMpduStartSpacing << 2) | (capabilities.ampduParametersReserved << 5));

    std::vector<uint8_t> mcs(16, 0);
    for (int mcsIndex = 0; mcsIndex < 77; mcsIndex++)
        if (capabilities.rxMcsSupported[mcsIndex])
            setBit(mcs, mcsIndex);
    for (int bit = 0; bit < 10; bit++)
        if (capabilities.rxHighestSupportedDataRate & (1 << bit))
            setBit(mcs, 80 + bit);
    if (capabilities.txMcsSetDefined)
        setBit(mcs, 96);
    if (capabilities.txMcsSetDefined && capabilities.txRxMcsSetNotEqual) {
        setBit(mcs, 97);
        if ((capabilities.txMaxNss - 1) & 1)
            setBit(mcs, 98);
        if ((capabilities.txMaxNss - 1) & 2)
            setBit(mcs, 99);
        if (capabilities.txUnequalModulation)
            setBit(mcs, 100);
    }
    for (auto byte : mcs)
        stream.writeByte(byte);
    stream.writeUint16Le(capabilities.htExtendedCapabilities);
    stream.writeUint32Le(capabilities.transmitBeamformingCapabilities);
    stream.writeByte(capabilities.aselCapabilities);
}

inline void writeHtOperationElement(MemoryOutputStream& stream, const Ieee80211HtOperationElement& operation, bool basicMcsSetPresent)
{
    // IEEE Std 802.11-2024, 9.4.2.55 and Table 9-230.
    if (operation.primaryChannel < 0 || operation.primaryChannel > 255 ||
            operation.secondaryChannelOffset < 0 || operation.secondaryChannelOffset > 3 || operation.secondaryChannelOffset == 2 ||
            operation.protectionMode < 0 || operation.protectionMode > 3 ||
            operation.channelCenterFrequencySegment2 < 0 || operation.channelCenterFrequencySegment2 > 255)
        throw cRuntimeError("Malformed HT Operation element fields");
    stream.writeByte(61);
    stream.writeByte(22);
    stream.writeByte(operation.primaryChannel);
    uint64_t information = (operation.secondaryChannelOffset & 3) |
            (operation.staChannelWidth40Mhz ? uint64_t(1) << 2 : 0) |
            (operation.rifsMode ? uint64_t(1) << 3 : 0) |
            (uint64_t(operation.protectionMode) << 8) |
            (operation.nongreenfieldHtStasPresent ? uint64_t(1) << 10 : 0) |
            (operation.obssNonHtStasPresent ? uint64_t(1) << 12 : 0) |
            (uint64_t(operation.channelCenterFrequencySegment2) << 13) |
            (operation.dualBeacon ? uint64_t(1) << 30 : 0) |
            (operation.dualCtsProtection ? uint64_t(1) << 31 : 0) |
            (operation.stbcBeacon ? uint64_t(1) << 32 : 0);
    for (int i = 0; i < 5; i++)
        stream.writeByte((information >> (i * 8)) & 0xff);
    std::vector<uint8_t> basicMcs(16, 0);
    if (basicMcsSetPresent)
        for (int mcsIndex = 0; mcsIndex < 77; mcsIndex++)
            if (operation.basicMcsSupported[mcsIndex])
                setBit(basicMcs, mcsIndex);
    for (auto byte : basicMcs)
        stream.writeByte(byte);
}

inline void readHtCapabilitiesElement(MemoryInputStream& stream, int length, Ieee80211MeshFrame& frame)
{
    if (length != 26 || frame.getHtCapabilitiesPresent()) {
        frame.markIncorrect();
        stream.seek(stream.getPosition() + B(length));
        return;
    }
    Ieee80211HtCapabilitiesElement capabilities;
    uint16_t information = stream.readUint16Le();
    capabilities.ldpc = information & 1;
    capabilities.supportedChannelWidth40Mhz = information & (1 << 1);
    capabilities.smPowerSave = (information >> 2) & 3;
    capabilities.greenfield = information & (1 << 4);
    capabilities.shortGi20 = information & (1 << 5);
    capabilities.shortGi40 = information & (1 << 6);
    capabilities.txStbc = information & (1 << 7);
    capabilities.rxStbc = (information >> 8) & 3;
    capabilities.htDelayedBlockAck = information & (1 << 10);
    capabilities.maxAmsduLength7935 = information & (1 << 11);
    capabilities.dsssCckModeIn40Mhz = information & (1 << 12);
    capabilities.fortyMhzIntolerant = information & (1 << 14);
    capabilities.lsigTxopProtectionSupport = information & (1 << 15);
    uint8_t ampduParameters = stream.readByte();
    capabilities.maxAmpduLengthExponent = ampduParameters & 3;
    capabilities.minimumMpduStartSpacing = (ampduParameters >> 2) & 7;
    capabilities.ampduParametersReserved = ampduParameters >> 5;
    std::vector<uint8_t> mcs(16);
    for (auto& byte : mcs)
        byte = stream.readByte();
    for (int mcsIndex = 0; mcsIndex < 77; mcsIndex++)
        capabilities.rxMcsSupported[mcsIndex] = getBit(mcs, mcsIndex);
    capabilities.rxHighestSupportedDataRate = 0;
    for (int bit = 0; bit < 10; bit++)
        if (getBit(mcs, 80 + bit))
            capabilities.rxHighestSupportedDataRate |= 1 << bit;
    capabilities.txMcsSetDefined = getBit(mcs, 96);
    capabilities.txRxMcsSetNotEqual = getBit(mcs, 97);
    bool txNssBitsSet = getBit(mcs, 98) || getBit(mcs, 99);
    bool txUnequalModulation = getBit(mcs, 100);
    if (!capabilities.txMcsSetDefined && (capabilities.txRxMcsSetNotEqual || txNssBitsSet || txUnequalModulation))
        frame.markIncorrect();
    if (capabilities.txMcsSetDefined && !capabilities.txRxMcsSetNotEqual && (txNssBitsSet || txUnequalModulation))
        frame.markIncorrect();
    capabilities.txMaxNss = capabilities.txRxMcsSetNotEqual ?
            (getBit(mcs, 98) ? 1 : 0) + (getBit(mcs, 99) ? 2 : 0) + 1 : 0;
    capabilities.txUnequalModulation = txUnequalModulation;
    capabilities.htExtendedCapabilities = stream.readUint16Le();
    capabilities.transmitBeamformingCapabilities = stream.readUint32Le();
    capabilities.aselCapabilities = stream.readByte();
    frame.setHtCapabilitiesPresent(true);
    frame.setHtCapabilities(capabilities);
}

inline void readHtOperationElement(MemoryInputStream& stream, int length, Ieee80211MeshFrame& frame, bool basicMcsSetPresent)
{
    if (length != 22 || frame.getHtOperationPresent()) {
        frame.markIncorrect();
        stream.seek(stream.getPosition() + B(length));
        return;
    }
    Ieee80211HtOperationElement operation;
    operation.primaryChannel = stream.readByte();
    uint64_t information = 0;
    for (int i = 0; i < 5; i++)
        information |= uint64_t(stream.readByte()) << (i * 8);
    operation.secondaryChannelOffset = information & 3;
    operation.staChannelWidth40Mhz = information & (1 << 2);
    operation.rifsMode = information & (1 << 3);
    operation.protectionMode = (information >> 8) & 3;
    operation.nongreenfieldHtStasPresent = information & (1 << 10);
    operation.obssNonHtStasPresent = information & (1 << 12);
    operation.channelCenterFrequencySegment2 = (information >> 13) & 0xff;
    operation.dualBeacon = information & (uint64_t(1) << 30);
    operation.dualCtsProtection = information & (uint64_t(1) << 31);
    operation.stbcBeacon = information & (uint64_t(1) << 32);
    std::vector<uint8_t> basicMcs(16);
    for (auto& byte : basicMcs)
        byte = stream.readByte();
    if (basicMcsSetPresent)
        for (int mcsIndex = 0; mcsIndex < 77; mcsIndex++)
            operation.basicMcsSupported[mcsIndex] = getBit(basicMcs, mcsIndex);
    if (operation.secondaryChannelOffset == 2)
        frame.markIncorrect();
    frame.setHtOperationPresent(true);
    frame.setHtOperation(operation);
}

inline void writeVhtElements(MemoryOutputStream& stream, const Ieee80211MeshFrame& frame, const std::function<void()>& before = {})
{
    if (frame.getVhtCapabilitiesPresent()) {
        if (before) before();
        auto& c = frame.getVhtCapabilities();
        stream.writeByte(191); stream.writeByte(12);
        stream.writeUint32Le(c.capabilitiesInfo);
        stream.writeUint16Le(c.rxMcsMap); stream.writeUint16Le(c.rxHighestSupportedDataRate);
        stream.writeUint16Le(c.txMcsMap); stream.writeUint16Le(c.txHighestSupportedDataRate);
    }
    if (frame.getVhtOperationPresent()) {
        if (before) before();
        auto& o = frame.getVhtOperation();
        stream.writeByte(192); stream.writeByte(5);
        stream.writeByte(o.channelWidth); stream.writeByte(o.centerFrequencySegment0); stream.writeByte(o.centerFrequencySegment1);
        stream.writeUint16Le(o.basicMcsMap);
    }
}
inline bool readVhtElement(MemoryInputStream& stream, uint8_t id, uint8_t length, Ieee80211MeshFrame& frame)
{
    if (id == 191) {
        if (length != 12 || frame.getVhtCapabilitiesPresent()) return false;
        Ieee80211VhtCapabilitiesElement c;
        c.capabilitiesInfo = stream.readUint32Le();
        c.rxMcsMap = stream.readUint16Le(); c.rxHighestSupportedDataRate = stream.readUint16Le();
        c.txMcsMap = stream.readUint16Le(); c.txHighestSupportedDataRate = stream.readUint16Le();
        if ((c.capabilitiesInfo & 3) == 3 || ((c.capabilitiesInfo >> 2) & 3) == 3) return false;
        frame.setVhtCapabilities(c); frame.setVhtCapabilitiesPresent(true);
    }
    else if (id == 192) {
        if (length != 5 || frame.getVhtOperationPresent()) return false;
        Ieee80211VhtOperationElement o;
        o.channelWidth = stream.readByte(); o.centerFrequencySegment0 = stream.readByte(); o.centerFrequencySegment1 = stream.readByte();
        o.basicMcsMap = stream.readUint16Le();
        if (o.channelWidth > 3) return false;
        frame.setVhtOperation(o); frame.setVhtOperationPresent(true);
    }
    else return false;
    return true;
}
} // namespace inet::ieee80211
#endif
