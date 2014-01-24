#ifndef TESTPHYLAYER_H_
#define TESTPHYLAYER_H_

#include <BasePhyLayer.h>
#include <TestModule.h>
#include "TestDecider.h"

#include <list>

class TestPhyLayer:public BasePhyLayer, public TestModule {
private:
	/** @brief Copy constructor is not allowed.
	 */
	TestPhyLayer(const TestPhyLayer&);
	/** @brief Assignment operator is not allowed.
	 */
	TestPhyLayer& operator=(const TestPhyLayer&);

private:

	class TestAnalogueModel:public AnalogueModel {
	public:
		double att;

		TestAnalogueModel() : att(0) {}

		/** @brief Initialize the analog model from XML map data.
		 *
		 * This method should be defined for generic analog model initialization.
		 *
		 * @param params The parameter map which was filled by XML reader.
		 *
		 * @return true if the initialization was successfully.
		 */
		virtual bool initFromMap(const ParameterMap& params) {
			ParameterMap::const_iterator it;
			bool                         bInitSuccess = true;

			if ((it = params.find("seed")) != params.end()) {
				srand( ParameterMap::mapped_type(it->second).longValue() );
			}
			if ((it = params.find("attenuation")) != params.end()) {
				att = ParameterMap::mapped_type(it->second).doubleValue();
			}
			else {
				bInitSuccess = false;
				opp_warning("No attenuation defined in config.xml for TestAnalogueModel!");
			}

			return AnalogueModel::initFromMap(params) && bInitSuccess;
		}

		void filterSignal(DetailedRadioFrame *, const Coord&, const Coord&) {
			return;
		}
	};
protected:

	int myIndex;
	int protocolID;

	// prepared RSSI mapping for testing purposes
	Mapping* testRSSIMap;

	/**
	 * @brief Creates and returns an instance of the AnalogueModel with the
	 *        specified name.
	 *
	 * Is able to initialize the following AnalogueModels:
	 * - TestAnalogueModel
	 */
	virtual AnalogueModel* getAnalogueModelFromName(const std::string& name, ParameterMap& params) const;

	/**
	 * @brief Creates and returns an instance of the decider with the specified
	 *        name.
	 *
	 * Is able to initialize directly the following decider:
	 * - TestDecider
	 */
	virtual Decider* getDeciderFromName(const std::string& name, ParameterMap& params);

	virtual bool isKnownProtocolId(int id) const;
	virtual int myProtocolId() const;

public:
	TestPhyLayer()
		: BasePhyLayer()
		, TestModule()
		, myIndex(0)
		, protocolID(0)
		, testRSSIMap(NULL)
	{}

	virtual void initialize(int stage);

	virtual void handleMessage(cMessage* msg);

	virtual ~TestPhyLayer();

	void testInitialisation();

};

#endif /*TESTPHYLAYER_H_*/
