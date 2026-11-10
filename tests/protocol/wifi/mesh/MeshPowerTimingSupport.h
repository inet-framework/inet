// SPDX-License-Identifier: LGPL-3.0-or-later
#ifndef INET_TEST_MESHPOWERTIMINGSUPPORT_H
#define INET_TEST_MESHPOWERTIMINGSUPPORT_H

// Observation-only proxy delegates every selection to the production owner.
class TimRateProbe : public IQosRateSelection
{
    IQosRateSelection *delegate;
  public:
    bool largeTimSeen=false;
    bool conditionalWindowSeen=false;
    explicit TimRateProbe(IQosRateSelection *value):delegate(value){}
    const IIeee80211Mode *computeResponseCtsFrameMode(Packet *p,const Ptr<const Ieee80211RtsFrame>& h,ResponseQueryRole r) override{return delegate->computeResponseCtsFrameMode(p,h,r);}
    const IIeee80211Mode *computeResponseAckFrameMode(Packet *p,const Ptr<const Ieee80211DataOrMgmtHeader>& h,ResponseQueryRole r) override{return delegate->computeResponseAckFrameMode(p,h,r);}
    const IIeee80211Mode *computeResponseBlockAckFrameMode(Packet *p,const Ptr<const Ieee80211TwoAddressHeader>& h,ResponseQueryRole r) override{return delegate->computeResponseBlockAckFrameMode(p,h,r);}
    const IIeee80211Mode *computeCompressedBlockAckReqMode(Packet *p,const Ptr<const Ieee80211CompressedBlockAckReq>& h,bool start) override{return delegate->computeCompressedBlockAckReqMode(p,h,start);}
    std::pair<const IIeee80211Mode *,const IIeee80211Mode *> computeMeshPeerModes(MacAddress p,const Ieee80211RateSetState& r,const Ieee80211PeerPhyCapabilities& c) override{return delegate->computeMeshPeerModes(p,r,c);}
    physicallayer::Ieee80211TxVector computeResponseTxVector(Packet *p,const IIeee80211Mode *m,ResponseQueryRole r) const override{return delegate->computeResponseTxVector(p,m,r);}
    const IIeee80211Mode *computeMode(Packet *packet,const Ptr<const Ieee80211MacHeader>& header,TxopProcedure *txop) override{
        if(header->getType()==ST_BEACON){
            auto body=packet->peekDataAt<Ieee80211BeaconFrame>(header->getChunkLength());
            if(body->getTim().bitmapLength==129){ASSERT(packet->getDataLength()==B(210));largeTimSeen=true;}
            if(body->getTim().dtimPeriod==2 && body->getTim().dtimCount==1
                    && body->getMeshAwakeWindowPresent()){
                ASSERT(body->getTim().bitmapLength==1 && packet->getDataLength()==B(82));
                conditionalWindowSeen=true;
            }
        }
        return delegate->computeMode(packet,header,txop);
    }
};
class TimHcfFixture : public Hcf
{
    std::unique_ptr<TimRateProbe> probe;
  protected:
    void initialize(int stage) override{
        Hcf::initialize(stage);
        if(stage==INITSTAGE_LOCAL){probe=std::make_unique<TimRateProbe>(rateSelection);rateSelection=probe.get();}
    }
  public:
    bool sawLargeTimBeforeRate() const{return probe->largeTimSeen;}
    bool sawConditionalWindowBeforeRate() const{return probe->conditionalWindowSeen;}
};
namespace power_timing_registration { Define_Module(::TimHcfFixture); }
#endif
