/**
 *
 * @class VirtualIPs VirtualIPs.cc VirtualIPs.h
 *
 *  Class to represent a virtual ip grouped by 4 groups of numbers.
 *
 * @authors: Gabriel Gonz&aacute;lez Casta&ntilde;&eacute
 * @date 2012-12-11
 */

#ifndef VIRTUALIPS_H_
#define VIRTUALIPS_H_

#include <sstream>
#include <vector>
#include "stdio.h"
#include "string.h"

namespace inet {

namespace icancloud {


using std::string;
using std::pair;
using std::vector;

class VirtualIPs {

protected:

	int first;
	int second;
	int third;
	int fourth;

public:
	VirtualIPs();
	virtual ~VirtualIPs();

	void setVirtualIP(string newVIP);
	string getVirtualIP();

	string incVirtualIP();
	string generateHole(string ipBasis);

	string getNetwork ();
private:
	string virtualIPToString(int ip1,int ip2, int ip3, int ip4);
	vector<int> stringTovirtualIP (string vIP);
	string intToString(int i);
};

} // namespace icancloud
} // namespace inet

#endif /* VIRTUALIPS_H_ */
