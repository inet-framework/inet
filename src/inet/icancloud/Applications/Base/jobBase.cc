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

#include "inet/icancloud/Applications/Base/jobBase.h"

namespace inet {

namespace icancloud {


jobBase::~jobBase() {
    delete(jobResults);
}


 void jobBase::initialize(int stage) {

    API_OS::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        appType = "";
        fsStructures.clear();
        preloadFiles.clear();
        jobResults = new JobResultsSet();
    }
}

 void jobBase::finish(){

     //delete(jobResults);
     jobResults->finishJobResultsSet();

     API_OS::finish();
     fsStructures.clear();
     preloadFiles.clear();
 }

 double jobBase::getTimeToStart(){
     double timeToStart;

     timeToStart = par("timeToProcessUntilStart").doubleValue();

     return timeToStart;
 }


} // namespace icancloud
} // namespace inet
