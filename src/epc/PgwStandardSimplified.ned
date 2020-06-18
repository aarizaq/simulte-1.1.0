//
//                           SimuLTE
//
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself,
// and cannot be removed from it.
//
package lte.epc;

import lte.epc.gtp.GtpUser;
import lte.epc.gtp.GtpUserSimplified;
import inet.linklayer.ppp.PppInterface;
import inet.node.base.TransportLayerNodeBase;

import lte.epc.TrafficFlowFilterSimplified;
import inet.transportlayer.udp.Udp;
import inet.common.MessageDispatcher;

//
// Implements an EPC PDN Gateway (P-GW)
// 
// 
// Implements a simplified version of the EPC PDN Gateway (P-GW)
//
module PgwStandardSimplified extends TransportLayerNodeBase
{
    parameters:
        string nodeType; // must be one between ENB and PGW
        //@display("bgb=1111.11,702.24;i=device/mainframe");               
        @display("i=device/mainframe");
    gates:
        inout filterGate @labels(PPPFrame-conn);

    submodules:
        trafficFlowFilter: TrafficFlowFilterSimplified {
            ownerType = nodeType;
            @display("p=854.7,62.37");
        }
        pppInterface: PppInterface {
            @display("p=963.27,495.495");
        }
        gtp_user: GtpUserSimplified {
            @display("p=694.704,90.368004");
        }
        at: MessageDispatcher {
            parameters:
                @display("p=750,146;b=1000,5,,,,1");
        }
    connections:
        gtp_user.socketOut --> at.in++;
        gtp_user.socketIn <-- at.out++;

        at.out++ --> udp.appIn if hasUdp;
        at.in++ <-- udp.appOut if hasUdp;

        at.out++ --> tcp.appIn if hasTcp;
        at.in++ <-- tcp.appOut if hasTcp;

        at.out++ --> sctp.appIn if hasSctp;
        at.in++ <-- sctp.appOut if hasSctp;

        pppInterface.upperLayerOut --> trafficFlowFilter.internetFilterGateIn;
        pppInterface.upperLayerIn <-- gtp_user.pppGate;
        pppInterface.phys <--> filterGate;

        trafficFlowFilter.gtpUserGateOut --> gtp_user.trafficFlowFilterGate;
}
