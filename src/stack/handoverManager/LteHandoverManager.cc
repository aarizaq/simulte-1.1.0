//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "stack/handoverManager/LteHandoverManager.h"
#include "stack/handoverManager/X2HandoverCommandIE.h"
#include "inet/networklayer/ipv4/Ipv4Header_m.h"

Define_Module(LteHandoverManager);

void LteHandoverManager::initialize()
{
    // get the node id
    nodeId_ = getAncestorPar("macCellId");

    // get reference to the gates
    x2Manager_[IN_GATE] = gate("x2ManagerIn");
    x2Manager_[OUT_GATE] = gate("x2ManagerOut");

    // get reference to the IP2lte layer
    ip2lte_ = check_and_cast<IP2lte*>(getParentModule()->getSubmodule("ip2lte"));

    losslessHandover_ = par("losslessHandover").boolValue();

    // register to the X2 Manager

    auto initMsg = makeShared<X2HandoverControlMsg>();
    auto pktAux = new Packet();
    pktAux->insertAtFront(initMsg);

    pktAux->addTagIfAbsent<X2ControlInfo>()->setInit(true);

    /*X2HandoverControlMsg* initMsg = new X2HandoverControlMsg();
    X2ControlInfo* ctrlInfo = new X2ControlInfo();
    ctrlInfo->setInit(true);
    initMsg->setControlInfo(ctrlInfo);*/
    send(pktAux, x2Manager_[OUT_GATE]);
}

void LteHandoverManager::handleMessage(cMessage *msg)
{
    Packet* pkt = check_and_cast<Packet*>(msg);
    cGate* incoming = pkt->getArrivalGate();
    if (incoming == x2Manager_[IN_GATE])
    {
        // incoming data from X2 Manager
        EV << "LteHandoverManager::handleMessage - Received message from X2 manager" << endl;
        handleX2Message(pkt);
    }
    else
        delete msg;
}



void LteHandoverManager::handleX2Message(Packet* pkt)
{
    auto x2msg = pkt->removeAtFront<LteX2Message>();
    X2NodeId sourceId = x2msg->getSourceId();

    if (x2msg->getType() == X2_HANDOVER_DATA_MSG)
    {
        Ptr<X2HandoverDataMsg> hoDataMsg = dynamicPtrCast<X2HandoverDataMsg>(x2msg);
        if (hoDataMsg.get() == nullptr)
            throw cRuntimeError("No of type X2HandoverDataMsg");
        //X2HandoverDataMsg* hoDataMsg = check_and_cast<X2HandoverDataMsg*>(x2msg);
        auto ipDatagram = pkt->peekAtFront<Ipv4Header>();
        pkt->addTagIfAbsent<NetworkProtocolInd>()->setProtocol(&Protocol::ipv4);
        pkt->addTagIfAbsent<NetworkProtocolInd>()->setNetworkProtocolHeader(ipDatagram);
        //inet::Ipv4Datagram* datagram = check_and_cast<IPv4Datagram*>(hoDataMsg->decapsulate());
        receiveDataFromSourceEnb(pkt, sourceId);
    }
    else   // X2_HANDOVER_CONTROL_MSG
    {
        Ptr<X2HandoverControlMsg> hoCommandMsg = dynamicPtrCast<X2HandoverControlMsg>(x2msg);
        if (hoCommandMsg.get() == nullptr)
            throw cRuntimeError("No of type X2HandoverControlMsg");
        //X2HandoverControlMsg* hoCommandMsg = check_and_cast<X2HandoverControlMsg*>(x2msg);
        auto pktAux = hoCommandMsg->popIe();
        auto hoCommandIe = pktAux->peekAtFront<X2HandoverCommandIE>();
        receiveHandoverCommand(hoCommandIe->getUeId(), hoCommandMsg->getSourceId(), hoCommandIe->isStartHandover());
        delete pktAux;
    }

    delete pkt;
}

/*
void LteHandoverManager::handleX2Message(Packet* pkt)
{
    LteX2Message* x2msg = check_and_cast<LteX2Message*>(pkt);
    X2NodeId sourceId = x2msg->getSourceId();

    if (x2msg->getType() == X2_HANDOVER_DATA_MSG)
    {
        X2HandoverDataMsg* hoDataMsg = check_and_cast<X2HandoverDataMsg*>(x2msg);
        IPv4Datagram* datagram = check_and_cast<IPv4Datagram*>(hoDataMsg->decapsulate());
        receiveDataFromSourceEnb(datagram, sourceId);
    }
    else   // X2_HANDOVER_CONTROL_MSG
    {
        X2HandoverControlMsg* hoCommandMsg = check_and_cast<X2HandoverControlMsg*>(x2msg);
        X2HandoverCommandIE* hoCommandIe = check_and_cast<X2HandoverCommandIE*>(hoCommandMsg->popIe());
        receiveHandoverCommand(hoCommandIe->getUeId(), hoCommandMsg->getSourceId(), hoCommandIe->isStartHandover());

        delete hoCommandIe;
    }

    delete x2msg;
}
*/

