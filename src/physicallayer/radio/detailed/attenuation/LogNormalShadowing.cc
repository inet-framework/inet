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

#include "LogNormalShadowing.h"

#include "Mapping.h"
#include "DetailedRadioFrame.h"

LogNormalShadowing::LogNormalShadowing()
    : mean(0)
    , stdDev(0)
    , interval()
{ }

bool LogNormalShadowing::initFromMap(const ParameterMap& params) {
    ParameterMap::const_iterator it;
    bool                         bInitSuccess = true;

    if ((it = params.find("seed")) != params.end()) {
        srand( ParameterMap::mapped_type(it->second).longValue() );
    }
    if ((it = params.find("mean")) != params.end()) {
        mean = ParameterMap::mapped_type(it->second).doubleValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No mean defined in config.xml for LogNormalShadowing!");
    }
    if ((it = params.find("stdDev")) != params.end()) {
        stdDev = ParameterMap::mapped_type(it->second).doubleValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No stdDev defined in config.xml for LogNormalShadowing!");
    }
    if ((it = params.find("interval")) != params.end()) {
        interval = simtime_t(ParameterMap::mapped_type(it->second).doubleValue());
    }
    else {
        bInitSuccess = false;
        opp_warning("No interval defined in config.xml for LogNormalShadowing!");
    }

    return AnalogueModel::initFromMap(params) && bInitSuccess;
}

LogNormalShadowing::~LogNormalShadowing() {}

double LogNormalShadowing::randomLogNormalGain() const {
	return FWMath::dBm2mW(-1.0 * normal(mean, stdDev));
}

void LogNormalShadowing::filterSignal(DetailedRadioFrame* frame, const Coord& /*sendersPos*/, const Coord& /*receiverPos*/) {
    DetailedRadioSignal&   signal = frame->getSignal();
	simtime_t start  = signal.getReceptionStart();
	simtime_t end    = signal.getReceptionEnd();
	Mapping*  att    = MappingUtils::createMapping(DimensionSet::timeDomain, Mapping::LINEAR);

	Argument pos;

	for(simtime_t t = start; t <= end; t += interval)
	{
		pos.setTime(t);
		att->appendValue(pos, randomLogNormalGain());
	}

	signal.addAttenuation(att);
}
