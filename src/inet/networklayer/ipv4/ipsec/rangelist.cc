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

#include "rangelist.h"

namespace inet {
namespace ipsec {

static std::string trim(const std::string& text)
{
    int length = text.length();
    int pos = 0;
    while (pos < length && isspace(text[pos]))
        pos++;
    int endpos = length;
    while (endpos > pos && isspace(text[endpos-1]))
        endpos--;
    return text.substr(pos, endpos-pos);
}

std::vector<std::pair<std::string,std::string>> rangelist_preparse(const std::string& text)
{
    std::vector<std::pair<std::string,std::string>> res;
    for (std::string elem : cStringTokenizer(text.c_str(), ",").asVector()) {
        auto pos = elem.find("-");
        if (pos == std::string::npos)
            res.push_back(std::make_pair(trim(elem),""));
        else
            res.push_back(std::make_pair(trim(elem.substr(0,pos)), trim(elem.substr(pos+1))));
    }
    return res;
}

}  // namespace ipsec
}  // namespace inet

