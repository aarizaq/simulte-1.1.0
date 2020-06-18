
#include "CbrReceiver.h"

Define_Module(CbrReceiver);

simsignal_t CbrReceiver::cbrFrameLossSignal_ = registerSignal("cbrFrameLossSignal");
simsignal_t CbrReceiver::cbrFrameDelaySignal_ = registerSignal("cbrFrameDelaySignal");
simsignal_t CbrReceiver::cbrJitterSignal_ = registerSignal("cbrJitterSignal");
simsignal_t CbrReceiver::cbrReceivedThroughtput_ = registerSignal("cbrReceivedThroughtputSignal");
simsignal_t CbrReceiver::cbrReceivedBytesSignal_ = registerSignal("cbrReceivedBytesSignal");

CbrReceiver::~CbrReceiver()
{

}

void CbrReceiver::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        mInit_ = true;

        numReceived_ = 0;

        recvBytes_ = 0;

        cbrRcvdPkt_ = registerSignal("cbrRcvdPkt");
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER)
    {
        int port = par("localPort");
        EV << "CbrReceiver::initialize - binding to port: local:" << port << endl;
        if (port != -1)
        {
            socket.setOutputGate(gate("socketOut"));
            socket.bind(port);
        }
    }
}

/*
void CbrReceiver::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
        return;

    auto pkt = check_and_cast<inet::Packet *>(msg);

    auto pPacket = pkt->peekAtFront<CbrPacket>();

    numReceived_++;
    totFrames_ = pPacket->getNframes(); // XXX this value can be written just once
    int pktSize = (int)pkt->getByteLength();

    // just to make sure we do not update recvBytes AND we avoid dividing by 0
    if( simTime() > getSimulation()->getWarmupPeriod() )
    {
        recvBytes_ += pktSize;
        emit( cbrReceivedBytesSignal_ , pktSize );
    }

    simtime_t delay = simTime() - pkt->getCreationTime();
    emit(cbrFrameDelaySignal_,delay );

    EV << "CbrReceiver::handleMessage - Packet received: FRAME[" << pPacket->getIDframe() << "/" << pPacket->getNframes() << "] with delay["<< delay << "]" << endl;

    emit(cbrRcvdPkt_, (long)pPacket->getIDframe());

    delete msg;
}
*/

void CbrReceiver::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
       delete msg;
    }
    else
        socket.processMessage(msg);
}

void CbrReceiver::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    auto pPacket = packet->peekAtFront<CbrPacket>();
    numReceived_++;
    totFrames_ = pPacket->getNframes(); // XXX this value can be written just once
    int pktSize = (int)packet->getByteLength();

    // just to make sure we do not update recvBytes AND we avoid dividing by 0
    if( simTime() > getSimulation()->getWarmupPeriod() )
    {
        recvBytes_ += pktSize;
        emit( cbrReceivedBytesSignal_ , pktSize );
    }

    simtime_t delay = simTime() - packet->getCreationTime();
    emit(cbrFrameDelaySignal_,delay );


    EV << "CbrReceiver::handleMessage - Packet received: FRAME[" << pPacket->getIDframe() << "/" << pPacket->getNframes() << "] with delay["<< delay << "]" << endl;

    emit(cbrRcvdPkt_, (long)pPacket->getIDframe());
    delete packet;
}

void CbrReceiver::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void CbrReceiver::finish()
{
    double lossRate = 0;
    if(totFrames_ > 0)
        lossRate = 1.0-(numReceived_/(totFrames_*1.0));

    emit(cbrFrameLossSignal_,lossRate);

    simtime_t elapsedTime = simTime() - getSimulation()->getWarmupPeriod();
    emit( cbrReceivedThroughtput_, recvBytes_ / elapsedTime );
}



