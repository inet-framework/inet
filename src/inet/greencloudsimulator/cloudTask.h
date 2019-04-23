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

#ifndef CLOUDTASK_H_
#define CLOUDTASK_H_

#include "inet/common/INETDefs.h"
#include "inet/greencloudsimulator/CloudTask_m.h"

namespace inet {
namespace greencloudsimulator {

//#define DBL_MAX (9.999999999999999e999)

class cloudTask : public cMessage
{
public:
    cloudTask();
    cloudTask(unsigned int size, unsigned int mips, double duration, double deadlin);
    virtual ~cloudTask();
    void updateMIPS(double ttime);
    void setComputingRate(double rate, double ttime);
    double execTime();

    int getID() {return id_;};
    unsigned int getSize() {return size_;};
    unsigned int getMIPS() {return mips_;};
    double getDeadline() {return deadline_;};
    int getOutput() {return output_;};
    int getIntercom() { return intercom_;};
    double getComputingRate();

    void setSize(unsigned int size) {size_ = size;};
    void setMIPS(unsigned int mips) {mips_ = mips;};
    void setExecTime(double execTime) {executedSince_ = execTime;};
    void setID(int id) {id_ = id;};
    void setDeadline(double deadline) {deadline_ = deadline;};
    void setOutput(int output) {output_ = output;};
    void setIntercom(int intercom) {intercom_ = intercom;};
    double getExecTime();
    int id_;

protected:
    //void handler(Event *);

    unsigned int size_; /* amount of bytes transferred to servers for task execution */
    unsigned int mips_; /* computational requirement of the task */
    double deadline_;   /* task deadline */

    int output_;    /* amount of data in bytes sent out of the data center upon task completion */
    int intercom_;  /* amoutn of data in bytes to be transferred to another data center application */

    double currProcRate_;   /* current processing rate of the task (determinded by the server) */
    double executedSince_;  /* last time instance of task execution */
};

}
} /* namespace greencloudsimulator */
#endif /* CLOUDTASK_H_ */
