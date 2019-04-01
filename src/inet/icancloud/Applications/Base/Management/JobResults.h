/**
 * @class JobResults JobResults.cc JobResults.h
 *
 * This class is responsible for the model of a user into the icancloud.
 * The functionality is to allocate one result of the execution of a job.
 * Any work produces a result perfectly identificated with the attributes name and value.
 *
 * @author Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2011-15-08
 */

#ifndef JOBRESULTS_H_
#define JOBRESULTS_H_
#include <sstream>
#include <vector>
#include <string>

namespace inet {

namespace icancloud {


using std::string;
using std::pair;
using std::vector;

class JobResults {

	private:

		/** the name of the result **/
		string name;

		/** The values of the result **/
		vector<string> value;

	public:

		/*
		 * Constructor.
		 */
		JobResults();

		/*
		 * Destructor.
		 */
		virtual ~JobResults();

		/*
		 * This method returns the attribute name.
		 */
		string getName();

		/*
		 * This method set the attribute name with the value of newName.
		 * @param: the new name to assign.
		 */
		void setName(string newName);

        /*
         * This method returns the quantity of values allocated in this class
         */
		int getValuesSize();

        /*
         * This method returns the attribute value.
         */
        string getValue(int index);

        /*
         * This method set the attribute name with the value of newValue.
         * @param: the new value to assign.
         */
        void setValue(string newValue);

        /*
         * This method returns a replica of job results.
         */
        JobResults* dup();

		/*
		 *  Method that return a string with the result to print it.
		 */
		string toString();
};

} // namespace icancloud
} // namespace inet

#endif /* JOBRESULTS_H_ */
