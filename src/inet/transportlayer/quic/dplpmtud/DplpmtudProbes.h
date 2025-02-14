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

#ifndef INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDPROBES_H_
#define INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDPROBES_H_

#include "DplpmtudProbe.h"
#include <map>

namespace inet {
namespace quic {

class DplpmtudProbes : public std::map<int, DplpmtudProbe *> {
public:
    DplpmtudProbes();
    virtual ~DplpmtudProbes();

    void add(DplpmtudProbe *probe);
    void removeAll();
    void removeAllEqualOrSmaller(int size);
    void removeAllLarger(int size);
    void removeAllEqualOrLarger(int size);
    DplpmtudProbe *getBySize(int size);
    DplpmtudProbe *getSmallest();
    DplpmtudProbe *getSmallestLost();
    bool containsProbesNotLost();
    std::string str();
};

} /* namespace quic */
} /* namespace inet */

#endif /* INET_TRANSPORTLAYER_QUIC_DPLPMTUD_DPLPMTUDPROBES_H_ */
