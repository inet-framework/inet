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

#include "inet/icancloud/Applications/Base/Management/JobResultsSet.h"

namespace inet {

namespace icancloud {


JobResultsSet::JobResultsSet() {
    jobResultsSet.clear();
    jobID = "";
    job_startTime = SimTime();
    job_endTime = SimTime();
}

JobResultsSet::~JobResultsSet() {
    jobResultsSet.clear();
    jobID = "";
}

void JobResultsSet::finishJobResultsSet(){
    jobResultsSet.clear();
    jobID = "";
}

int JobResultsSet::getJobResultSize (){

    return jobResultsSet.size();

}

JobResults* JobResultsSet::getJobResultSet (string value){

	//vector<JobResults*>::iterator it;
	bool found = false;
	JobResults* jobRes = nullptr;

	for (unsigned int i = 0; (i < jobResultsSet.size()) && (!found); i++){

	    if (strcmp ((*(jobResultsSet.begin() + i))->getName().c_str(), value.c_str()) == 0){
	        found = true;
	        jobRes = (*(jobResultsSet.begin() + i));
	    }
	}

	return jobRes;
}

void JobResultsSet::newJobResultSet(string jobResult){
    JobResults* jobRes;

    // check if the value exists
    for (unsigned int i = 0; (i < jobResultsSet.size()); i++){
        jobRes = (*(jobResultsSet.begin() + i));
        if (strcmp (jobRes->getName().c_str(), jobResult.c_str()) == 0){
            throw cRuntimeError("Error: JobResultsSet::newJobResultSet->%s. The value %s exists previously.", jobResult.c_str());
        }
    }

    jobRes = new JobResults();
    jobRes->setName(jobResult);
    jobResultsSet.push_back(jobRes);
}

void JobResultsSet::setJobResult (string jobResultsName, int value){
    std::ostringstream info;
    info << value;
    setJobResult_p (jobResultsName, info.str());
}

void JobResultsSet::setJobResult (string jobResultsName, double value){
    std::ostringstream info;
    info << value;
    setJobResult_p (jobResultsName, info.str());
}

void JobResultsSet::setJobResult (string jobResultsName, simtime_t value){
    std::ostringstream info;
    info << value.dbl();
    setJobResult_p (jobResultsName, info.str());
}

void JobResultsSet::setJobResult (string jobResultsName, bool value){
    if (value)
        setJobResult_p (jobResultsName, "true");
    else
        setJobResult_p (jobResultsName, "false");

}

void JobResultsSet::setJobResult (string jobResultsName, string value){
    setJobResult_p (jobResultsName, value.c_str());
}

void JobResultsSet::setJobResult (int indexJobResult, int value){
    std::ostringstream info;
    info << value;
    setJobResult_p (indexJobResult, info.str());
}

void JobResultsSet::setJobResult (int indexJobResult, double value){
    std::ostringstream info;
    info << value;
    setJobResult_p (indexJobResult, info.str());
}

void JobResultsSet::setJobResult (int indexJobResult, simtime_t value){
    std::ostringstream info;
    info << value.dbl();
    setJobResult_p (indexJobResult, info.str());
}

void JobResultsSet::setJobResult (int indexJobResult, bool value){
    if (value)
        setJobResult_p (indexJobResult, "true");
    else
        setJobResult_p (indexJobResult, "false");
}

void JobResultsSet::setJobResult (int indexJobResult, string value){
    setJobResult_p (indexJobResult, value.c_str());
}

JobResultsSet* JobResultsSet::dup (){

    JobResultsSet* set;
    JobResults* jobResults;
    set = new JobResultsSet();

    for (int i = 0; i < (int) jobResultsSet.size(); i++){

        jobResults = getJobResultSet(i)->dup();
        set->newJobResultSet(jobResults->getName());
        for (int j = 0; j < jobResults->getValuesSize(); j++){
            set->setJobResult(i,jobResults->getValue(j));
        }
    }

    set->setJobID(getJobID());
    set->setMachineType(getMachineType());
    set->setJob_startTime(getJob_startTime());
    set->setJob_endTime(getJob_endTime());

    return set;
}

void JobResultsSet::setJob_startTime(simtime_t simtime){
    job_startTime = simtime;
}

void JobResultsSet::setJob_endTime(simtime_t simtime){
    job_endTime = simtime;
}

string JobResultsSet::toString(){

	// Define..
	std::ostringstream info;
	vector <JobResults*>::iterator it;

	if (jobResultsSet.size() == 0)
		info << "  There are no results yet!";
	else
		info << "\n\t\t\t";

	for (it = jobResultsSet.begin(); it != jobResultsSet.end() ; it++){

		info << "{" << (*it)->toString() << "} - " ;
	}

	return info.str();
}

void JobResultsSet::setJobResult_p (string jobResultsName, string value){
        //vector<JobResults*>::iterator it;
        bool found = false;
        JobResults* jobRes = nullptr;

        for (int i = 0; (i < (int)jobResultsSet.size()) && (!found); i++){

            if (strcmp ((*(jobResultsSet.begin()) + i)->getName().c_str(), jobResultsName.c_str()) == 0){
                found = true;
                (*(jobResultsSet.begin() + i))->setValue(value);
            }
        }

        if (!found){
            jobRes = new JobResults();
            jobRes->setName(jobResultsName.c_str());
        }
}

void JobResultsSet::setJobResult_p (int indexJobResult, string value){
    (*(jobResultsSet.begin() + indexJobResult))->setValue(value);
}

} // namespace icancloud
} // namespace inet
