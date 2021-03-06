//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "stack/rlc/um/entity/UmTxEntity.h"
#include "stack/rlc/am/packet/LteRlcAmPdu.h"

Define_Module(UmTxEntity);

/*
 * Main functions
 */

void UmTxEntity::initialize()
{
    sno_ = 0;
    firstIsFragment_ = false;
    notifyEmptyBuffer_ = false;
    holdingDownstreamInPackets_ = false;

    // store the node id of the owner module
    LteMacBase* mac = check_and_cast<LteMacBase*>(getParentModule()->getParentModule()->getSubmodule("mac"));
    ownerNodeId_ = mac->getMacNodeId();

    // get the reference to the RLC module
    lteRlc_ = check_and_cast<LteRlcUm*>(getParentModule()->getSubmodule("um"));
}

void UmTxEntity::enque(cPacket* pkt)
{
    EV << NOW << " UmTxEntity::enque - bufferize new SDU  " << endl;
    // Buffer the SDU in the TX buffer
    sduQueue_.insert(pkt);
}

void UmTxEntity::rlcPduMake(int pduLength)
{
    EV << NOW << " UmTxEntity::rlcPduMake - PDU with size " << pduLength << " requested from MAC"<< endl;

    // create the RLC PDU
    auto pktAux = new inet::Packet("lteRlcFragment");
    auto rlcPdu = inet::makeShared<LteRlcUmDataPdu>();
    //LteRlcUmDataPdu* rlcPdu = new LteRlcUmDataPdu("lteRlcFragment");

    // the request from MAC takes into account also the size of the RLC header
    pduLength -= RLC_HEADER_UM;

    int len = 0;

    bool startFrag = firstIsFragment_;
    bool endFrag = false;

    while (!sduQueue_.isEmpty() && pduLength > 0)
    {
        // detach data from the SDU buffer
        auto pkt = check_and_cast<inet::Packet *>(sduQueue_.front());
        //cPacket* pkt = sduQueue_.front();
        auto rlcSdu = pkt->removeAtFront<LteRlcSdu>();
        //LteRlcSdu* rlcSdu = check_and_cast<LteRlcSdu*>(pkt);
        unsigned int sduSequenceNumber = rlcSdu->getSnoMainPacket();
        int sduLength = pkt->getByteLength(); // length without the SDU header
        pkt->insertAtFront(rlcSdu);

        if (fragmentInfo) {
            if (fragmentInfo->pkt != pkt)
                throw cRuntimeError("Packets are different");
            sduLength = fragmentInfo->size;
        }

        EV << NOW << " UmTxEntity::rlcPduMake - Next data chunk from the queue, sduSno[" << sduSequenceNumber
                << "], length[" << sduLength << "]"<< endl;

        if (pduLength >= sduLength)
        {
            EV << NOW << " UmTxEntity::rlcPduMake - Add " << sduLength << " bytes to the new SDU, sduSno[" << sduSequenceNumber << "]" << endl;

            // add the whole SDU
            if (fragmentInfo) {
                delete fragmentInfo;
                fragmentInfo = nullptr;
            }
            pduLength -= sduLength;
            len += sduLength;

            sduQueue_.pop();

            rlcPdu->pushSdu(pkt, sduLength);

            EV << NOW << " UmTxEntity::rlcPduMake - Pop data chunk from the queue, sduSno[" << sduSequenceNumber << "]" << endl;

            // now, the first SDU is a fragment
            firstIsFragment_ = false;

            EV << NOW << " UmTxEntity::rlcPduMake - The new SDU has length " << len << ", left space is " << pduLength << endl;
        }
        else
        {
            EV << NOW << " UmTxEntity::rlcPduMake - Add " << pduLength << " bytes to the new SDU, sduSno[" << sduSequenceNumber << "]" << endl;

            // add partial SDU

            len += pduLength;

            auto rlcSduDup = pkt->dup();
            if (fragmentInfo) {
                fragmentInfo->size -= pduLength;
                if (fragmentInfo->size < 0)
                    throw cRuntimeError("Fragmentation error");
            }
            else {
                fragmentInfo  = new FragmentInfo;
                fragmentInfo->pkt = pkt;
                fragmentInfo->size = sduLength - pduLength;
            }
            rlcPdu->pushSdu(rlcSduDup, pduLength);

            endFrag = true;

            // update SDU in the buffer
            int newLength = sduLength - pduLength;

            EV << NOW << " UmTxEntity::rlcPduMake - Data chunk in the queue is now " << newLength << " bytes, sduSno[" << sduSequenceNumber << "]" << endl;

            pduLength = 0;

            // now, the first SDU in the buffer is a fragment
            firstIsFragment_ = true;

            EV << NOW << " UmTxEntity::rlcPduMake - The new SDU has length " << len << ", left space is " << pduLength << endl;

        }
    }

    if (len == 0)
    {
        // send an empty message to notify the MAC that there is not enough space to send RLC PDU

        //rlcPdu->setControlInfo(flowControlInfo_->dup());
        rlcPdu->setChunkLength(inet::b(1)); // send only a bit, minimum size.
        *pktAux->addTagIfAbsent<FlowControlInfo>() = *flowControlInfo_;
        //rlcPdu->setByteLength(len);
    }
    else
    {
        // compute FI
        // the meaning of this field is specified in 3GPP TS 36.322
        FramingInfo fi = 0;
        unsigned short int mask;
        if (endFrag)
        {
            mask = 1;   // 01
            fi |= mask;
        }
        if (startFrag)
        {
            mask = 2;   // 10
            fi |= mask;
        }

        rlcPdu->setFramingInfo(fi);
        rlcPdu->setPduSequenceNumber(sno_++);
        *(pktAux->addTagIfAbsent<FlowControlInfo>()) = *flowControlInfo_;
        //rlcPdu->setControlInfo(flowControlInfo_->dup());
        rlcPdu->setChunkLength(inet::B(RLC_HEADER_UM + len)); // send only a bit, minimum size.
        //rlcPdu->setByteLength(RLC_HEADER_UM + len);  // add the header size
    }

    // send to MAC layer
    pktAux->insertAtFront(rlcPdu);
    EV << NOW << " UmTxEntity::rlcPduMake - send PDU " << rlcPdu->getPduSequenceNumber() << " with size " << pktAux->getByteLength() << " bytes to lower layer" << endl;
    lteRlc_->sendToLowerLayer(pktAux);

    // if incoming connection was halted
    if (notifyEmptyBuffer_ && sduQueue_.isEmpty())
    {
        notifyEmptyBuffer_ = false;

        // tell the RLC UM to resume packets for the new mode
        lteRlc_->resumeDownstreamInPackets(flowControlInfo_->getD2dRxPeerId());
    }
}

