
#include "UnitTest.h"

#include "XMLUtils.h"

void UnitTest::activity()
{
	Daemon::init();
	config = par("config").xmlValue();
	outp = par("output").xmlValue();
	execute();
	
	// XXX FIXME: oppsim_exit???
	cQueue queue;
	while(true)	waitAndEnqueue(100000, &queue);  
}

void UnitTest::output(std::string s)
{
	EV << "OUTPUT: " << s << endl;
	outMessage.push_back(s);
}

void UnitTest::output(std::stringstream& s)
{
	output(s.str());
	s.str("");
}

void UnitTest::finish()
{
	cXMLElementList list = outp->getChildrenByTagName("line");
	
	bool match = (list.size() == outMessage.size());
	for(int i = 0; match && i < list.size(); i++)
	{
		if(strcmp(outMessage[i].c_str(), list[i]->getNodeValue())) match = false;
	}

	if(!match)
	{
		EV << "Expected:" << endl;
		for(int i = 0; i < list.size(); i++)
			EV << list[i]->getNodeValue() << endl;

		EV << "Got:" << endl;
		for(int i = 0; i < outMessage.size(); i++)
			EV << outMessage[i] << endl;
		
		opp_error("Test failed");
	}	
}

inline char nextChar(char c)
{
	if(c == '9') return 'A';
	else if(c == 'F') return '1';
	else return c + 1;
}

inline int charIndex(char c)
{
	if(c >= '1' && c <= '9') return c - '1';
	else if(c >= 'A' && c <= 'F') return c - 'A' + 9;
	else return -1;
}

std::string createMessage(int size)
{
	std::string ret;
	char c = '1';
	for(int i = 0; i < size; i++) {
		ret.append(1, c);
		c = nextChar(c);
	}
	return ret;
}

int checkMessage(const char *s, int size)
{
	int ret;
	if((ret = charIndex(s[0])) == -1) return -1;
	char c = s[0];
	for(int i = 0; i < size; i++)
	{
		if(s[i] != c) return -1;
		c = nextChar(c);
	}
	return ret;
}

