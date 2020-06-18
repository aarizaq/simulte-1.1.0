//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

//
// This is the X2 message flowing through X2 interface.
// It contains a vector of Information Elements (see LteX2Message.h),
// the total length of the message and the type of the message
//

import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

class LteX2Message extends inet::FieldsChunk
{
    @customize(true);
    // TODO: size 0
    chunkLength = inet::B(1);
    unsigned int sourceId;
    unsigned int destinationId;
}

class X2InformationElement extends inet::FieldsChunk
{
    @customize(true);
}