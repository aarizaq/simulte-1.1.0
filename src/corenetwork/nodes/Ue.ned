// 
//                           SimuLTE
// 
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself, 
// and cannot be removed from it.
// 


package lte.corenetwork.nodes;

import inet.applications.contract.IApp;
//import inet.applications.contract.ITCPApp;
//import inet.applications.contract.IUDPApp;
import inet.mobility.contract.IMobility;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.contract.IRoutingTable;
import inet.networklayer.contract.INetworkLayer;
import inet.transportlayer.tcp.Tcp;
import inet.transportlayer.udp.Udp;
import lte.stack.ILteNic;
import inet.common.MessageDispatcher;
// 
// User Equipment Module
//
module Ue
{
    parameters:
        @networkNode();
        @display("i=device/pocketpc;bgb=667.116,517.82404");

        //# Mobility
        string mobilityType = default("StationaryMobility");

        //# Apps
        int numApps = default(0);
        //int numTcpApps = default(0);  // no of TCP apps. Specify the app types in INI file with tcpApp[0..1].typename="TCPEchoApp" syntax
        //int numUdpApps = default(0);  // no of UDP apps. Specify the app types in INI file with udpApp[0..1].typename="UDPVideoStreamCli" syntax

        //# Node specs
        string nodeType = "UE";  // DO NOT CHANGE
        int masterId;
        int macNodeId = default(0); // TODO: this is not a real parameter
        int macCellId = default(0); // TODO: this is not a real parameter

        //# D2D capability
        string nicType = default("LteNicUe");

        //# Network Layer specs
        string networkLayerType = default("Ipv4NetworkLayer");
        //string routingTableType = default("IPv4RoutingTable");
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        *.routingTableModule = default(absPath(".routingTable"));

    gates:
        input radioIn @directIn;     // connection to master

    submodules:
        at: MessageDispatcher {
            parameters:
                @display("p=500,108;b=180,5,,,,1");
        }
        tn: MessageDispatcher {
            parameters:
                @display("p=500,202;b=180,5,,,,1");
        }
        nl: MessageDispatcher {
            parameters:
                @display("p=500,286;b=180,5,,,,1");
        }

        interfaceTable: InterfaceTable {
            @display("p=50,75;is=s");
        }
        // routing table
        // routingTable: <routingTableType> like IRoutingTable if routingTableType != "" {
        //     parameters:
        //         @display("p=50,125;is=s");
        // }
        app[numApps]: <> like IApp {
            parameters:
                @display("p=551.1275,47.035,row,150");
        }

        mobility: <mobilityType> like IMobility {
            @display("p=50,175;is=s");
        }
        // tcpApp[numTcpApps]: <> like ITCPApp {
        //     @display("p=175,50,row");
        // }
        tcp: Tcp {
            @display("p=175,150");
        }
        // udpApp[numUdpApps]: <> like IUDPApp {
        //     @display("p=325,50,row");
        // }
        udp: Udp {
            @display("p=325,150");
        }
        wlanLteNic: <nicType> like ILteNic {
            nodeType = nodeType;
            //d2dCapable = d2dCapable;
            @display("p=250,407");
        }
        // network layer
        ipv4: <default("Ipv4NetworkLayer")> like INetworkLayer {
            parameters:
                @display("p=435.585,240.2875;q=queue");
        }
    connections allowunconnected:
        //# 
        //# Internal TCP/UDP applications connections with IP stack
        //#
        at.out++ --> tn.in++;
        at.in++ <-- tn.out++;
        tn.out++ --> nl.in++;
        tn.in++ <-- nl.out++;

        udp.ipOut --> tn.in++;
        udp.ipIn <-- tn.out++;

        tcp.ipOut --> tn.in++;
        tcp.ipIn <-- tn.out++;

        tn.out++ --> ipv4.transportIn;
        tn.in++ <-- ipv4.transportOut;

        at.out++ --> udp.appIn;
        at.in++ <-- udp.appOut;

        at.out++ --> tcp.appIn;
        at.in++ <-- tcp.appOut;

        for i=0..numApps-1 {
            app[i].socketOut --> at.in++;
            app[i].socketIn <-- at.out++;
        }
        
        //for i=0..numTcpApps-1 {
        //    tcpApp[i].tcpOut --> tcp.appIn++;
        //    tcpApp[i].tcpIn <-- tcp.appOut++;
        //}

        //tcp.ipOut --> ipv4.transportIn++ if numTcpApps>0;
        //tcp.ipIn <-- ipv4.transportOut++ if numTcpApps>0;

        //for i=0..numUdpApps-1 {
        //    udpApp[i].udpOut --> udp.appIn++;
        //    udpApp[i].udpIn <-- udp.appOut++;
        //}

        //udp.ipOut --> ipv4.transportIn++ if numUdpApps>0;
        //udp.ipIn <-- ipv4.transportOut++ if numUdpApps>0;
        ipv4.ifIn <-- nl.out++;
        ipv4.ifOut --> nl.in++;

        wlanLteNic.radioIn <-- radioIn;

        //ipv4.ifOut++ --> wlanLteNic.upperLayerIn;
        //ipv4.ifIn++ <-- wlanLteNic.upperLayerOut;
        nl.out++ --> wlanLteNic.upperLayerIn;
        nl.in++ <-- wlanLteNic.upperLayerOut;
}
