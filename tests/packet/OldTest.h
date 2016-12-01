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

#ifndef __NEWTEST_H_
#define __NEWTEST_H_

#include "inet/common/INETDefs.h"
#include "OldTest_m.h"

namespace inet {

class TcpSegment : public TcpSegment_Base
{
  private:
    void copy(const TcpSegment& other);

  public:
    TcpSegment(const char *name=nullptr, int kind=0) : TcpSegment_Base(name,kind) {}
    TcpSegment(const TcpSegment& other) : TcpSegment_Base(other) {copy(other);}
    virtual ~TcpSegment();
    TcpSegment& operator=(const TcpSegment& other) {if (this==&other) return *this; TcpSegment_Base::operator=(other); copy(other); return *this;}
    virtual TcpSegment *dup() const {return new TcpSegment(*this);}
};

class OldMedium
{
  protected:
    bool serialize = false;
    std::vector<cPacket *> packets;

  protected:
    cPacket *serializePacket(cPacket *packet);
    cPacket *deserializePacket(cPacket *packet);

  public:
    OldMedium(bool serialize) : serialize(serialize) { }
    ~OldMedium() { for (auto packet : packets) delete packet; }

    void sendPacket(cPacket *packet);
    const std::vector<cPacket *> receivePackets();
};

class OldSender
{
  protected:
    OldMedium& medium;
    TcpSegment *tcpSegment = nullptr;

  protected:
    void sendEthernet(cPacket *packet);
    void sendIp(cPacket *packet);
    void sendTcp(cPacket *packet);
    TcpSegment *createTcpSegment();

  public:
    OldSender(OldMedium& medium) : medium(medium) { }
    ~OldSender() { delete tcpSegment; }

    void sendPackets();
};

class OldReceiver
{
  protected:
    OldMedium& medium;
    std::vector<Fragment *> applicationData;
    int applicationDataIndex = 0;
    int applicationDataPosition = 0;
    int applicationDataByteLength = 0;

  protected:
    cPacket *getApplicationData(int byteLength);
    void receiveApplication(std::vector<Fragment *>& fragments);
    void receiveTcp(cPacket *packet);
    void receiveIp(cPacket *packet);
    void receiveEthernet(cPacket *packet);

  public:
    OldReceiver(OldMedium& medium) : medium(medium) { }
    ~OldReceiver() { }

    void receivePackets();
};

class OldTest : public cSimpleModule
{
  protected:
    double runtime = -1;

  protected:
    void initialize() override;
    void finish() override;
};

} // namespace

#endif // #ifndef __NEWTEST_H_

