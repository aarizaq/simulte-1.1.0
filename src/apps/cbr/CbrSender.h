//
//

#ifndef _CBRSENDER_H_
#define _CBRSENDER_H_

#include <string.h>
#include <omnetpp.h>

#include "inet/common/INETDefs.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/applications/base/ApplicationBase.h"

#include "CbrPacket_m.h"

using namespace inet;

class CbrSender : public inet::ApplicationBase, public inet::UdpSocket::ICallback
{
    UdpSocket socket;
    //has the sender been initialized?
    bool initialized_;

    cMessage* selfSource_;
    //sender
    int iDtalk_;
    int nframes_;
    int iDframe_;
    int nframesTmp_;
    int size_;
    simtime_t sampling_time;
    simtime_t startTime_;
    simtime_t finishTime_;

    bool silences_;

    static simsignal_t cbrGeneratedThroughtputSignal_;
    static simsignal_t cbrGeneratedBytesSignal_;
    static simsignal_t cbrSentPktSignal_;

    int txBytes_;
    // ----------------------------

    cMessage *selfSender_;
    cMessage *initTraffic_;

    simtime_t timestamp_;
    int localPort_;
    int destPort_;
    L3Address destAddress_;

    void initTraffic();
    void sendCbrPacket();

  public:
    ~CbrSender();
    CbrSender();

  protected:
    virtual void handleStartOperation(LifecycleOperation *operation) override {};
    virtual void handleStopOperation(LifecycleOperation *operation) override {};
    virtual void handleCrashOperation(LifecycleOperation *operation) override {};


    virtual void initialize(int stage) override;
    virtual void finish() override;
    //void handleMessage(cMessage *msg) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override {};
};

#endif

