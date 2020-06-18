//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "apps/alert/AlertReceiver.h"
#include "inet/common/ModuleAccess.h"  // for multicast support

Define_Module(AlertReceiver);

void AlertReceiver::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage != inet::INITSTAGE_APPLICATION_LAYER)
        return;

    int port = par("localPort");
    EV << "AlertReceiver::initialize - binding to port: local:" << port << endl;
    if (port != -1)
    {
        socket.setOutputGate(gate("socketOut"));
        socket.bind(port);

        // for multicast support
        inet::IInterfaceTable *ift = inet::getModuleFromPar<inet::IInterfaceTable>(par("interfaceTableModule"), this);
        inet::MulticastGroupList mgl = ift->collectMulticastGroups();
        socket.joinLocalMulticastGroups(mgl);
        inet::InterfaceEntry *ie = ift->findInterfaceByName("wlan");
        if (!ie)
            throw cRuntimeError("Wrong multicastInterface setting: no interface named wlan");
        socket.setMulticastOutputInterface(ie->getInterfaceId());
        // -------------------- //
    }

    alertDelay_ = registerSignal("alertDelay");
    alertRcvdMsg_ = registerSignal("alertRcvdMsg");
}

void AlertReceiver::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage())
        return;
    socket.processMessage(msg);
}


void AlertReceiver::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    processPacket(packet);
}

void AlertReceiver::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void AlertReceiver::processPacket(inet::Packet *pkt) {

    if (pkt->getKind() == UDP_I_ERROR) {
        EV_WARN << "UDP error received\n";
        delete pkt;
        return;
    }

    auto alertHeader = pkt->peekAtFront<AlertPacket>();

    if (pkt == 0)
        throw cRuntimeError("AlertReceiver::handleMessage - FATAL! Error when casting to AlertPacket");

    // emit statistics
    simtime_t delay = simTime() - pkt->getCreationTime();
    emit(alertDelay_, delay);
    emit(alertRcvdMsg_, (long)1);

    EV << "AlertReceiver::handleMessage - Packet received: SeqNo[" << alertHeader->getSno() << "] Delay[" << delay << "]" << endl;

    delete pkt;
}

