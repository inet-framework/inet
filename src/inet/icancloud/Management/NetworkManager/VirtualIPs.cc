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

#include "inet/icancloud/Management/NetworkManager/VirtualIPs.h"

namespace inet {

namespace icancloud {


VirtualIPs::VirtualIPs() {
	// TODO Auto-generated constructor stub
	first = -1;
	second = -1;
	third = -1;
	fourth = -1;
}

VirtualIPs::~VirtualIPs() {
	// TODO Auto-generated destructor stub
}


void VirtualIPs::setVirtualIP(string newVIP){
	vector<int> ip;
	ip = stringTovirtualIP(newVIP);

	first = *(ip.begin());
	second = *(ip.begin()+1);
	third = *(ip.begin()+2);
	fourth = *(ip.begin()+3);

}

string VirtualIPs::getVirtualIP(){
	return virtualIPToString(first, second, third, fourth);
}

string VirtualIPs::incVirtualIP(){

	string vip;
	bool error = false;

	if (fourth < 256){
		fourth++;
	} else {

		if (third < 256){
			third++;
			fourth = 0;
		} else {

			if (second < 256){
				second ++;
				third = 0;
				fourth = 0;
			} else {
				if (first < 256){
					first ++;
					second = 0;
					third = 0;
					fourth = 0;

				} else {
					// ERROR
					vip = "";
					error = true;
				}
			}
		}
	}

	if (!error) vip = virtualIPToString(first, second, third, fourth);
	return vip;
}

string VirtualIPs::generateHole(string ipBasis){
	vector<int> ipBasisT;
	int hole1,hole2,hole3,hole4;

	ipBasisT.clear();

	ipBasisT = stringTovirtualIP(ipBasis);

	hole1 = first - (*(ipBasisT.begin()));
	hole2 = second - (*(ipBasisT.begin()+1));
	hole3 = third - (*(ipBasisT.begin()+2));
	hole4 = fourth - (*(ipBasisT.begin()+3));

	return virtualIPToString(hole1, hole2, hole3, hole4);
}

string VirtualIPs::getNetwork (){
	string res;

	res = virtualIPToString (first,second,0,0);

	return res;
}

string VirtualIPs::virtualIPToString(int ip1,int ip2, int ip3, int ip4){
	std::stringstream ip;

	ip << ip1 << ".";
	ip << ip2 << ".";
	ip << ip3 << ".";
	ip << ip4;

	return ip.str().c_str();

}

vector<int> VirtualIPs::stringTovirtualIP (string vIP){

	char ipStr [16];
	int ip1,ip2,ip3,ip4;
	vector<int> vectorIP;
	//vector<int>::iterator it;

	memset (ipStr, 0, 16);
	strcpy (ipStr, vIP.c_str());
	sscanf (ipStr, "%i.%i.%i.%i", &ip1,&ip2,&ip3,&ip4);

	vectorIP.clear();
	vectorIP.insert(vectorIP.begin(),ip1);
	vectorIP.insert(vectorIP.begin()+1,ip2);
	vectorIP.insert(vectorIP.begin()+2,ip3);
	vectorIP.insert(vectorIP.begin()+3,ip4);

	return vectorIP;
}

string VirtualIPs::intToString(int i)
{
  std::ostringstream stream;
  stream << i << std::flush;
  std::string str(stream.str());
  return str;
}

} // namespace icancloud
} // namespace inet
