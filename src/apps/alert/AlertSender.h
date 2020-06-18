//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#ifndef _LTE_ALERTSENDER_H_
#define _LTE_ALERTSENDER_H_

#include <string.h>
#include <omnetpp.h>
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "apps/alert/AlertPacket_m.h"
#include "inet/applications/base/ApplicationBase.h"

using namespace inet;

class AlertSender : public inet::ApplicationBase, public inet::UdpSocket::ICallback
{
    UdpSocket socket;

    //sender
    int nextSno_;
    int size_;
    simtime_t period_;

    simtime_t stopTime_;

    simsignal_t alertSentMsg_;
    // ----------------------------

    cMessage *selfSender_;

    int localPort_;
    int destPort_;
    inet::L3Address destAddress_;

    void sendAlertPacket();

  public:
    ~AlertSender();
    AlertSender();

  protected:

    virtual void handleStartOperation(LifecycleOperation *operation) override {};
    virtual void handleStopOperation(LifecycleOperation *operation) override {};
    virtual void handleCrashOperation(LifecycleOperation *operation) override {};


    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override {};
};

#endif

