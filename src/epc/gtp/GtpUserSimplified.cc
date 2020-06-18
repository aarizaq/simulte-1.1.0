//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "epc/gtp/GtpUserSimplified.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include <iostream>

Define_Module(GtpUserSimplified);

void GtpUserSimplified::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    // wait until all the IP addresses are configured
    if (stage != inet::INITSTAGE_APPLICATION_LAYER)
        return;
    localPort_ = par("localPort");

    // get reference to the binder
    binder_ = getBinder();

    socket_.setOutputGate(gate("socketOut"));
    socket_.bind(localPort_);

    tunnelPeerPort_ = par("tunnelPeerPort");

    // get the address of the PGW
    pgwAddress_ = L3AddressResolver().resolve("pgw");

    ownerType_ = selectOwnerType(getAncestorPar("nodeType"));
}

EpcNodeType GtpUserSimplified::selectOwnerType(const char * type)
{
    EV << "GtpUserSimplified::selectOwnerType - setting owner type to " << type << endl;
    if(strcmp(type,"ENODEB") == 0)
        return ENB;
    if(strcmp(type,"PGW") != 0)
        error("GtpUserSimplified::selectOwnerType - unknown owner type [%s]. Aborting...",type);
    return PGW;
}

void GtpUserSimplified::handleMessage(cMessage *msg)
{
    if (strcmp(msg->getArrivalGate()->getFullName(), "trafficFlowFilterGate") == 0)
    {
        EV << "GtpUserSimplified::handleMessage - message from trafficFlowFilter" << endl;
        // obtain the encapsulated IPv4 datagram
        auto pkt = check_and_cast<Packet*>(msg);
        pkt->trim();
        auto ipHeader = pkt->peekAtFront<Ipv4Header>();
        pkt->addTagIfAbsent<NetworkProtocolInd>()->setProtocol(&Protocol::ipv4);
        pkt->addTagIfAbsent<NetworkProtocolInd>()->setNetworkProtocolHeader(ipHeader);
        //IPv4Datagram * datagram = check_and_cast<IPv4Datagram*>(msg);
        handleFromTrafficFlowFilter(pkt);
    }
    else if(strcmp(msg->getArrivalGate()->getFullName(),"socketIn")==0)
    {
        EV << "GtpUserSimplified::handleMessage - message from udp layer" << endl;
        auto pkt = check_and_cast<Packet*>(msg);
        pkt->trim();
        auto gtpMsg = pkt->peekAtFront<GtpUserMsg>();
        //GtpUserMsg * gtpMsg = check_and_cast<GtpUserMsg *>(msg);
        handleFromUdp(pkt);
    }
}

void GtpUserSimplified::handleFromTrafficFlowFilter(Packet * pkt)
{
    // extract control info from the datagram
    auto tftInfo = pkt->removeTag<TftControlInfo>();
    //TftControlInfo * tftInfo = check_and_cast<TftControlInfo *>(datagram->removeControlInfo());
    TrafficFlowTemplateId flowId = tftInfo->getTft();
    delete (tftInfo);
    removeAllSimulLteTags(pkt);

    EV << "GtpUserSimplified::handleFromTrafficFlowFilter - Received a tftMessage with flowId[" << flowId << "]" << endl;

    // If we are on the eNB and the flowId represents the ID of this eNB, forward the packet locally
    if (flowId == 0)
    {
        // local delivery
        send(pkt,"pppGate");
    }
    else
    {
        // create a new GtpUserSimplifiedMessage
        auto gtpMsg = makeShared<GtpUserMsg>();

        auto ipHeader = pkt->peekAtFront<Ipv4Header>();
        pkt->addTagIfAbsent<NetworkProtocolInd>()->setProtocol(&Protocol::ipv4);
        pkt->addTagIfAbsent<NetworkProtocolInd>()->setNetworkProtocolHeader(ipHeader);
        //IPv4Datagram * datagram = check_and_cast<IPv4Datagram*>(msg);

        //gtpMsg->setName("GtpUserMessage");

        // encapsulate the datagram within the GtpUserSimplifiedMessage
        pkt->trim();
        pkt->insertAtFront(gtpMsg);
        //gtpMsg->encapsulate(datagram);

        L3Address tunnelPeerAddress;
        if (flowId == -1) // send to the PGW
        {
            tunnelPeerAddress = pgwAddress_;
        }
        else
        {
            // get the symbolic IP address of the tunnel destination ID
            // then obtain the address via IPvXAddressResolver
            const char* symbolicName = binder_->getModuleNameByMacNodeId(flowId);
            tunnelPeerAddress = L3AddressResolver().resolve(symbolicName);
        }
        socket_.sendTo(pkt, tunnelPeerAddress, tunnelPeerPort_);
    }
}

