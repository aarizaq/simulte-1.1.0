//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//

#ifndef _LTE_VODUDPSRV_H_
#define _LTE_VODUDPSRV_H_

#include <platdep/sockets.h>
#include <omnetpp.h>
#include <fstream>
#include "apps/vod/VoDUDPStruct.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "apps/vod/VoDPacket_m.h"
#include "apps/vod/M1Message_m.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/applications/base/ApplicationBase.h"

using namespace std;
using namespace inet;

class VoDUDPServer : public inet::ApplicationBase, public inet::UdpSocket::ICallback
{
  protected:
    UdpSocket socket;
    /* Server parameters */

    int serverPort;
    ifstream infile;
    string inputFileName;
    int fps;
    string traceType;
    fstream outfile;
    double TIME_SLOT;

    const char * clientsIP;
    int clientsPort;
    double clientsStartStreamTime;
    const char * clientsReqTime;

    std::vector<string> vclientsIP;

    std::vector<int> vclientsPort;
    std::vector<double> vclientsStartStreamTime;
    std::vector<double> vclientsReqTime;
    std::vector<L3Address> clientAddr;

    /* Statistics */

    unsigned int numStreams;  // number of video streams served
    unsigned long numPkSent;  // total number of packets sent

    struct tracerec
    {
        uint32_t trec_time;
        uint32_t trec_size;
    };
    struct svcPacket
    {
        int tid;
        int lid;
        int qid;
        int length;
        int frameNumber;
        int timestamp;
        int currentFrame;
        string memoryAdd;
        string isDiscardable;
        string isTruncatable;
        string isControl;
        string frameType;
        long int index;
    };
    unsigned int nrec_;

    tracerec* trace_;

    std::vector<svcPacket> svcTrace_;

  public:
    VoDUDPServer();
    virtual ~VoDUDPServer();

  protected:
    virtual void handleStartOperation(LifecycleOperation *operation) override {};
    virtual void handleStopOperation(LifecycleOperation *operation) override {};
    virtual void handleCrashOperation(LifecycleOperation *operation) override {};


    virtual void initialize(int stage) override;
    virtual void finish() override;
    //virtual void handleMessage(cMessage*);
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override {};
    virtual void handleNS2Message(cMessage*);
    virtual void handleSVCMessage(cMessage*);
};

#endif
