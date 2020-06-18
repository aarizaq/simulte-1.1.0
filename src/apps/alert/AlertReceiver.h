//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#ifndef _LTE_AlertReceiver_H_
#define _LTE_AlertReceiver_H_

#include <string.h>
#include <omnetpp.h>

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "apps/alert/AlertPacket_m.h"
#include "inet/applications/base/ApplicationBase.h"

using namespace inet;

class AlertReceiver : public inet::ApplicationBase, public inet::UdpSocket::ICallback
{
    UdpSocket socket;

    simsignal_t alertDelay_;
    simsignal_t alertRcvdMsg_;

  protected:
    void processPacket(inet::Packet *pkt);

    virtual void handleStartOperation(LifecycleOperation *operation) override {};
    virtual void handleStopOperation(LifecycleOperation *operation) override {};
    virtual void handleCrashOperation(LifecycleOperation *operation) override {};

    virtual void initialize(int stage) override;
    // void handleMessage(cMessage *msg);
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override {};
};

#endif

