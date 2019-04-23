//
// Module that defines a cell that makes users appear at system.
//
// This module represents 24 hours creating users at system.
//
// @author Gabriel Gonzalez Casta;&ntilde;e
// @date 2012-11-30

#ifndef USERGENERATORDAY_H_
#define USERGENERATORDAY_H_

#include <omnetpp.h>
#include "inet/icancloud/Base/icancloud_Base.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "inet/icancloud/Base/include/icancloud_types.h"
#include "inet/icancloud/Users/UserGenerator/core/AbstractUserGenerator.h"

namespace inet {

namespace icancloud {


class UserGeneratorDay : public AbstractUserGenerator{

	// To define the behavior of the cell
		simtime_t time_intervals_H;
		int quantity_user_preloaded;
		int total_users;
		int repetitions;
		vector<int> users_grouped_by_dist;

		vector<SimTime> subinterval_per_granularity;
		int hour;
		cMessage *newIntervalEvent;
		cMessage *newHourEvent;

protected:

	virtual ~UserGeneratorDay();

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    

   /**
	* Module ending.
	*/
	virtual void finish() override;

   /**
	* Process a self message.
	* @param msg Self message.
	*/
	virtual void processSelfMessage (cMessage *msg) override;;

	virtual void userCreateGroups(int intervals, int nusers) override;

};

} // namespace icancloud
} // namespace inet

#endif /* USERGENERATOR_H_ */


