//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#include "stack/mac/buffer/harq_d2d/LteHarqProcessRxD2D.h"
#include "stack/mac/layer/LteMacBase.h"
#include "common/LteControlInfo.h"
#include "stack/mac/packet/LteHarqFeedback_m.h"
#include "stack/mac/packet/LteMacPdu.h"

LteHarqProcessRxD2D::LteHarqProcessRxD2D(unsigned char acid, LteMacBase *owner)
    : LteHarqProcessRx(acid, owner)
{
}

LteHarqProcessRxD2D::~LteHarqProcessRxD2D()
{
}

//LteHarqFeedback *LteHarqProcessRxD2D::createFeedback(Codeword cw)
Packet *LteHarqProcessRxD2D::createFeedback(Codeword cw)
{
    if (!isEvaluated(cw))
        throw cRuntimeError("Cannot send feedback for a pdu not in EVALUATING state");



    Packet *pkt = nullptr;
    //LteHarqFeedback *fb;

    //UserControlInfo *pduInfo = check_and_cast<UserControlInfo *>(pdu_.at(cw)->getControlInfo());
    auto pduInfo = (pdu_.at(cw)->getTag<UserControlInfo>());

    if (pduInfo->getDirection() == D2D_MULTI)
    {
        // if the PDU belongs to a multicast connection, then do not create feedback
        // fb = nullptr;
    }
    else
    {
        // TODO: Change LteHarqFeedback from chunk to tag,
        pkt = new Packet();
        auto fb = makeShared<LteHarqFeedback>();
        //fb = new LteHarqFeedback();
        fb->setAcid(acid_);
        fb->setCw(cw);
        fb->setResult(result_.at(cw));
        fb->setFbMacPduId(pdu_.at(cw)->getId());
        //fb->setByteLength(0);
        fb->setChunkLength(b(1));
        //UserControlInfo *fbInfo = new UserControlInfo();
        pkt->addTagIfAbsent<UserControlInfo>()->setSourceId(pduInfo->getDestId());
        pkt->addTagIfAbsent<UserControlInfo>()->setDestId(pduInfo->getSourceId());
        pkt->addTagIfAbsent<UserControlInfo>()->setFrameType(HARQPKT);
        pkt->insertAtFront(fb);
        //fb->setControlInfo(fbInfo);
    }

    if (!result_.at(cw))
    {
        if (pduInfo->getDirection() == D2D_MULTI)
        {
            // if the PDU belongs to a multicast/broadcast connection, then reset the codeword, since there will be no retransmission
            EV << NOW << " LteHarqProcessRxD2D::createFeedback - pdu for cw " << cw << " belonged to a multicast/broadcast connection. Resetting cw " << endl;
            delete pdu_.at(cw);
            pdu_.at(cw) = NULL;
            resetCodeword(cw);
        }
        else
        {
            // NACK will be sent
            status_.at(cw) = RXHARQ_PDU_CORRUPTED;

            EV << "LteHarqProcessRx::createFeedback - tx number " << (unsigned int)transmissions_ << endl;
            if (transmissions_ == (maxHarqRtx_ + 1))
            {
                EV << NOW << " LteHarqProcessRxD2D::createFeedback - max number of tx reached for cw " << cw << ". Resetting cw" << endl;

                // purge PDU
                purgeCorruptedPdu(cw);
                resetCodeword(cw);
            }
        }
    }
    else
    {
        status_.at(cw) = RXHARQ_PDU_CORRECT;
    }

    return pkt;
}

//LteHarqFeedbackMirror* LteHarqProcessRxD2D::createFeedbackMirror(Codeword cw)
Packet* LteHarqProcessRxD2D::createFeedbackMirror(Codeword cw)
{
    if (!isEvaluated(cw))
        throw cRuntimeError("Cannot send feedback for a pdu not in EVALUATING state");

    auto pduInfo = pdu_.at(cw)->getTag<UserControlInfo>();

    //UserControlInfo *pduInfo = check_and_cast<UserControlInfo *>(pdu_.at(cw)->getControlInfo());
    //LteHarqFeedbackMirror *fb;
    Packet *pkt = nullptr;

    if (pduInfo->getDirection() == D2D_MULTI)
    {
        // if the PDU belongs to a multicast connection, then do not create feedback
        //fb = NULL;
    }
    else
    {
        // TODO: Change LteHarqFeedbackMirror from chunk to tag,
        pkt = new Packet();
        auto fb = makeShared<LteHarqFeedbackMirror>();
        fb->setAcid(acid_);
        fb->setCw(cw);
        fb->setResult(result_.at(cw));
        fb->setFbMacPduId(pdu_.at(cw)->getId());
        fb->setChunkLength(b(1)); // TODO: should be 0
        fb->setPduLength(pdu_.at(cw)->getByteLength());
        fb->setD2dSenderId(pduInfo->getSourceId());
        fb->setD2dReceiverId(pduInfo->getDestId());
        pkt->insertAtFront(fb);
        //UserControlInfo *fbInfo = new UserControlInfo();
        pkt->addTagIfAbsent<UserControlInfo>()->setSourceId(pduInfo->getDestId());
        pkt->addTagIfAbsent<UserControlInfo>()->setDestId(macOwner_->getMacCellId());
        pkt->addTagIfAbsent<UserControlInfo>()->setFrameType(HARQPKT);
        // fb->setControlInfo(fbInfo);
    }
    return pkt;
}
