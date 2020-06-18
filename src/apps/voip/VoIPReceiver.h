//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#ifndef _LTE_VOIPRECEIVER_H_
#define _LTE_VOIPRECEIVER_H_

#include <string.h>
#include <omnetpp.h>

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "apps/voip/VoipPacket_m.h"
#include "inet/applications/base/ApplicationBase.h"

#include <list>

using namespace inet;

class VoIPReceiver : public inet::ApplicationBase, public inet::UdpSocket::ICallback
{
    UdpSocket socket;

    ~VoIPReceiver();

    int emodel_Ie_;
    int emodel_Bpl_;
    int emodel_A_;
    double emodel_Ro_;

    typedef std::list<inet::Packet*> PacketsList;
    PacketsList mPacketsList_;
    PacketsList mPlayoutQueue_;
    unsigned int mCurrentTalkspurt_;
    unsigned int mBufferSpace_;
    simtime_t mSamplingDelta_;
    simtime_t mPlayoutDelay_;

    bool mInit_;

    unsigned int totalRcvdBytes_;
    simtime_t warmUpPer_;

    simsignal_t voIPFrameLossSignal_;
    simsignal_t voIPFrameDelaySignal_;
    simsignal_t voIPPlayoutDelaySignal_;
    simsignal_t voIPMosSignal_;
    simsignal_t voIPTaildropLossSignal_;
    simsignal_t voIPPlayoutLossSignal_;
    simsignal_t voIPJitterSignal_;
    simsignal_t voIPReceivedThroughput_;

    virtual void finish();

  protected:
    virtual void handleStartOperation(LifecycleOperation *operation) override {};
    virtual void handleStopOperation(LifecycleOperation *operation) override {};
    virtual void handleCrashOperation(LifecycleOperation *operation) override {};


    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override {};
    //void handleMessage(cMessage *msg);
    double eModel(simtime_t delay, double loss);
    void playout(bool finish);
};

#endif

