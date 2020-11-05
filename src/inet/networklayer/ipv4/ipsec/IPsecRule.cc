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

#include "inet/networklayer/ipv4/ipsec/IPsecRule.h"

namespace inet {
namespace ipsec {

std::string IPsecRule::str() const
{
    std::stringstream out;
    out << "Rule: " << selector;

    out << " Direction: ";
    switch (direction) {
        case Direction::INVALID:
            out << "n/a";
            break;

        case Direction::IN:
            out << "IN";
            break;

        case Direction::OUT:
            out << "OUT";
            break;
    }

    out << " Action: ";
    switch (action) {
        case Action::BYPASS:
            out << "BYPASS";
            break;

        case Action::DISCARD:
            out << "DISCARD";
            break;

        case Action::PROTECT:
            out << "PROTECT";
            out << " with: ";
            switch (protection) {
                case Protection::NONE:
                    out << "NONE";
                    break;

                case Protection::AH:
                    out << "AH";
                    break;

                case Protection::ESP:
                    out << "ESP";
                    break;
            }
    }

    return out.str();
}

}    // namespace ipsec
}    // namespace inet

