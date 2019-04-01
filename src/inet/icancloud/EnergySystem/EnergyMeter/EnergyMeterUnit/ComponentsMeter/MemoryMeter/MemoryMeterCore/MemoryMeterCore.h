//
// Module that implements a memory meter core
//
// Memory meter core is responsible for performing energy measurements.
// The core supply operations to get memory energy consumed (Joules) and memory power consumption(Watts)
// 
//
// @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
// @date 2013-11-07
//

#ifndef MEMORYMETERCORE_H_
#define MEMORYMETERCORE_H_

#include "inet/icancloud/EnergySystem/EnergyMeter/EnergyMeterUnit/AbstractMeterUnit.h"
#include "inet/icancloud/Architecture/NodeComponents/Hardware/Memories/MainMemories/RAMmemory/RAMmemory.h"

namespace inet {

namespace icancloud {


class MemoryMeterCore : public AbstractMeterUnit {

    int numModules;                              // Number of memory modules plugged in the main board
    int numDRAMChips;                            // Number of DRAM Chips per module

public:

    // Initialize
    virtual void initialize() override;
    

    // Destructor
	    virtual ~MemoryMeterCore();

    // This method returns the value of the instant power consumed by CPU
    double getInstantConsumption(const string & state, int partIndex) override;


    // This method returns the quantity of jules that
    double getEnergyConsumed(int partIndex) override;

};

} // namespace icancloud
} // namespace inet

#endif /* MEMORYMETERCORE_H_ */




