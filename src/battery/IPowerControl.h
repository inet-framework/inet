//
// Copyright (C) 2010 Juan-Carlos Maureira
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

#ifndef IPOWERCONTROL_H_
#define IPOWERCONTROL_H_

#include "PowerControlManager.h"

class IPowerControl {
private:
	bool enable;
public:

	IPowerControl() {
		this->enable = true;
		WATCH(enable);
	}

	virtual bool enableModule( ){
		if (!this->enable) {
			this->enable = true;
			this->enablingInitialization();
			return true;
		}
		return false;
	}
	virtual bool disableModule() {
		if (this->enable) {
			this->enable = false;
			this->disablingInitialization();
			return true;
		}
		return false;
	}

	virtual bool isEnabled() {
		return this->enable;
	}
protected:
	/* Interfaces that modules might implement to perform any task
	 * when enabling or disabling the module */
	virtual void enablingInitialization() {};
	virtual void disablingInitialization() {};
};

#endif /* IPOWERCONTROL_H_ */
