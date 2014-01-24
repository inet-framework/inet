#include "TestPhyLayer.h"

#include "../testUtils/asserts.h"

Define_Module(TestPhyLayer);

void TestPhyLayer::initialize(int stage) {
	if(stage == 0)
	{
		myIndex = findContainingNode(this)->getIndex();
		protocolID = par("protocol").longValue();
	}

	//call BasePhy's initialize
	BasePhyLayer::initialize(stage);

	//run basic tests
	if(stage == 0) {
		init("phy" + toString(myIndex));
	}
}

bool TestPhyLayer::isKnownProtocolId(int id) const {
	return id == protocolID;
}

int TestPhyLayer::myProtocolId() const {
	return protocolID;
}

void TestPhyLayer::handleMessage(cMessage* msg) {
	announceMessage(msg);

	BasePhyLayer::handleMessage(msg);
}

TestPhyLayer::~TestPhyLayer() {
	finalize();
}

void TestPhyLayer::testInitialisation() {
	Enter_Method_Silent();

	//run dependend tests
	assertFalse("Check parameter \"usePropagationDelay\".", usePropagationDelay);

	assertEqual("Check parameter \"sensitivity\".", FWMath::dBm2mW(6), sensitivity);
	assertEqual("Check parameter \"maxTXPower\".", 10.0, maxTXPower);


	ConstantSimpleConstMapping* thNoise = dynamic_cast<ConstantSimpleConstMapping*>(thermalNoise);
	assertNotEqual("Check if thermalNoise map is of type ConstantSimpleConstMapping.", (void*)0, thNoise);
	assertEqual("Check parameter \"thermalNoise\".", FWMath::dBm2mW(1.0), thNoise->getValue());

	thNoise = dynamic_cast<ConstantSimpleConstMapping*>(getThermalNoise(1.0, 2.0));
	assertNotEqual("Check if thermalNoise map returned by \"getThermalNoise()\" is of type ConstantSimpleConstMapping.", (void*)0, thNoise);
	assertEqual("Check value of (\"getThermalNoise()\"-mapping).", FWMath::dBm2mW(1.0), thNoise->getValue());
	assertEqual("Check value of (\"getThermalNoise()\"-mapping at a position).", FWMath::dBm2mW(1.0), thNoise->getValue(Argument(1.5)));

	assertTrue("Check upperLayerIn ID.", upperLayerIn != NULL);
	assertTrue("Check upperLayerOut ID.", upperLayerOut != NULL);

	//test radio state switching times
	radio->switchTo(IRadio::RADIO_MODE_SLEEP, simTime());
	radio->endSwitch(simTime());
	simtime_t swTime = radio->switchTo(IRadio::RADIO_MODE_RECEIVER, simTime());
	assertEqual("Switchtime SLEEP to RX.", 3.0, SIMTIME_DBL(swTime));
	radio->endSwitch(simTime());

	swTime = radio->switchTo(IRadio::RADIO_MODE_TRANSMITTER, simTime());
	assertEqual("Switchtime RX to TX.", 1.0, SIMTIME_DBL(swTime));
	radio->endSwitch(simTime());

	swTime = radio->switchTo(IRadio::RADIO_MODE_SLEEP, simTime());
	assertEqual("Switchtime TX to SLEEP.", 2.5, SIMTIME_DBL(swTime));
	radio->endSwitch(simTime());

	swTime = radio->switchTo(IRadio::RADIO_MODE_TRANSMITTER, simTime());
	assertEqual("Switchtime SLEEP to TX.", 3.5, SIMTIME_DBL(swTime));
	radio->endSwitch(simTime());

	swTime = radio->switchTo(IRadio::RADIO_MODE_RECEIVER, simTime());
	assertEqual("Switchtime TX to RX.", 2.0, SIMTIME_DBL(swTime));
	radio->endSwitch(simTime());

	swTime = radio->switchTo(IRadio::RADIO_MODE_SLEEP, simTime());
	assertEqual("Switchtime RX to SLEEP.", 1.5, SIMTIME_DBL(swTime));
	radio->endSwitch(simTime());

	TestDecider* dec = dynamic_cast<TestDecider*>(decider);
	assertTrue("Decider is of type TestDecider.", dec != 0);

	assertEqual("Check size of AnalogueModel list.", (size_t)3, analogueModels.size());
	double att1 = -1.0;
	double att2 = -1.0;

	// handling first Analogue Model (RSAM)
	AnalogueModelList::const_iterator it = analogueModels.begin();
	RadioStateAnalogueModel* model = dynamic_cast<RadioStateAnalogueModel*>(*it);
	assertTrue("Analogue model is of type RadioStateAnalogueModel.", model != 0);
	model = 0;
	it++;

	// handling all other analogue models
	for(; it != analogueModels.end(); it++) {

		TestAnalogueModel* model = dynamic_cast<TestAnalogueModel*>(*it);
		assertTrue("Analogue model is of type TestAnalogueModel.", model != NULL);
		if(att1 < 0.0)
			att1 = model->att;
		else
			att2 = model->att;
	}

	assertTrue("Check attenuation value of AnalogueModels.",
				(FWMath::close(att1, 1.1) && FWMath::close(att2, 2.1))
				|| (FWMath::close(att1, 2.1) && FWMath::close(att2, 1.1)));

	//check initialisation of timers
	assertNotEqual("Check initialisation of TX-OVER timer", (void*)0, txOverTimer);
	assertEqual("Check kind of TX_OVER timer", TX_OVER, txOverTimer->getKind());

	assertNotEqual("Check initialisation of radioSwitchOver timer", (void*)0, radioSwitchingOverTimer);
	assertEqual("Check kind of radioSwitchOver timer", RADIO_SWITCHING_OVER, radioSwitchingOverTimer->getKind());

	testPassed("0");
}


