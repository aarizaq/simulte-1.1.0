//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#ifndef _LTE_X2COMPPROPORTIONALREPLYIE_H_
#define _LTE_X2COMPPROPORTIONALREPLYIE_H_

#include "stack/compManager/X2CompReplyIE.h"

enum CompRbStatus
{
    AVAILABLE_RB, NOT_AVAILABLE_RB
};

//
// X2CompProportionalReplyIE
//
class X2CompProportionalReplyIE : public X2CompReplyIE
{
  protected:

    // for each block, it denotes whether the eNB can use that block
    std::vector<CompRbStatus> allowedBlocksMap_;

  public:
    X2CompProportionalReplyIE()
    {
        length_ = 0;
    }
    X2CompProportionalReplyIE(const X2CompProportionalReplyIE& other) :
        X2CompReplyIE()
    {
        operator=(other);
    }

    X2CompProportionalReplyIE& operator=(const X2CompProportionalReplyIE& other)
    {
        if (&other == this)
            return *this;
        allowedBlocksMap_ = other.allowedBlocksMap_;
        X2InformationElement::operator=(other);
        return *this;
    }
    virtual X2CompProportionalReplyIE *dup() const
    {
        return new X2CompProportionalReplyIE(*this);
    }
    virtual ~X2CompProportionalReplyIE() {}

    // getter/setter methods
    void setAllowedBlocksMap(std::vector<CompRbStatus>& map)
    {
        allowedBlocksMap_ = map;
        length_ += allowedBlocksMap_.size() * sizeof(CompRbStatus);
    }
    std::vector<CompRbStatus> getAllowedBlocksMap() const { return allowedBlocksMap_; }
};

#endif