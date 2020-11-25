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

#ifndef INET_NETWORKLAYER_IPV4_IPSEC_ENUM_H_
#define INET_NETWORKLAYER_IPV4_IPSEC_ENUM_H_

#include <algorithm>
#include <exception>
#include <unordered_map>
#include "inet/common/INETDefs.h"
#include "inet/common/INETUtils.h"

namespace inet {
namespace ipsec {

/**
 * Utility class for mapping enum values to/from their string representation.
 * E is expected to be an enum name.
 */
template<typename E>
class Enum {
private:
    struct EnumClassHash{
        template <typename T>
        std::size_t operator()(T t) const{
            return static_cast<std::size_t>(t);
        }
    };

    std::unordered_map<E,std::string,EnumClassHash> map;

public:
    Enum(std::initializer_list<typename std::unordered_map<E,std::string,EnumClassHash>::value_type> values) : map(values) {}
    const char *nameOf(E e) const {
        auto it = map.find(e);
        if (it == map.end())
            throw cRuntimeError("Invalid value %d for enum %s", (int)e, opp_typename(typeid(E)));
        return it->second.c_str();
    }
    E valueFor(const char *name) const {
        for (auto p : map)
            if (p.second == name)
                return p.first;
        throw cRuntimeError("Invalid value \"%s\" for enum %s, must be one of (%s)", name, opp_typename(typeid(E)), utils::join(getValueStrings(), ", ").c_str());
    }
    E valueFor(const char *name, E fallback) const {
        for (auto p : map)
            if (p.second == name)
                return p.first;
        return fallback;
    }
    std::vector<std::string> getValueStrings() const {
        std::vector<std::string> result;
        for (auto p : map)
            result.push_back(p.second);
        std::sort(result.begin(), result.end());
        return result;
    }
};

}  // namespace ipsec
}  // namespace inet

#endif /* INET_NETWORKLAYER_IPV4_IPSEC_ENUM_H_ */

