//
// Copyright (C) 2020 OpenSim Ltd and Marcel Marek
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
// along with this program.  If not, see http://www.gnu.org/licenses/.
//  along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef INET_NETWORKLAYER_IPV4_IPSEC_IPSECRULE_H_
#define INET_NETWORKLAYER_IPV4_IPSEC_IPSECRULE_H_

#include "inet/common/INETDefs.h"
#include "PacketSelector.h"

// Some Windows header defines IN and OUT as macros, which clashes with our use of those words.
#ifdef _WIN32
#  ifdef IN
#    undef IN
#  endif
#  ifdef OUT
#    undef OUT
#  endif
#endif

namespace inet {
namespace ipsec {

/**
 * Encapsulates an IPsec rule: "if packet matches the given direction and selector,
 * take action DISCARD, BYPASS or PROTECT, and for the last one, protection
 * should be AH, ESP or both".
 */
class INET_API IPsecRule
{
  public:
    enum class Direction {
        INVALID,
        IN,
        OUT
    };

    enum class Action {
        DISCARD,
        BYPASS,
        PROTECT
    };

    enum class Protection {
        NONE,
        AH,
        ESP
    };

  private:
    Direction direction = Direction::INVALID;
    PacketSelector selector;
    Action action = Action::DISCARD;
    Protection protection = Protection::NONE;  // if action=PROTECT
    int icvNumBits = 0;

  public:
    Direction getDirection() const { return direction; }
    void setDirection(Direction direction) { this->direction = direction; }
    const PacketSelector& getSelector() const { return selector; }
    void setSelector(const PacketSelector& selector) { this->selector = selector; }
    Action getAction() const { return action; }
    void setAction(Action action = Action::DISCARD) { this->action = action; }
    Protection getProtection() const { return protection; }
    void setProtection(Protection protection) { this->protection = protection; }
    int getIcvNumBits() const { return icvNumBits; }
    void setIcvNumBits(int len) { this->icvNumBits = len; }
    std::string str() const;
};

inline std::ostream& operator<<(std::ostream& os, const IPsecRule& e)
{
    return os << e.str();
}

}  // namespace ipsec
}  // namespace inet

#endif /* INET_NETWORKLAYER_IPV4_IPSEC_IPSECRULE_H_ */