void UmTxEntity::removeDataFromQueue()
{
    EV << NOW << " UmTxEntity::removeDataFromQueue - removed SDU " << endl;

    // get the last packet...
    cPacket* pkt = sduQueue_.back();

    // ...and remove it
    cPacket* retPkt = sduQueue_.remove(pkt);
    delete retPkt;
}

void UmTxEntity::clearQueue()
{
    // empty buffer
    while (!sduQueue_.isEmpty())
        delete sduQueue_.pop();


    // reset variables except for sequence number
    firstIsFragment_ = false;
}

bool UmTxEntity::isHoldingDownstreamInPackets()
{
    return holdingDownstreamInPackets_;
}

void UmTxEntity::enqueHoldingPackets(cPacket* pkt)
{
    EV << NOW << " UmTxEntity::enqueHoldingPackets - storing new SDU into the holding buffer " << endl;
    sduHoldingQueue_.insert(pkt);
}


void UmTxEntity::resumeDownstreamInPackets()
{
    EV << NOW << " UmTxEntity::resumeDownstreamInPackets - resume buffering incoming downstream packets of the RLC entity associated to the new mode" << endl;

    holdingDownstreamInPackets_ = false;

    // move all SDUs in the holding buffer to the TX buffer
    while (!sduHoldingQueue_.isEmpty())
    {
        auto pktRlc = check_and_cast<inet::Packet *> (sduHoldingQueue_.front());
        auto rlcHeader = pktRlc->peekAtFront<LteRlcSdu>();
        //LteRlcSdu* rlcPkt = check_and_cast<LteRlcSdu*>(sduHoldingQueue_.front());
        sduHoldingQueue_.pop();

        // create a message so as to notify the MAC layer that the queue contains new data
        //LteRlcPdu* newDataPkt = new LteRlcPdu("newDataPkt");
        auto newDataPkt = inet::makeShared<LteRlcPduNewData>();
        // make a copy of the RLC SDU
        auto pktRlcdup = pktRlc->dup();
        pktRlcdup->insertAtFront(newDataPkt);
        //FlowControlInfo* lteInfo = check_and_cast<FlowControlInfo*>(rlcPkt->getControlInfo());
        // the MAC will only be interested in the size of this packet
        //newDataPkt->encapsulate(rlcPktDup);
        //newDataPkt->setControlInfo(lteInfo->dup());

        lteRlc_->sendToLowerLayer(pktRlcdup);

        // store the SDU in the TX buffer
        enque(pktRlc);
    }
}

void UmTxEntity::rlcHandleD2DModeSwitch(bool oldConnection, bool clearBuffer)
{
    if (oldConnection)
    {
        if (getNodeTypeById(ownerNodeId_) == ENODEB)
        {
            EV << NOW << " UmRxEntity::rlcHandleD2DModeSwitch - nothing to do on DL leg of IM flow" << endl;
            return;
        }

        if (clearBuffer)
        {
            EV << NOW << " UmTxEntity::rlcHandleD2DModeSwitch - clear TX buffer of the RLC entity associated to the old mode" << endl;
            clearQueue();
        }
        else
        {
            if (!sduQueue_.isEmpty())
            {
                EV << NOW << " UmTxEntity::rlcHandleD2DModeSwitch - check when TX buffer the RLC entity associated to the old mode becomes empty - queue length[" << sduQueue_.getLength() << "]" << endl;
                notifyEmptyBuffer_ = true;
            }
            else
            {
                EV << NOW << " UmTxEntity::rlcHandleD2DModeSwitch - TX buffer of the RLC entity associated to the old mode is already empty" << endl;
            }
        }
    }
    else
    {
        EV << " UmTxEntity::rlcHandleD2DModeSwitch - reset numbering of the RLC TX entity corresponding to the new mode" << endl;
        sno_ = 0;

        if (!clearBuffer)
        {
            if (lteRlc_->isEmptyingTxBuffer(flowControlInfo_->getD2dRxPeerId()))
            {
                // stop incoming connections, until
                EV << NOW << " UmTxEntity::rlcHandleD2DModeSwitch - halt incoming downstream connections of the RLC entity associated to the new mode" << endl;
                startHoldingDownstreamInPackets();
            }
        }
    }
}
