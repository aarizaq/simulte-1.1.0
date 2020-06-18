//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#define DATAPORT_OUT "dataPort$o"
#define DATAPORT_IN "dataPort$i"

#include "x2/LteX2Manager.h"
#include "inet/networklayer/common/InterfaceEntry.h"
#include "inet/networklayer/ipv4/Ipv4InterfaceData.h"
#include "inet/networklayer/configurator/ipv4/Ipv4NetworkConfigurator.h"

Define_Module(LteX2Manager);

LteX2Manager::LteX2Manager() {
}

LteX2Manager::~LteX2Manager() {
}

void LteX2Manager::initialize(int stage)
{
    if (stage == inet::INITSTAGE_LOCAL)
    {
        // get the node id
        nodeId_ = getAncestorPar("macCellId");
    }
    else if (stage == inet::INITSTAGE_NETWORK_LAYER)
    {
        // find x2ppp interface entries and register their IP addresses to the binder
        // IP addresses will be used in the next init stage to get the X2 id of the peer
        Ipv4NetworkConfigurator* configurator = check_and_cast<Ipv4NetworkConfigurator*>(getModuleByPath("configurator"));
        IInterfaceTable *interfaceTable =  configurator->findInterfaceTableOf(getParentModule()->getParentModule());
        for (int i=0; i<interfaceTable->getNumInterfaces(); i++)
        {
            // look for x2ppp interfaces in the interface table
            InterfaceEntry * interfaceEntry = interfaceTable->getInterface(i);
            const char* ifName = interfaceEntry->getInterfaceName();
            if (strstr(ifName,"x2ppp") != NULL)
            {
                Ipv4Address addr = interfaceEntry->getProtocolData<Ipv4InterfaceData>()->getIPAddress();
                getBinder()->setX2NodeId(interfaceEntry->getProtocolData<Ipv4InterfaceData>()->getIPAddress(), nodeId_);
            }
        }
    }
    else if (stage == inet::INITSTAGE_TRANSPORT_LAYER)
    {
        // for each X2App, get the client submodule and set connection parameters (connectPort)
        for (int i=0; i<gateSize("x2$i"); i++)
        {
            // client of the X2Apps is connected to the input sides of "x2" gate
            cGate* inGate = gate("x2$i",i);

            // get the X2App client connected to this gate
            //                                                  x2  -> X2App.x2ManagerIn ->  X2App.client
            X2AppClient* client = check_and_cast<X2AppClient*>(inGate->getPathStartGate()->getOwnerModule());

            // get the connectAddress for the X2App client and the corresponding X2 id
            L3Address addr = L3AddressResolver().resolve(client->par("connectAddress").stringValue());
            X2NodeId peerId = getBinder()->getX2NodeId(addr.toIpv4());

            // bind the peerId to the output gate
            x2InterfaceTable_[peerId] = i;
        }
    }
}

void LteX2Manager::handleMessage(cMessage *msg)
{
    cPacket* pkt = check_and_cast<cPacket*>(msg);
    cGate* incoming = pkt->getArrivalGate();

    // the incoming gate is part of a gate vector, so get the base name
    if (strcmp(incoming->getBaseName(), "dataPort") == 0)
    {
        // incoming data from LTE stack
        EV << "LteX2Manager::handleMessage - Received message from LTE stack" << endl;

        fromStack(pkt);
    }
    else  // from X2
    {
        // the incoming gate belongs to a gate vector, so get its index
        int gateIndex = incoming->getIndex();

        // incoming data from X2
        EV << "LteX2Manager::handleMessage - Received message from X2, gate " << gateIndex << endl;

        // call handler
        fromX2(pkt);
    }
}

void LteX2Manager::fromStack(cPacket* pktAux)
{

    auto pkt = check_and_cast<Packet *>(pktAux);
    auto x2msg = pkt->peekAtFront<LteX2Message>();
    auto x2Info = pkt->removeTag<X2ControlInfo>();
    //LteX2Message* x2msg = check_and_cast<LteX2Message*>(pkt);
    //X2ControlInfo* x2Info = check_and_cast<X2ControlInfo*>(x2msg->removeControlInfo());


    if (x2Info->getInit())
    {
        // gate initialization
        LteX2MessageType msgType = x2msg->getType();
        int gateIndex = pktAux->getArrivalGate()->getIndex();
        dataInterfaceTable_[msgType] = gateIndex;

        delete x2Info;
        delete pktAux;
        return;
    }

    // If the message is a HandoverDataMsg, send to the GTPUserX2 module
    if (x2msg->getType() == X2_HANDOVER_DATA_MSG)
    {
        auto x2msg2 = pkt->removeAtFront<LteX2Message>();
        // GTPUserX2 module will tunnel this datagram towards the target eNB
        DestinationIdList destList = x2Info->getDestIdList();
        DestinationIdList::iterator it = destList.begin();
        for (; it != destList.end(); ++it)
        {
            auto pktAux = pkt->dup();
            auto x2msg2Aux = pktAux->removeAtFront<LteX2Message>();
            X2NodeId targetEnb = *it;
            x2msg2Aux->setSourceId(nodeId_);
            x2msg2Aux->setDestinationId(targetEnb);
            pktAux->insertAtFront(x2msg2Aux);

            // send to the gate connected to the GTPUser module
            cGate* outputGate = gate("x2Gtp$o");
            send(pktAux, outputGate);
        }
        delete pkt;
        delete x2Info;
    }
    else  // X2 control messages
    {
        DestinationIdList destList = x2Info->getDestIdList();
        DestinationIdList::iterator it = destList.begin();
        for (; it != destList.end(); ++it)
        {
            // send a X2 message to each destination eNodeB
            auto pktDup = pkt->dup();

            //LteX2Message* x2msg_dup = x2msg->dup();
            auto x2msg_dup = pktDup->removeAtFront<LteX2Message>();
            x2msg_dup->setSourceId(nodeId_);
            x2msg_dup->setDestinationId(*it);
            pktDup->insertAtFront(x2msg_dup);

            // select the index for the output gate (it belongs to a vector)
            int gateIndex = x2InterfaceTable_[*it];
            cGate* outputGate = gate("x2$o",gateIndex);
            send(pktDup, outputGate);
        }
        delete x2Info;
        delete pkt;
    }

}

void LteX2Manager::fromX2(cPacket* pktAux)
{
    //LteX2Message* x2msg = check_and_cast<LteX2Message*>(pkt);
    //

    auto pkt = check_and_cast<Packet *>(pktAux);
    auto x2msg = pkt->peekAtFront<LteX2Message>();
    LteX2MessageType msgType = x2msg->getType();

    if (msgType == X2_UNKNOWN_MSG)
    {
        EV << " LteX2Manager::fromX2 - Unknown type of the X2 message. Discard." << endl;
        return;
    }

    // get the correct output gate for the message
    int gateIndex = dataInterfaceTable_[msgType];
    cGate* outGate = gate(DATAPORT_OUT, gateIndex);

    // send X2 msg to stack
    EV << "LteX2Manager::fromX2 - send X2MSG to LTE stack" << endl;
    send(pkt, outGate);
}
