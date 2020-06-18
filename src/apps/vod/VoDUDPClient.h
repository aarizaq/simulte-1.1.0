//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//
//

#ifndef _LTE_VODUDPCLIENT_H_
#define _LTE_VODUDPCLIENT_H_

#include <omnetpp.h>
#include <string.h>
#include <fstream>

#include "apps/vod/VoDPacket_m.h"
#include "apps/vod/VoDUDPStruct.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/applications/base/ApplicationBase.h"

using namespace std;
using namespace inet;

class VoDUDPClient : public inet::ApplicationBase, public inet::UdpSocket::ICallback
{
    UdpSocket socket;
    fstream outfile;
    unsigned int totalRcvdBytes_;

  public:
    simsignal_t tptLayer0_;
    simsignal_t tptLayer1_;
    simsignal_t tptLayer2_;
    simsignal_t tptLayer3_;
    simsignal_t delayLayer0_;
    simsignal_t delayLayer1_;
    simsignal_t delayLayer2_;
    simsignal_t delayLayer3_;

  protected:

    virtual void handleStartOperation(LifecycleOperation *operation) override {};
    virtual void handleStopOperation(LifecycleOperation *operation) override {};
    virtual void handleCrashOperation(LifecycleOperation *operation) override {};

    virtual void initialize(int stage) override;
    virtual void finish() override;
    //virtual void handleMessage(cMessage *msg);
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override {};
    virtual void receiveStream(inet::Packet *msg);
};

#endif

