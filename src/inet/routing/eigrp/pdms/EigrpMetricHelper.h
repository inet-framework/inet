//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
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
/**
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#ifndef EIGRPMETRICHELPER_H_
#define EIGRPMETRICHELPER_H_

//#include "InterfaceEntry.h"

#include "inet/routing/eigrp/messages/EigrpMessage_m.h"
#include "inet/routing/eigrp/tables/EigrpInterfaceTable.h"
namespace inet {
namespace eigrp {
bool operator==(const EigrpKValues& k1, const EigrpKValues& k2);

/**
 * Class for EIGRP metric computation.
 */
class EigrpMetricHelper
{
  private:
    // Constants for metric computation
    const uint32_t DELAY_PICO;
    const uint32_t BANDWIDTH;
    const uint32_t CLASSIC_SCALE;
    const uint32_t WIDE_SCALE;

    /**
     * Returns smaller of two parameters.
     */
    unsigned int getMin(unsigned int p1, unsigned int p2) { return (p1 < p2) ? p1 : p2; }
    /**
     * Returns greater of two parameters.
     */
    unsigned int getMax(unsigned int p1, unsigned int p2) { return (p1 < p2) ? p2 : p1; }

  public:
    // Constants for unreachable route
    static const uint64_t DELAY_INF = 0xFFFFFFFFFFFF;       // 2^48
    static const uint64_t BANDWIDTH_INF = 0xFFFFFFFFFFFF;   // 2^48
    static const uint64_t METRIC_INF = 0xFFFFFFFFFFFFFF;    // 2^56

    EigrpMetricHelper();
    virtual ~EigrpMetricHelper();

    /**
     * Sets parameters from interface for metric computation.
     */
    EigrpWideMetricPar getParam(EigrpInterface *eigrpIface);
    /**
     * Adjust parameters of metric by interface parameters.
     */
    EigrpWideMetricPar adjustParam(const EigrpWideMetricPar& ifParam, const EigrpWideMetricPar& neighParam);
    /**
     * Computes classic metric.
     */
    uint64_t computeClassicMetric(const EigrpWideMetricPar& par, const EigrpKValues& kValues);
    /**
     * Computes wide metric.
     */
    uint64_t computeWideMetric(const EigrpWideMetricPar& par, const EigrpKValues& kValues);
    /**
     * Compares metric enabled parameters.
     */
    bool compareParameters(const EigrpWideMetricPar& par1, const EigrpWideMetricPar& par2, EigrpKValues& kValues);
    /**
     * Returns true, if parameters are set to infinite, otherwise false.
     */
    bool isParamMaximal(const EigrpWideMetricPar& par) { return par.delay == DELAY_INF; }
};
} //eigrp
} //inet
#endif /* EIGRPMETRICHELPER_H_ */
