//
// Copyright (C) 2008 Juan-Carlos Maureira
// Copyright (C) INRIA
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __DHCPOPTIONS__
#define __DHCPOPTIONS__

#include <map>
#include <sstream>
#include "Byte.h"

enum op_code
{
    DHCP_MSG_TYPE = 53,
    CLIENT_ID = 61,
    HOSTNAME = 12,
    REQUESTED_IP = 50,
    PARAM_LIST = 55,
    SUBNET_MASK = 1,
    ROUTER = 3,
    DNS = 6,
    NTP_SRV = 42,
    RENEWAL_TIME = 58,
    REBIND_TIME = 59,
    LEASE_TIME = 51,
    SERVER_ID = 54,
};

class DHCPOption
{
    public:
        typedef std::map<op_code, Byte> DHCPOptionsMap;
    private:
        DHCPOptionsMap options;

    public:
        void set(op_code code, std::string data)
        {
            options[code] = Byte(data);
        }

        void set(op_code code, int data)
        {
            options[code] = Byte(data);
        }

        void add(op_code code, int data)
        {
            if (options.find(code) == options.end())
            {
                options[code] = Byte(data);
            }
            else
            {
                options[code].concat(Byte(data));
            }
        }

        Byte get(op_code code)
        {
            DHCPOptionsMap::iterator it = options.find(code);
            if (it != options.end())
            {
                return it->second;
            }
            return 0;
        }

        DHCPOptionsMap::iterator begin()
        {
            return (options.begin());
        }

        DHCPOptionsMap::iterator end()
        {
            return (options.end());
        }

        friend std::ostream& operator <<(std::ostream& os, DHCPOption& obj)
        {
            for (DHCPOptionsMap::iterator it = obj.begin(); it != obj.end(); it++)
            {
                os << "        " << it->first << " = " << it->second << endl;
            }
            return os;
        }
};
#endif
