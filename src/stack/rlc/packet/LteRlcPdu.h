//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#ifndef LTERLCPDU_H_
#define LTERLCPDU_H_

#include "stack/rlc/packet/LteRlcPdu_m.h"
/*
#include "common/LteControlInfo.h"
class LteRlcPdu : public LteRlcPdu_Base {
   private:

     void copy(const LteRlcPdu& other)
     {
         this->totalFragments = other.totalFragments;
         this->snoFragment = other.snoFragment;
         this->snoMainPacket = other.snoMainPacket;

         // copy the attached control info, if any
     }

   public:
     LteRlcPdu() : LteRlcPdu_Base() {}
     LteRlcPdu(const LteRlcPdu& other) : LteRlcPdu_Base(other) {copy(other);}

     LteRlcPdu& operator=(const LteRlcPdu& other)
     {
         if (this==&other)
             return *this;
         LteRlcPdu_Base::operator=(other);
         copy(other);
         return *this;
     }
     virtual LteRlcPdu *dup() const {return new LteRlcPdu(*this);}
     virtual ~LteRlcPdu(){
     }
};

Register_Class(LteRlcPdu);
*/
#endif /* LTERLCPDU_H_ */