void GtpUserSimplified::handleFromUdp(Packet *pkt)
{
    EV << "GtpUserSimplified::handleFromUdp - Decapsulating datagram from GTP tunnel" << endl;

    auto gtpMsg = pkt->removeAtFront<GtpUserMsg>();
    // obtain the original IP datagram and send it to the local network
    auto ipHeader = pkt->peekAtFront<Ipv4Header>();
    pkt->addTagIfAbsent<NetworkProtocolInd>()->setProtocol(&Protocol::ipv4);
    pkt->addTagIfAbsent<NetworkProtocolInd>()->setNetworkProtocolHeader(ipHeader);

    //IPv4Datagram * datagram = check_and_cast<IPv4Datagram*>(gtpMsg->decapsulate());
    //delete(gtpMsg);

    if (ownerType_ == PGW)
    {
        Ipv4Address destAddr = ipHeader->getDestAddress();
        MacNodeId destId = binder_->getMacNodeId(destAddr);
        if (destId != 0)
        {
             // create a new GtpUserSimplifiedMessage
             auto gtpMsg = makeShared<GtpUserMsg>();
             //GtpUserMsg * gtpMsg = new GtpUserMsg();
             //gtpMsg->setName("GtpUserMessage");

             // encapsulate the datagram within the GtpUserSimplifiedMessage
             pkt->insertAtFront(gtpMsg);
             //gtpMsg->encapsulate(datagram);

             MacNodeId destMaster = binder_->getNextHop(destId);
             const char* symbolicName = binder_->getModuleNameByMacNodeId(destMaster);
             L3Address tunnelPeerAddress = L3AddressResolver().resolve(symbolicName);
             socket_.sendTo(pkt, tunnelPeerAddress, tunnelPeerPort_);
             EV << "GtpUserSimplified::handleFromUdp - Destination is a MEC server. Sending GTP packet to " << symbolicName << endl;
        }
        else
        {
            // destination is outside the LTE network
            EV << "GtpUserSimplified::handleFromUdp - Deliver datagram to the internet " << endl;
            send(pkt,"pppGate");
        }
    }
    else if (ownerType_ == ENB)
    {
        Ipv4Address destAddr = ipHeader->getDestAddress();
        MacNodeId destId = binder_->getMacNodeId(destAddr);
        if (destId != 0)
        {
            MacNodeId enbId = getAncestorPar("macNodeId");
            MacNodeId destMaster = binder_->getNextHop(destId);
            if (destMaster == enbId)
            {
                EV << "GtpUserSimplified::handleFromUdp - Deliver datagram to the LTE NIC " << endl;
                send(pkt,"pppGate");
                return;
            }
        }
        // send the message to the correct eNB or to the internet, through the PGW
        // create a new GtpUserSimplifiedMessage
        auto gtpMsg = makeShared<GtpUserMsg>();
        pkt->insertAtFront(gtpMsg);

        //GtpUserMsg * gtpMsg = new GtpUserMsg();
        //gtpMsg->setName("GtpUserMessage");
        // encapsulate the datagram within the GtpUserSimplifiedMessage
        //gtpMsg->encapsulate(datagram);

        socket_.sendTo(pkt, pgwAddress_, tunnelPeerPort_);

        EV << "GtpUserSimplified::handleFromUdp - Destination is not served by this eNodeB. Sending GTP packet to the PGW"<< endl;
    }
}