AnalogueModel* TestPhyLayer::getAnalogueModelFromName(const std::string& name, ParameterMap& params) const {

	AnalogueModel* model = BasePhyLayer::getAnalogueModelFromName(name, params);

	if (model != NULL)
	{
		assertEqual("Check AnalogueModel name.", std::string("RadioStateAnalogueModel"), name);
		assertEqual("Check for correct RSAM-pointer.", radio->getAnalogueModel(), model);
		assertEqual("Check AnalogueModel parameter count.", (unsigned int)0, params.size());

		return model;
	}

	assertEqual("Check AnalogueModel name.", std::string("TestAnalogueModel"), name);

	assertEqual("Check AnalogueModel parameter count.", (unsigned int)1, params.size());

	assertEqual("Check for parameter \"attenuation\".", (unsigned int)1, params.count("attenuation"));
	ParameterMap::mapped_type par = params["attenuation"];
	assertEqual("Check type of parameter \"attenuation\".", 'D', par.getType());

	return createAnalogueModel<TestAnalogueModel>(params);
}

Decider* TestPhyLayer::getDeciderFromName(const std::string& name, ParameterMap& params) {

	assertEqual("Check Decider name.", std::string("TestDecider"), name);

	assertEqual("Check Decider parameter count.", (unsigned int)8, params.size());

	assertEqual("Check for parameter \"aString\".", (unsigned int)1, params.count("aString"));
	ParameterMap::mapped_type par = params["aString"];
	assertEqual("Check type of parameter \"aString\".", 'S', par.getType());
	assertEqual("Check value of parameter \"aString\".", std::string("teststring"), std::string(par.stringValue()));

	assertEqual("Check for parameter \"anotherString\".", (unsigned int)1, params.count("anotherString"));
	par = params["anotherString"];
	assertEqual("Check type of parameter \"anotherString\".", 'S', par.getType());
	assertEqual("Check value of parameter \"anotherString\".", std::string("testString2"), std::string(par.stringValue()));

	assertEqual("Check for parameter \"aBool\".", (unsigned int)1, params.count("aBool"));
	par = params["aBool"];
	assertEqual("Check type of parameter \"aBool\".", 'B', par.getType());
	assertEqual("Check value of parameter \"aBool\".", true, par.boolValue());

	assertEqual("Check for parameter \"anotherBool\".", (unsigned int)1, params.count("anotherBool"));
	par = params["anotherBool"];
	assertEqual("Check type of parameter \"anotherBool\".", 'B', par.getType());
	assertEqual("Check value of parameter \"anotherBool\".", false, par.boolValue());

	assertEqual("Check for parameter \"aDouble\".", (unsigned int)1, params.count("aDouble"));
	par = params["aDouble"];
	assertEqual("Check type of parameter \"aDouble\".", 'D', par.getType());
	assertEqual("Check value of parameter \"aDouble\".", 0.25, par.doubleValue());

	assertEqual("Check for parameter \"anotherDouble\".", (unsigned int)1, params.count("anotherDouble"));
	par = params["anotherDouble"];
	assertEqual("Check type of parameter \"anotherDouble\".", 'D', par.getType());
	assertEqual("Check value of parameter \"anotherDouble\".", -13.52, par.doubleValue());

	assertEqual("Check for parameter \"aLong\".", (unsigned int)1, params.count("aLong"));
	par = params["aLong"];
	assertEqual("Check type of parameter \"aLong\".", 'L', par.getType());
	assertEqual("Check value of parameter \"aLong\".", 234567, par.longValue());

	assertEqual("Check for parameter \"anotherLong\".", (unsigned int)1, params.count("anotherLong"));
	par = params["anotherLong"];
	assertEqual("Check type of parameter \"anotherLong\".", 'L', par.getType());
	assertEqual("Check value of parameter \"anotherLong\".", -34567, par.longValue());

	int runNumber = simulation.getSystemModule()->par("run");

	std::string sParamName = "recvState";
	params[sParamName.c_str()] = ParameterMap::mapped_type(sParamName.c_str()).setLongValue(RECEIVING);
	sParamName = "runNumber";
	params[sParamName.c_str()] = ParameterMap::mapped_type(sParamName.c_str()).setLongValue(runNumber);

	return createDecider<TestDecider>(params);
}
