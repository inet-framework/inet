///
/// @file   SampleGenerator.cc
/// @author Kyeong Soo (Joseph) Kim <kyeongsoo.kim@gmail.com>
/// @date   2010-03-24
///
/// @brief  Implements 'SampleGenerator' class for probability distribution tests.
///
/// @remarks Copyright (C) 2010 Kyeong Soo (Joseph) Kim. All rights reserved.
///
/// @remarks This software is written and distributed under the GNU General
///          Public License Version 2 (http://www.gnu.org/licenses/gpl-2.0.html).
///          You must not remove this notice, or any other, from this software.
///


#include <omnetpp.h>


//namespace fifo {

///
/// Generates samples from a given probability distribution.
///
class SampleGenerator: public cSimpleModule
{
private:
	long n;	// number of samples generated
	long numSamples; // number of samples to generate
	// cStdDev sampleStats; // sample statistics
	// cPSquare sampleStats; // sample statistics
	cPSquare* sampleStats; // sample statistics
	cOutVector sampleVector; // sample vector
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
	n = 0;
	numSamples = par("numSamples").longValue();
    sampleStats = new cPSquare("sample statistic", 100);
	// sampleStats.setName("sample statistics");
	sampleVector.setName("sample vector");

    genEvent = new cMessage("New sample generation event");
    scheduleAt(simTime()+simtime_t(1), genEvent);
}

void SampleGenerator::handleMessage(cMessage *msg)
{
	ASSERT(msg==genEvent);

	double sample = par("distribution").doubleValue();
	sampleStats->collect(sample);
	sampleVector.record(sample);

	EV<< "Generate sample value = " << sample << endl;

	n++;
	if (n < numSamples)
	{
		scheduleAt(simTime()+simtime_t(1), genEvent);
	}
}

void SampleGenerator::finish()
{
	EV << "Total number of samples generated: " << sampleStats->getCount() <<endl;
	EV << "Sample mean: " << sampleStats->getMean() << endl;
	EV << "Sample max.: " << sampleStats->getMax() << endl;
	EV << "Sample std.: " << sampleStats->getStddev() << endl;

	recordScalar("Simulation duration", simTime());
    // 95-percentile
	sampleStats->record();
    delete sampleStats;
}

//}	// end of namespace
