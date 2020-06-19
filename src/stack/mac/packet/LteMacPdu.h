//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#ifndef _LTE_LTEMACPDU_H_
#define _LTE_LTEMACPDU_H_

#include "stack/mac/packet/LteMacPdu_m.h"
#include "common/LteCommon.h"
#include "common/LteControlInfo.h"

/**
 * @class LteMacPdu
 * @brief Lte MAC Pdu
 *
 * Class derived from base class contained
 * in msg declaration: adds the sdu and control elements list
 * TODO: Add Control Elements
 */
class LteMacPdu : public LteMacPdu_Base
{
  private:
    void copy(const LteMacPdu& other);
  protected:
    /// List Of MAC SDUs
    cPacketQueue* sduList_;

    /// List of MAC CEs
    MacControlElementsList ceList_;

    /// Length of the PDU
    inet::int64 macPduLength_;

    /**
     * ID of the MAC PDU
     * at the creation, the ID is set equal to cMessage::msgid. When the message is duplicated,
     * the new message gets a different msgid. However, we need that multiple copies of the
     * same PDU have the same PDU ID (for example, multiple copies of a broadcast transmission)
     */
    inet::int64 macPduId_;

    inet::int64 getByteLength() const
    {
        return macPduLength_ + getHeaderLength();
    }

    inet::int64 getBitLength() const
    {
        return (getByteLength() * 8);
    }

  public:

    /**
     * Constructor
     */
    LteMacPdu();

    /*
     * Copy constructors
     */

    LteMacPdu(const LteMacPdu& other);

    LteMacPdu& operator=(const LteMacPdu& other);

    virtual LteMacPdu *dup() const override;

    /**
     * info() prints a one line description of this object
     */
    std::string str() const override;

    /**
     * Destructor
     */
    virtual ~LteMacPdu();

    virtual void setSduArraySize(size_t size) override
    {
        ASSERT(false);
    }

    virtual size_t getSduArraySize() const override;

    virtual const inet::Packet& getSdu(size_t k) const override;

    virtual void setSdu(size_t k, const Packet& sdu) override
    {
        ASSERT(false);
    }

    virtual void insertSdu(const inet::Packet& sdu) override
    {
        ASSERT(false);
    }

    virtual void insertSdu(size_t k, const inet::Packet& sdu) override
    {
        ASSERT(false);
    }

    virtual void eraseSdu(size_t k) override
    {
        ASSERT(false);
    }
    /**
     * pushSdu() gets ownership of the packet
     * and stores it inside the mac sdu list
     * in back position
     *
     * @param pkt packet to store
     */
    virtual void pushSdu(Packet* pkt);
    /**
     * popSdu() pops a packet from front of
     * the sdu list and drops ownership before
     * returning it
     *
     * @return popped packet
     */
    virtual Packet* popSdu();

    /**
     * hasSdu() verifies if there are other
     * SDUs inside the sdu list
     *
     * @return true if list is not empty, false otherwise
     */
    virtual bool hasSdu() const;

    /**
     * pushCe() stores a CE inside the
     * MAC CE list in back position
     *
     * @param pkt CE to store
     */
    virtual void pushCe(MacControlElement* ce);

    /**
     * popCe() pops a CE from front of
     * the CE list and returns it
     *
     * @return popped CE
     */
    virtual MacControlElement* popCe();

    /**
     * hasCe() verifies if there are other
     * CEs inside the ce list
     *
     * @return true if list is not empty, false otherwise
     */
    virtual bool hasCe() const;
    /**
     *
     */

    long getId() const;

    virtual void setHeaderLength(unsigned int headerLength) override;

};

#endif

