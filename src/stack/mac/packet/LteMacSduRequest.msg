//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

//
// This is the message that asks the RLC layer for a new MAC SDU.
//
import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

class LteMacSduRequest extends inet::FieldsChunk
{
    @customize(true);
    unsigned int sduSize;       // size of the requested SDU
    chunkLength = inet::B(1); // TODO: size 0
}