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

#include "UserGeneratorDay.h"

namespace inet {

namespace icancloud {


Define_Module(UserGeneratorDay);

UserGeneratorDay::~UserGeneratorDay() {

    cancelAndDelete (newHourEvent);
    cancelAndDelete (newIntervalEvent);
}

void UserGeneratorDay::initialize(int stage) {

    // Init the superclass
    AbstractUserGenerator::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {

        // Get the ned parameters
        time_intervals_H = par("time_creation");
        quantity_user_preloaded = par("quantity_user_preloaded").intValue();
        total_users = par("total_users").intValue();
        repetitions = par("repetitions").intValue();

        // Initialize ..
        newIntervalEvent = new cMessage("intervalEvent");
        newHourEvent = new cMessage("hourEvent");
        hour = 0;

        if ((total_users == 0) && (quantity_user_preloaded == 0)) {
            showErrorMessage(
                    "There are no users at userGenerator day. The users preloaded and the total users are 0");
        }

        cMessage *waitToExecuteMsg = new cMessage(SM_WAIT_TO_EXECUTE.c_str());
        scheduleAt(simTime(), waitToExecuteMsg);
    }

}

void UserGeneratorDay::finish(){

	AbstractUserGenerator::finish();
}

void UserGeneratorDay::processSelfMessage(cMessage *msg) {

    SimTime nextEvent;

    unsigned int i, j;
    double k;
    bool finalization = false;
    string distribution = distributionName;
    int interval = static_cast<int>(time_intervals_H.dbl());

    if (!strcmp(msg->getName(), SM_WAIT_TO_EXECUTE.c_str())) {

        delete (msg);

        // create the preloaded users
        if (quantity_user_preloaded != 0)
            for (i = 0; (int) i < quantity_user_preloaded; i++)
                createUser();

        if (strcmp(distribution.c_str(), "no_distribution") == 0) {

            for (i = 0; (int) i < total_users; i++)
                createUser();

        }
        else {

            // Prepare to user arrival .. !!
            userCreateGroups(interval, total_users);
        }

        if (time_intervals_H != 0)
            scheduleAt(simTime(), newHourEvent);
        else
            finalizeUserGenerator(false);

    }
    else if (!strcmp(msg->getName(), "intervalEvent")) {

        cancelEvent(newIntervalEvent);

        // create the user for this interval..
        createUser();

        if (subinterval_per_granularity.size() != 0) {
            // Prepare the nextEvent ..
            nextEvent = (*(subinterval_per_granularity.begin()));
            subinterval_per_granularity.erase(
                    subinterval_per_granularity.begin());

            scheduleAt(nextEvent, newIntervalEvent);
        }

    }
    else if (!strcmp(msg->getName(), "hourEvent")) {

        // if hour == time intervals, the time limit for user's creation has been reached.

        if (hour == interval) {

            // If repetitions != 0, repeat the process
            if (repetitions != 0) {
                repetitions--;
                hour = 0;

                // If there are no distribution, when the process has to be repeated, it is the moment for creating the
                // new subset of users for this repetition cycle
                if (strcmp(distribution.c_str(), "no_distribution") == 0) {
                    for (i = 0; (int) i < total_users; i++)
                        createUser();
                }
            }
            // If repetitions == 0, finalize the user generation
            else {
                // Finalization of userGeneratorDay
                AbstractUserGenerator::finalizeUserGenerator(false);
                finalization = true;
            }
        }

        cancelEvent(newHourEvent);

        if (!finalization) {
            // If the users appear at the cloud system following a statistical distribution
            if (strcmp(distribution.c_str(), "no_distribution") != 0) {

                subinterval_per_granularity.clear();

                // j is the number of users to be created
                j = (*(users_grouped_by_dist.begin() + hour));

                // there are not users at this interval
                if (j == 0) {

                    scheduleAt(simTime() + 3600, newHourEvent);

                    // There are only one user. So only it is needed to create a user
                }
                else if (j == 1) {
                    scheduleAt(simTime() + 3600, newHourEvent);
                    scheduleAt(simTime(), newIntervalEvent);

                    // There are a group of users to distribute along the subintervals created by granularity..
                } else {
                    // k = 3600 sec per hour / j is the instant per create the user
                    k = 3600 / j;

                    // Creates the array per each next instant to create a user
                    nextEvent = simTime();

                    for (i = 0; i < j - 1; i++) {
                        nextEvent += k;
                        subinterval_per_granularity.push_back(nextEvent);
                    }

                    scheduleAt(simTime() + 3600, newHourEvent);
                    scheduleAt(simTime(), newIntervalEvent);
                }

                hour++;

                // Schedule the next event, for no distribution when all the time intervals (in hours 60 * 60 = 3600) has passed.
            }
            else {
                scheduleAt(simTime() + (3600 * time_intervals_H.dbl()),
                        newHourEvent);
            }
        }
        else {
            cancelEvent(newIntervalEvent);
        }

    } else {

        showErrorMessage(
                "Error in UserGenerator_cell. Error in name of message: %s ",
                msg->getName());

    }
}

void UserGeneratorDay::userCreateGroups(int intervals, int nusers){

    double number;
    vector<long int> p;

    const long int nExperiments=nusers * 100;  // number of experiments

       for (long int i =0; i < intervals; i ++) p.push_back(0);

       for (long int i=0; i<nExperiments; ++i) {

           number = selectDistribution();

         if ((number>=0.0)&&(number<intervals)){
             (*(p.begin() + ((long int)(round(number)))))++;
         }
       }

       for (long int i=0; i<intervals; ++i) {
           users_grouped_by_dist.push_back((long int)(((*(p.begin() + i))*nusers/nExperiments)));
       }

       if (true){

           for (int i=0; i<intervals; ++i) {
               printf("%i - %i\n: [%i]",i, i+1, (*(users_grouped_by_dist.begin() + i)) );
           }
       }
}


} // namespace icancloud
} // namespace inet
