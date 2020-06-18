#ifndef _CBRRECEIVER_H_
#define _CBRRECEIVER_H_

#include <string.h>
#include <omnetpp.h>

#include "inet/common/INETDefs.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/applications/base/ApplicationBase.h"

#include "CbrPacket_m.h"

using namespace inet;

class CbrReceiver : public inet::ApplicationBase, public inet::UdpSocket::ICallback
{
    UdpSocket socket;

    ~CbrReceiver();

    int numReceived_;
    int totFrames_;

    int recvBytes_;


    bool mInit_;

    static simsignal_t cbrFrameLossSignal_;
    static simsignal_t cbrFrameDelaySignal_;
    static simsignal_t cbrJitterSignal_;
    static simsignal_t cbrReceivedThroughtput_;
    static simsignal_t cbrReceivedBytesSignal_;

    simsignal_t cbrRcvdPkt_;

  protected:
    virtual void handleStartOperation(LifecycleOperation *operation) override {};
    virtual void handleStopOperation(LifecycleOperation *operation) override {};
    virtual void handleCrashOperation(LifecycleOperation *operation) override {};


    virtual void initialize(int stage) override;
    //void handleMessage(cMessage *msg) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override {};
    virtual void finish() override;
};

#endif

