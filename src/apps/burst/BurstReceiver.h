#ifndef _BURSTRECEIVER_H_
#define _BURSTRECEIVER_H_

#include <string.h>
#include <omnetpp.h>

#include "inet/common/INETDefs.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/applications/base/ApplicationBase.h"

#include "BurstPacket_m.h"

using namespace inet;

class BurstReceiver : public inet::ApplicationBase, public inet::UdpSocket::ICallback
{
    UdpSocket socket;

    ~BurstReceiver();

    int numReceived_;
    int recvBytes_;

    bool mInit_;

    simsignal_t burstRcvdPkt_;
    simsignal_t burstPktDelay_;

  protected:

    virtual void handleStartOperation(LifecycleOperation *operation) override {};
    virtual void handleStopOperation(LifecycleOperation *operation) override {};
    virtual void handleCrashOperation(LifecycleOperation *operation) override {};


    virtual void initialize(int stage) override;
    // void handleMessage(cMessage *msg) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override {};
};

#endif

