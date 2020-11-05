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
//

#ifndef INET_NETWORKLAYER_IPV4_IPSEC_RANGELIST_H_
#define INET_NETWORKLAYER_IPV4_IPSEC_RANGELIST_H_

#include <vector>
#include <utility>
#include <functional>
#include "inet/common/INETDefs.h"

namespace inet {
namespace ipsec {

std::vector<std::pair<std::string,std::string>> rangelist_preparse(const std::string& text);

/**
 * Utility class that stores a multiple values and/or value ranges, and
 * can answer the question whether a value x is contained in the set.
 *
 * Conversion from/to string form is provided. The string form is a comma-
 * separated list of values and/or ranges. Ranges use hyphen to separate
 * the start/end values (both inclusive). Example for rangelist<int>:
 * "1,5,8-12"
 */
template<typename T>
struct rangelist {
    std::vector<std::pair<T,T>> ranges;

    rangelist() {}
    rangelist(T x) {ranges.push_back(std::make_pair(x,x));}
    rangelist(T a, T b) {ranges.push_back(std::make_pair(a,b));}
    rangelist(const std::vector<std::pair<T,T>>& ranges) : ranges(ranges) {}

    bool empty() const {return ranges.empty();}

    bool contains(T x) const {
        for (const auto& r : ranges)
            if (r.first <= x && x <= r.second)
                return true;
        return false;
    }

    static rangelist<T> parse(const std::string& text, std::function<T(std::string)> fromString) {
        rangelist<T> res;
        for (const auto& r : rangelist_preparse(text)) {
            T a = fromString(r.first);
            T b = r.second.empty() ? a : fromString(r.second);
            res.ranges.push_back(std::make_pair(a,b));
        }
        return res;
    }

    std::string str() const {
        std::stringstream os;
        bool first = true;
        for (const auto& r : ranges) {
            if (!first)
                os << ", ";
            else
                first = false;
            if (r.first == r.second)
                os << r.first;
            else
                os << r.first << "-" << r.second;
        }
        return os.str();
    }
};

}  // namespace ipsec
}  // namespace inet

#endif /* INET_NETWORKLAYER_IPV4_IPSEC_RANGELIST_H_ */

