//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include <stdlib.h>
#include <stdio.h>
#include "x2/X2AppServer.h"
#include "inet/transportlayer/contract/sctp/SctpCommand_m.h"
#include "inet/transportlayer/sctp/SctpHeader_m.h"

#include "inet/transportlayer/contract/sctp/SctpSocket.h"
#include "inet/transportlayer/sctp/SctpAssociation.h"
#include "inet/applications/common/SocketTag_m.h"

Define_Module(X2AppServer);

void X2AppServer::initialize(int stage)
{
    SctpServer::initialize(stage);
    if (stage==inet::INITSTAGE_LOCAL)
    {
        x2ManagerIn_ = gate("x2ManagerIn");

        X2NodeId id = getAncestorPar("macCellId");

        // register listening port to the binder. It will be used by
        // the client side as connectPort
        int localPort = par("localPort");
        getBinder()->registerX2Port(id, localPort);
    }
}

void X2AppServer::generateAndSend(Packet* pkt)
{
    //cPacket* cmsg = new cPacket("CMSG");
    //SctpSimpleMessage* msg = new SctpSimpleMessage("Server");
    //int numBytes = pkt->getByteLength();
    auto cmd = pkt->addTag<SctpSendReq>();
    //msg->setDataArraySize(numBytes);
    //for (int i=0; i<numBytes; i++)
    //    msg->setData(i, 's');

    //msg->setDataLen(numBytes);

    // encapsulate packet
    // msg->setEncaps(true);
    // msg->encapsulate(pkt);

    //msg->setBitLength(numBytes * 8);
    //cmsg->encapsulate(msg);
    //SctpSendInfo *cmd = new SCTPSendInfo("Send1");
    cmd->setSocketId(assocId);
    cmd->setSendUnordered(ordered ? COMPLETE_MESG_ORDERED : COMPLETE_MESG_UNORDERED);
    lastStream = (lastStream+1)%outboundStreams;
    cmd->setSid(lastStream);
    cmd->setPrValue(par("prValue"));
    cmd->setPrMethod((int32)par("prMethod"));
    if (queueSize>0 && numRequestsToSend > 0 && count < queueSize*2)
        cmd->setLast(false);
    else
        cmd->setLast(true);

    pkt->setKind(ordered ? SCTP_C_SEND_ORDERED : SCTP_C_SEND_UNORDERED);
    pkt->addTag<SocketReq>()->setSocketId(assocId);
    pkt->addTag<DispatchProtocolReq>()->setProtocol(&Protocol::sctp);

    //pkt->setKind(SCTP_C_SEND);
    //cmsg->setControlInfo(cmd);
    packetsSent++;
    bytesSent += pkt->getBitLength()/8;

    sendOrSchedule(pkt);
}

void X2AppServer::handleMessage(cMessage *msg)
{
    Packet* pkt = check_and_cast<Packet*>(msg);
    cGate* incoming = pkt->getArrivalGate();
    if (incoming == x2ManagerIn_)
    {
        EV << "X2AppServer::handleMessage - Received message from x2 manager" << endl;
        EV << "X2AppServer::handleMessage - Forwarding to X2 interface" << endl;

        // generate a SCTP packet and sent to lower layer
        generateAndSend(pkt);
    }
    else
    {
        SctpServer::handleMessage(msg);
    }
}

/*
void X2AppServer::handleTimer(cMessage *msg)
{
    SctpServer::handleTimer(msg);
}
*/
