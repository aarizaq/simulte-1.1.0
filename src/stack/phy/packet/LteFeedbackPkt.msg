//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//
import inet.common.INETDefs;
import inet.common.packet.chunk.Chunk;

class LteFeedbackPkt extends inet::FieldsChunk {
    chunkLength = inet::B(1); // TODO: size 0
    @customize(true);  // see the generated C++ header for more info
}
