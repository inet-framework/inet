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

#include "JakesFading.h"

#include "DetailedRadioFrame.h"
#include "DetailedRadioChannelAccess.h"

DimensionSet JakesFadingMapping::dimensions(Dimension::time);

double JakesFadingMapping::getValue(const Argument& pos) const {
	double    f    = model->carrierFrequency;
	double    v    = relSpeed;
	simtime_t t    = pos.getTime();
	double    re_h = 0;
	double    im_h = 0;

	// Compute Doppler shift.
	double doppler_shift = v * f / SPEED_OF_LIGHT;

	for (int i = 0; i < model->fadingPaths; i++) {
		// Some math for complex numbers:
		//
		// Cartesian form: z = a + ib
		// Polar form:     z = p * e^i(phi)
		//
		// a = p * cos(phi)
		// b = p * sin(phi)
		// z1 * z2 = p1 * p2 * e^i(phi1 + phi2)

		// Phase shift due to Doppler => t-selectivity.
		double phi_d = model->angleOfArrival[i] * doppler_shift;
		// Phase shift due to delay spread => f-selectivity.
		double phi_i = SIMTIME_DBL(model->delay[i]) * f;
		// Calculate resulting phase due to t-selective and f-selective fading.
		double phi   = 2.00 * M_PI * (phi_d * SIMTIME_DBL(t) - phi_i);

		// One ring model/Clarke's model plus f-selectivity according to Cavers:
		// Due to isotropic antenna gain pattern on all paths only a^2 can be received on all paths.
		// Since we are interested in attenuation a:=1, attenuation per path is then:
		double attenuation = (1.00 / sqrt(static_cast<double>(model->fadingPaths)));

		// Convert to cartesian form and aggregate {Re, Im} over all fading paths.
		re_h = re_h + attenuation * cos(phi);
		im_h = im_h - attenuation * sin(phi);
	}

	// Output: |H_f|^2 = absolute channel impulse response due to fading.
	// Note that this may be >1 due to constructive interference.
	return re_h * re_h + im_h * im_h;
}


JakesFading::JakesFading()
	: AnalogueModel()
	, fadingPaths(0)
	, angleOfArrival(NULL)
	, delay(NULL)
	, carrierFrequency(0)
	, interval()
{
}

bool JakesFading::initFromMap(const ParameterMap& params) {
    ParameterMap::const_iterator it;
    bool                         bInitSuccess = true;
    double                       delayRMS     = 0.0;

    if ((it = params.find("seed")) != params.end()) {
        srand( ParameterMap::mapped_type(it->second).longValue() );
    }
    if ((it = params.find("fadingPaths")) != params.end()) {
        fadingPaths = ParameterMap::mapped_type(it->second).longValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No fadingPaths defined in config.xml for JakesFading!");
    }
    if ((it = params.find("delayRMS")) != params.end()) {
        delayRMS = ParameterMap::mapped_type(it->second).doubleValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No delayRMS defined in config.xml for JakesFading!");
    }
    if ((it = params.find("interval")) != params.end()) {
        interval = Argument(ParameterMap::mapped_type(it->second).doubleValue());
    }
    else {
        bInitSuccess = false;
        opp_warning("No interval defined in config.xml for JakesFading!");
    }
    if ((it = params.find("carrierFrequency")) != params.end()) {
        carrierFrequency = ParameterMap::mapped_type(it->second).doubleValue();
    }
    else {
        bInitSuccess = false;
        opp_warning("No carrierFrequency defined in config.xml for JakesFading!");
    }
    if (bInitSuccess) {
	angleOfArrival = new double[fadingPaths];
	delay = new simtime_t[fadingPaths];

	for (int i = 0; i < fadingPaths; ++i) {
		angleOfArrival[i] = cos(uniform(0, M_PI));
		delay[i] = exponential(delayRMS);
	}
    }
    return AnalogueModel::initFromMap(params) && bInitSuccess;
}

JakesFading::~JakesFading() {
    if (delay != NULL)
	delete[] delay;
    if (angleOfArrival != NULL)
	delete[] angleOfArrival;
}

void JakesFading::filterSignal(DetailedRadioFrame* frame, const Coord& /*sendersPos*/, const Coord& /*receiverPos*/)
{
	DetailedRadioSignal&                signal           = frame->getSignal();
	IMobility * senderMobility   = dynamic_cast<DetailedRadioChannelAccess *>(frame->getSenderModule())->getMobility();
	IMobility * receiverMobility = dynamic_cast<DetailedRadioChannelAccess *>(frame->getArrivalModule())->getMobility();
	const double           relSpeed         = (senderMobility->getCurrentSpeed() - receiverMobility->getCurrentSpeed()).length();

	signal.addAttenuation(new JakesFadingMapping(this, relSpeed,
	                                             Argument(signal.getReceptionStart()),
	                                             interval,
	                                             Argument(signal.getReceptionEnd())));
}
