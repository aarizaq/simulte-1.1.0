// 
//                           SimuLTE
// 
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself, 
// and cannot be removed from it.
// 


package lte.corenetwork.nodes;

import inet.linklayer.ppp.Ppp;
import inet.networklayer.common.InterfaceTable;
import lte.corenetwork.lteip.LteIp;
import lte.corenetwork.lteip.InternetQueue;
import inet.transportlayer.udp.Udp;
import inet.transportlayer.tcp.Tcp;
import inet.common.MessageDispatcher;

//

// Node representing the Internet in the Lte network model.
// It has TCP/UDP transport layer and related applications.
// It contains an LteIp module to connect the INTERNET 
// node to the ENODEB LteIp peer.
//
module LteInternet
{
    parameters:

        @display("i=misc/globe;bgb=622,370");
    gates:
        //#inout peerLteIp; 

        inout peerLteIp[];  // gate connecting the node with an LteIp peer

    submodules:
        tn: MessageDispatcher {
            parameters:
                @display("p=299,103;b=180,5,,,,1");
        }
        // Lte IP module , and gates for connecting to transport stack
        lteIp: LteIp {
            @display("p=240,142");
            nodeType = "INTERNET";
            protocolMapping = "6:0,17:1";
        }
        tcp: Tcp {
            @display("p=34,40");
        }
        udp: Udp {
            @display("p=190,40");
        }

        mux: InternetMux {
            @display("p=222,232");
        }

        internetQueue: InternetQueue {
            @dynamic(true);
            @display("p=54,218");
        }
        interfaceTable: InterfaceTable {
            @display("p=45,121");
        }

    connections allowunconnected:

        udp.ipOut --> tn.in++;
        udp.ipIn <-- tn.out++;

        tcp.ipOut --> tn.in++;
        tcp.ipIn <-- tn.out++;


        //#
        //# Internal LTE stack to PHY and
        //# Lte IP connections
        //#
        //lteIp.transportOut[0] --> tcp.ipIn;
        //lteIp.transportIn[0] <-- tcp.ipOut;

        //lteIp.transportOut[1] --> udp.ipIn;
        //lteIp.transportIn[1] <-- udp.ipOut;

        lteIp.transportOut --> tn.in++;
        lteIp.transportIn <-- tn.out++;


        lteIp.ifOut --> mux.inMux;

        //# 
        //# External connection to peer Lte Ip 
        //#

        lteIp.ifIn <-- mux.outMux;

}

simple InternetMux
{
    parameters:
        @display("i=block/dispatch");

    gates:
        input inMux;            // from lteIp
        output outMux;         // to lteIP

        //# externals

        input inExt[];                // from external (ltePeerIp)
        output outExt[];            // to internet Queues
}
