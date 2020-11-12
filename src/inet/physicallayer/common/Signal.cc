//
// Copyright (C) 2020 OpenSim Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/common/Signal.h"

#include "inet/common/Units.h"
#include "inet/common/packet/Packet.h"

namespace inet {
namespace physicallayer {

using namespace inet::units::values;

Register_Class(Signal);

Signal::Signal(const char *name, short kind, int64_t bitLength) :
    cPacket(name, kind, bitLength)
{
}

Signal::Signal(const Signal& other) :
    cPacket(other)
{
}

const char *Signal::getFullName() const
{
    if (!isUpdate())
        return getName();
    else {
        const char *suffix;
        if (getRemainingDuration().isZero())
            suffix = ":end";
        else if (getRemainingDuration() == getDuration())
            suffix = ":start";
        else
            suffix = ":progress";
        static std::set<std::string> pool;
        std::string fullname = std::string(getName()) + suffix;
        auto it = pool.insert(fullname).first;
        return it->c_str();
    }
}

std::ostream& Signal::printToStream(std::ostream& stream, int level, int evFlags) const
{
    std::string className = getClassName();
    auto index = className.rfind("::");
    if (index != std::string::npos)
        className = className.substr(index + 2);
    if (level <= PRINT_LEVEL_DETAIL)
        stream << EV_FAINT << "(" << className << ")" << EV_NORMAL;
    stream << EV_ITALIC << getName() << EV_NORMAL << " (" << simsec(getDuration()) << " " << b(getBitLength()) << ")";
    auto packet = check_and_cast<Packet *>(getEncapsulatedPacket());
    packet->printToStream(stream, level + 1, evFlags);
    return stream;
}

std::string Signal::str() const
{
    std::stringstream stream;
    stream << "(" << simsec(getDuration()) << " " << b(getBitLength()) << ")";
    if (auto packet = getEncapsulatedPacket())
        stream << " " << packet;
    return stream.str();
}

} // namespace physicallayer
} // namespace inet

