#include "asserts.h"

bool haltOnFails = false;
bool displayPassed = true;

void fail(std::string msg) {
	std::cout << "FAILED: " << msg << std::endl;
	if(haltOnFails)
		exit(1);
}

void pass(std::string msg, bool hidePassed) {
	if(!hidePassed && displayPassed)
		std::cout << "Passed: " << msg << std::endl;
}

void assertTrue(std::string msg, bool value, bool hidePassed) {
	if (!value) {
		fail(msg);
	} else {
		pass(msg, hidePassed);
	}
}

void assertFalse(std::string msg, bool value) { assertTrue(msg, !value); }