void LteHandoverManager::sendHandoverCommand(MacNodeId ueId, MacNodeId enb, bool startHo)
{
    Enter_Method("sendHandoverCommand");

    EV<<NOW<<" LteHandoverManager::sendHandoverCommand - Send handover command over X2 to eNB " << enb << " for UE " << ueId << endl;

    // build control info
    //X2ControlInfo* ctrlInfo = new X2ControlInfo();
    //ctrlInfo->setSourceId(nodeId_);
    //DestinationIdList destList;
    //destList.push_back(enb);
    //ctrlInfo->setDestIdList(destList);

    // build X2 Handover Msg
    auto hoCommandIe = makeShared<X2HandoverCommandIE>();
    // TODO: Change commands to tag structures.

    //X2HandoverCommandIE* hoCommandIe = new X2HandoverCommandIE();
    hoCommandIe->setUeId(ueId);
    if (startHo)
        hoCommandIe->setStartHandover();
    auto pkt1 = new Packet("X2HandoverCommandIE");
    pkt1->insertAtFront(hoCommandIe);

    //X2HandoverControlMsg* hoMsg = new X2HandoverControlMsg("X2HandoverControlMsg");
    auto hoMsg = makeShared<X2HandoverControlMsg>();
    auto pkt2 = new Packet("X2HandoverControlMsg");
    hoMsg->pushIe(pkt1);
    pkt2->insertAtFront(hoMsg);
    pkt2->addTagIfAbsent<X2ControlInfo>()->setSourceId(nodeId_);
    DestinationIdList destList;
    destList.push_back(enb);
    pkt2->addTagIfAbsent<X2ControlInfo>()->setDestIdList(destList);
    // send to X2 Manager
    send(pkt2,x2Manager_[OUT_GATE]);
}

void LteHandoverManager::receiveHandoverCommand(MacNodeId ueId, MacNodeId enb, bool startHo)
{
    EV<<NOW<<" LteHandoverManager::receivedHandoverCommand - Received handover command over X2 from eNB " << enb << " for UE " << ueId << endl;

    // send command to IP2lte/PDCP
    if (startHo)
        ip2lte_->triggerHandoverTarget(ueId, enb);
    else
        ip2lte_->signalHandoverCompleteSource(ueId, enb);
}


void LteHandoverManager::forwardDataToTargetEnb(Packet* pkt, MacNodeId targetEnb)
{
    Enter_Method("forwardDataToTargetEnb");
    take(pkt);

    auto ipHeader = pkt->peekAtFront<Ipv4Header>();

    // build control info
    pkt->addTagIfAbsent<X2ControlInfo>()->setSourceId(nodeId_);

    //X2ControlInfo* ctrlInfo = new X2ControlInfo();
    pkt->addTagIfAbsent<X2ControlInfo>()->setSourceId(nodeId_);
    DestinationIdList destList;
    destList.push_back(targetEnb);
    pkt->addTagIfAbsent<X2ControlInfo>()->setDestIdList(destList);

//    ctrlInfo->setSourceId(nodeId_);
//    DestinationIdList destList;
//    destList.push_back(targetEnb);
//    ctrlInfo->setDestIdList(destList);

    // build X2 Handover Msg
    auto hoMsg = makeShared<X2HandoverDataMsg>();
    pkt->insertAtFront(hoMsg);

    //X2HandoverDataMsg* hoMsg = new X2HandoverDataMsg("X2HandoverDataMsg");
    //hoMsg->encapsulate(datagram);
    //hoMsg->setControlInfo(ctrlInfo);

    EV<<NOW<<" LteHandoverManager::forwardDataToTargetEnb - Send IP datagram to eNB " << targetEnb << endl;

    // send to X2 Manager
    send(pkt,x2Manager_[OUT_GATE]);
}

void LteHandoverManager::receiveDataFromSourceEnb(Packet* pkt, MacNodeId sourceEnb)
{
    EV<<NOW<<" LteHandoverManager::receiveDataFromSourceEnb - Received IP datagram from eNB " << sourceEnb << endl;

    // send data to IP2lte/PDCP for transmission
    auto ipv4Header = pkt->peekAtFront<Ipv4Header>();
    const Protocol *protocol = ipv4Header->getProtocol();
    int protocolId = ipv4Header->getProtocolId();
    ip2lte_->receiveTunneledPacketOnHandover(pkt, sourceEnb);
}

