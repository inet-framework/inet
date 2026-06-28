//
// Copyright (C) 2012 - 2016 Brno University of Technology (http://nes.fit.vutbr.cz/ansa)
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
// IS-IS identifier value types, ported from the ANSA project (ISISCommon.h)
// to modern INET. These are plain C++ value classes (not cObjects): a System
// ID, an Area ID, and the derived Pseudonode ID and LSP ID. They are used both
// as fields in IS-IS PDUs and as keys in the protocol's databases, hence the
// full set of comparison operators.
//

#ifndef __INET_ISISCOMMON_H
#define __INET_ISISCOMMON_H

#include <climits>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <string>

#include "inet/common/INETDefs.h"

namespace inet {
namespace isis {

//
// 6-byte IS-IS System ID, packed into a uint64_t. Printed as "0000.0000.0000".
//
class SystemId
{
  protected:
    uint64_t systemId = 0;

  public:
    SystemId() {}
    SystemId(uint64_t sysId) : systemId(sysId) {}
    virtual ~SystemId() {}

    uint64_t getSystemId() const { return systemId; }
    void setSystemId(uint64_t sysId) { systemId = sysId; }

    virtual uint64_t toInt() const { return systemId; }

    virtual std::string str() const
    {
        char buf[16];
        snprintf(buf, sizeof(buf), "%04X.%04X.%04X",
                (unsigned)((systemId >> 32) & 0xFFFF),
                (unsigned)((systemId >> 16) & 0xFFFF),
                (unsigned)(systemId & 0xFFFF));
        return std::string(buf);
    }

    bool operator==(const SystemId& o) const { return systemId == o.getSystemId(); }
    bool operator!=(const SystemId& o) const { return systemId != o.getSystemId(); }
    bool operator<(const SystemId& o) const { return systemId < o.getSystemId(); }
    bool operator<=(const SystemId& o) const { return systemId <= o.getSystemId(); }
    bool operator>(const SystemId& o) const { return systemId > o.getSystemId(); }
    bool operator>=(const SystemId& o) const { return systemId >= o.getSystemId(); }
};

inline std::ostream& operator<<(std::ostream& os, const SystemId& sysId)
{
    return os << sysId.str();
}

//
// 3-byte IS-IS Area ID, packed into a uint64_t. Printed as "49.0001".
//
class AreaId
{
  private:
    uint64_t areaId = 0;

  public:
    AreaId() {}
    AreaId(uint64_t areaId) : areaId(areaId) {}

    uint64_t getAreaId() const { return areaId; }
    void setAreaId(uint64_t areaId) { this->areaId = areaId; }

    uint64_t toInt() const { return areaId; }

    std::string str() const
    {
        char buf[16];
        snprintf(buf, sizeof(buf), "%02X.%04X",
                (unsigned)((areaId >> 16) & 0xFF),
                (unsigned)(areaId & 0xFFFF));
        return std::string(buf);
    }

    bool operator==(const AreaId& o) const { return areaId == o.getAreaId(); }
    bool operator!=(const AreaId& o) const { return areaId != o.getAreaId(); }
    bool operator<(const AreaId& o) const { return areaId < o.getAreaId(); }
    bool operator<=(const AreaId& o) const { return areaId <= o.getAreaId(); }
    bool operator>(const AreaId& o) const { return areaId > o.getAreaId(); }
    bool operator>=(const AreaId& o) const { return areaId >= o.getAreaId(); }
};

inline std::ostream& operator<<(std::ostream& os, const AreaId& areaId)
{
    return os << areaId.str();
}

//
// A System ID extended with a 1-byte pseudonode (circuit) ID, identifying
// either a real IS (circuit ID 0) or a LAN pseudonode. Printed as
// "0000.0000.0000.00".
//
class PseudonodeId : public SystemId
{
  protected:
    unsigned int circuitId = 0;

  public:
    PseudonodeId() {}
    PseudonodeId(uint64_t sysId, unsigned int circId) { systemId = sysId; circuitId = circId; }
    PseudonodeId(SystemId sysId, unsigned int circId) { systemId = sysId.getSystemId(); circuitId = circId; }

    void set(SystemId sysId, unsigned int circId) { systemId = sysId.getSystemId(); circuitId = circId; }

    unsigned int getCircuitId() const { return circuitId; }
    void setCircuitId(unsigned int circuitId) { this->circuitId = circuitId; }

    SystemId getSystemId() const { return SystemId(systemId); }
    void setSystemId(const SystemId systemId) { this->systemId = systemId.getSystemId(); }

    virtual uint64_t toInt() const override { return (systemId << 8) + circuitId; }

    virtual std::string str() const override
    {
        char buf[8];
        snprintf(buf, sizeof(buf), ".%02X", circuitId & 0xFF);
        return SystemId::str() + buf;
    }

    bool operator==(const PseudonodeId& o) const { return toInt() == o.toInt(); }
    bool operator!=(const PseudonodeId& o) const { return toInt() != o.toInt(); }
    bool operator<(const PseudonodeId& o) const { return toInt() < o.toInt(); }
    bool operator<=(const PseudonodeId& o) const { return toInt() <= o.toInt(); }
    bool operator>(const PseudonodeId& o) const { return toInt() > o.toInt(); }
    bool operator>=(const PseudonodeId& o) const { return toInt() >= o.toInt(); }
};

inline std::ostream& operator<<(std::ostream& os, const PseudonodeId& pseudoId)
{
    return os << pseudoId.str();
}

//
// A Pseudonode ID extended with a 1-byte fragment ID, identifying one fragment
// of a Link State PDU. Printed as "0000.0000.0000.00-00".
//
class LspId : public PseudonodeId
{
  protected:
    unsigned int fragmentId = 0;

  public:
    LspId() {}
    LspId(SystemId sysId) : PseudonodeId(sysId, 0) {}
    LspId(PseudonodeId pseudoId) : PseudonodeId(pseudoId) {}

    void set(const SystemId sysId) { setSystemId(sysId); circuitId = 0; fragmentId = 0; }
    void set(const PseudonodeId pseudoId) { setSystemId(pseudoId.getSystemId()); circuitId = pseudoId.getCircuitId(); fragmentId = 0; }

    PseudonodeId getPseudonodeId() const { return PseudonodeId(getSystemId(), circuitId); }

    unsigned int getFragmentId() const { return fragmentId; }
    void setFragmentId(unsigned int fragmentId) { this->fragmentId = fragmentId; }

    virtual uint64_t toInt() const override { return (PseudonodeId::toInt() << 8) + fragmentId; }

    // Sets the largest possible LSP ID; used as the upper bound of CSNP ranges.
    void setMax() { systemId = UINT64_MAX; circuitId = UINT_MAX; fragmentId = UINT_MAX; }

    virtual std::string str() const override
    {
        char buf[8];
        snprintf(buf, sizeof(buf), "-%02X", fragmentId & 0xFF);
        return PseudonodeId::str() + buf;
    }

    bool operator==(const LspId& o) const { return toInt() == o.toInt(); }
    bool operator!=(const LspId& o) const { return toInt() != o.toInt(); }
    bool operator<(const LspId& o) const { return toInt() < o.toInt(); }
    bool operator<=(const LspId& o) const { return toInt() <= o.toInt(); }
    bool operator>(const LspId& o) const { return toInt() > o.toInt(); }
    bool operator>=(const LspId& o) const { return toInt() >= o.toInt(); }
};

inline std::ostream& operator<<(std::ostream& os, const LspId& lspId)
{
    return os << lspId.str();
}

} // namespace isis
} // namespace inet

#endif
