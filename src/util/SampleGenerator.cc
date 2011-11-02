///
/// @file   SampleGenerator.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2010-03-24
///
/// @brief  Implements 'SampleGenerator' class for probability distribution tests.
///
/// @remarks Copyright (C) 2010-2011 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#include <omnetpp.h>

class SampleGenerator: public cSimpleModule
{
private:
    simsignal_t sampleSignal;

	long n;	// number of samples generated
	long numSamples; // number of samples to generate
	cMessage *genEvent;

public:
	SampleGenerator();
	virtual ~SampleGenerator();

protected:
	virtual void initialize();
	virtual void handleMessage(cMessage *msg);
	virtual void finish();
};

Define_Module(SampleGenerator);

SampleGenerator::SampleGenerator()
{
	genEvent = NULL;
}

SampleGenerator::~SampleGenerator()
{
	cancelAndDelete(genEvent);
}

void SampleGenerator::initialize()
{
    sampleSignal = registerSignal("sample");
	n = 0;
	numSamples = par("numSamples").longValue();

    genEvent = new cMessage("New sample generation event");
    scheduleAt(simTime()+simtime_t(1), genEvent);
}

void SampleGenerator::handleMessage(cMessage *msg)
{
	ASSERT(msg==genEvent);

    if (n < numSamples) {
        double sample = par("distribution").doubleValue();
        emit(sampleSignal, sample);
        
        EV<< "Generate sample value = " << sample << endl;
        
        n++;
		scheduleAt(simTime()+simtime_t(1), genEvent);
    }
}

void SampleGenerator::finish()
{
	recordScalar("Simulation duration", simTime());
}
