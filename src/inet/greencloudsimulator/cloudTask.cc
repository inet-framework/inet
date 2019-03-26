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

#include "cloudTask.h"

namespace inet{
namespace greencloudsimulator {

cloudTask::cloudTask() : id_(0), size_(0), mips_(0), output_(0), intercom_(0), currProcRate_(0.0), executedSince_(0.0)
{

}

cloudTask::cloudTask(unsigned int size, unsigned int mips, double duration, double deadlin)
{
  id_ = 0;
  currProcRate_ = executedSince_ = 0.0;
  output_ = intercom_ = 0;

  size_ = size;
  mips_ = mips;
  deadline_ = deadlin;//Scheduler::instance().clock() + duration;
}

double cloudTask::getExecTime()
{
    return executedSince_;

}
cloudTask::~cloudTask()
{
}

void cloudTask::updateMIPS(double ttime)
{
  double operationsComputed = currProcRate_* (ttime - executedSince_);

  /* update mips_ for the amount executed */
  mips_ -= (unsigned int) operationsComputed;
  executedSince_ = ttime;//3;//Scheduler::instance().clock();
}

void cloudTask::setComputingRate(double rate, double ttime)
{
  /* update what has already been computed */
  updateMIPS(ttime);
  currProcRate_ = rate;
}

double cloudTask::getComputingRate()
{
    return currProcRate_;

}
double cloudTask::execTime()
{
  if (currProcRate_) return ((double)mips_/currProcRate_);
  else return DBL_MAX;
}


}
} /* namespace greencloudsimulator */
