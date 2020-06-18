//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#ifndef LTERLCSDU_H_
#define LTERLCSDU_H_

#include "stack/rlc/packet/LteRlcSdu_m.h"
/*
#include "common/LteControlInfo.h"

class LteRlcSdu : public LteRlcSdu_Base
{

  private:
    void copy(const LteRlcSdu& other)
    {
        this->snoMainPacket = other.snoMainPacket;

        // copy the attached control info, if any
        if (other.getControlInfo() != NULL)
        {
            FlowControlInfo* info = check_and_cast<FlowControlInfo*>(other.getControlInfo());
            FlowControlInfo* info_dup = info->dup();
            this->setControlInfo(info_dup);
        }
    }

  public:
    LteRlcSdu(const char *name=NULL, int kind=0) : LteRlcSdu_Base(name,kind) {}
    LteRlcSdu(const LteRlcSdu& other) : LteRlcSdu_Base(other) {copy(other);}
    LteRlcSdu& operator=(const LteRlcSdu& other) {if (this==&other) return *this; LteRlcSdu_Base::operator=(other); copy(other); return *this;}
    virtual LteRlcSdu *dup() const {return new LteRlcSdu(*this);}
};

Register_Class(LteRlcSdu);
*/
#endif /* LTERLCSDU_H_ */
