//
// Copyright (C) 2009 - today Brno University of Technology, Czech Republic
//
// SPDX-License-Identifier: LGPL-3.0-or-later
//
/**
 * @author Jan Zavrel (honza.zavrel96@gmail.com)
 * @author Jan Bloudicek (jbloudicek@gmail.com)
 * @author Vit Rek (rek@kn.vutbr.cz)
 * @author Vladimir Vesely (ivesely@fit.vutbr.cz)
 * @copyright Brno University of Technology (www.fit.vutbr.cz) under GPLv3
 */

#include "inet/routing/eigrp/pdms/EigrpMetricHelper.h"

namespace inet {
namespace eigrp {
bool operator==(const EigrpKValues& k1, const EigrpKValues& k2)
{
    return k1.K1 == k2.K1 && k1.K2 == k2.K2 &&
           k1.K3 == k2.K3 && k1.K4 == k2.K4 &&
           k1.K5 == k2.K5 && k1.K6 == k2.K6;
}

EigrpMetricHelper::EigrpMetricHelper() :
    DELAY_PICO(1000000), BANDWIDTH(10000000), CLASSIC_SCALE(256), WIDE_SCALE(65536)
{
}

EigrpMetricHelper::~EigrpMetricHelper()
{
}

EigrpWideMetricPar EigrpMetricHelper::getParam(EigrpInterface *eigrpIface)
{
    EigrpWideMetricPar newMetricPar;

    newMetricPar.bandwidth = eigrpIface->getBandwidth();
    newMetricPar.delay = eigrpIface->getDelay() * DELAY_PICO;
    newMetricPar.load = eigrpIface->getLoad();
    newMetricPar.reliability = eigrpIface->getReliability();
    newMetricPar.hopCount = 0;
    newMetricPar.mtu = eigrpIface->getMtu();

    return newMetricPar;
}

EigrpWideMetricPar EigrpMetricHelper::adjustParam(const EigrpWideMetricPar& ifParam, const EigrpWideMetricPar& neighParam)
{
    EigrpWideMetricPar newMetricPar;

    newMetricPar.load = getMax(ifParam.load, neighParam.delay);
    newMetricPar.reliability = getMin(ifParam.reliability, neighParam.reliability);
    newMetricPar.mtu = getMin(ifParam.mtu, neighParam.mtu);
    newMetricPar.hopCount = neighParam.hopCount + 1;

    if (isParamMaximal(neighParam)) {
        newMetricPar.delay = DELAY_INF;
        newMetricPar.bandwidth = BANDWIDTH_INF;
    }
    else {
        newMetricPar.delay = ifParam.delay + neighParam.delay;
        newMetricPar.bandwidth = getMin(ifParam.bandwidth, neighParam.bandwidth);
    }

    return newMetricPar;
}

uint64_t EigrpMetricHelper::computeClassicMetric(const EigrpWideMetricPar& par, const EigrpKValues& kValues)
{
    uint32_t metric;
    uint32_t classicDelay = 0, classicBw = 0;

    if (isParamMaximal(par))
        return METRIC_INF;

    // Adjust delay and bandwidth
    if (kValues.K3) { // Note: delay is in pico seconds and must be converted to micro seconds for classic metric
        classicDelay = par.delay / 10000000;
        classicDelay = classicDelay * CLASSIC_SCALE;
    }
    ASSERT(par.bandwidth > 0);

    if (kValues.K1)
        classicBw = BANDWIDTH / par.bandwidth * CLASSIC_SCALE;

    metric = kValues.K1 * classicBw + kValues.K2 * classicBw / (256 - par.load) + kValues.K3 * classicDelay;
    if (kValues.K5 != 0)
        metric = metric * kValues.K5 / (par.reliability + kValues.K4);

    return metric;
}

uint64_t EigrpMetricHelper::computeWideMetric(const EigrpWideMetricPar& par, const EigrpKValues& kValues)
{
    uint64_t metric, throughput, latency;

    if (isParamMaximal(par))
        return METRIC_INF;

    // TODO compute delay from bandwidth if bandwidth is greater than 1 Gb/s
    // TODO include also additional parameters associated with K6

    throughput = (BANDWIDTH * WIDE_SCALE) / par.bandwidth;
    latency = (par.delay * WIDE_SCALE) / DELAY_PICO;

    metric = kValues.K1 * throughput + kValues.K2 * kValues.K1 * throughput / (256 - par.load) + kValues.K3 * latency /*+ kValues.K6*extAttr*/;
    if (kValues.K5 != 0)
        metric = metric * kValues.K5 / (par.reliability + kValues.K4);

    return metric;
}

bool EigrpMetricHelper::compareParameters(const EigrpWideMetricPar& par1, const EigrpWideMetricPar& par2, EigrpKValues& kValues)
{
    if (kValues.K1 && par1.bandwidth != par2.bandwidth)
        return false;
    if (kValues.K2 && par1.load != par2.load)
        return false;
    if (kValues.K3 && par1.delay != par2.delay)
        return false;
    if (kValues.K5 && par1.reliability != par2.reliability)
        return false;

    return true;
}

} // namespace eigrp
} // namespace inet

