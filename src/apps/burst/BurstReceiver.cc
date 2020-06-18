
#include "apps/burst/BurstReceiver.h"

Define_Module(BurstReceiver);

BurstReceiver::~BurstReceiver()
{

}

void BurstReceiver::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL)
    {
        mInit_ = true;

        numReceived_ = 0;

        recvBytes_ = 0;

        burstRcvdPkt_ = registerSignal("burstRcvdPkt");
        burstPktDelay_ = registerSignal("burstPktDelay");
    }
    else if (stage == INITSTAGE_APPLICATION_LAYER)
    {
        int port = par("localPort");
        EV << "BurstReceiver::initialize - binding to port: local:" << port << endl;
        if (port != -1)
        {
            socket.setOutputGate(gate("socketOut"));
            socket.bind(port);
        }
    }
}
/*
void BurstReceiver::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
        return;

    auto pkt = check_and_cast<inet::Packet *>(msg);
    auto pPacket = pkt->peekAtFront<BurstPacket>();

    numReceived_++;

    simtime_t delay = simTime() - pkt->getCreationTime();
    EV << "BurstReceiver::handleMessage - Packet received: FRAME[" << pPacket->getMsgId() << "] with delay["<< delay << "]" << endl;

    emit(burstPktDelay_, delay);
    emit(burstRcvdPkt_, (long)pPacket->getMsgId());

    delete msg;
}
*/

void BurstReceiver::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        delete msg;
        return;
    }
    else
        socket.processMessage(msg);
}

void BurstReceiver::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    auto pPacket = packet->peekAtFront<BurstPacket>();

    numReceived_++;

    simtime_t delay = simTime() - packet->getCreationTime();
    EV << "BurstReceiver::handleMessage - Packet received: FRAME[" << pPacket->getMsgId() << "] with delay["<< delay << "]" << endl;

    emit(burstPktDelay_, delay);
    emit(burstRcvdPkt_, (long)pPacket->getMsgId());

    delete packet;
}

void BurstReceiver::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}


