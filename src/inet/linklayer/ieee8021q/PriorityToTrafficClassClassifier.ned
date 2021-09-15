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

package inet.linklayer.ieee8021q;

module PriorityToTrafficClassClassifier extends PcpClassifier
{
    // IEEE Standard for Local and Metropolitan Area Networks, Bridges and Bridged Networks
    // Table 8-5—Recommended priority to traffic class mappings
    object priorityToTrafficClassMappings = default([[0, 0, 0, 0, 0, 1, 1, 1],
                                                     [0, 0, 0, 0, 0, 0, 0, 0],
                                                     [0, 0, 0, 1, 1, 2, 2, 2],
                                                     [0, 0, 0, 1, 1, 2, 3, 3],
                                                     [0, 1, 1, 2, 2, 3, 4, 4],
                                                     [0, 1, 1, 2, 2, 3, 4, 5],
                                                     [0, 1, 2, 3, 3, 4, 5, 6],
                                                     [0, 1, 2, 3, 4, 5, 6, 7]]);
    int numTrafficClasses = default(sizeof(out)); // number of available traffic classes
    pcpToGateIndex = default([priorityToTrafficClassMappings.get(0).get(numTrafficClasses),
                              priorityToTrafficClassMappings.get(1).get(numTrafficClasses),
                              priorityToTrafficClassMappings.get(2).get(numTrafficClasses),
                              priorityToTrafficClassMappings.get(3).get(numTrafficClasses),
                              priorityToTrafficClassMappings.get(4).get(numTrafficClasses),
                              priorityToTrafficClassMappings.get(5).get(numTrafficClasses),
                              priorityToTrafficClassMappings.get(6).get(numTrafficClasses),
                              priorityToTrafficClassMappings.get(7).get(numTrafficClasses)]);
}
