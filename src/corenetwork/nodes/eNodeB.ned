// 
//                           SimuLTE
// 
// This file is part of a software released under the license included in file
// "license.pdf". This license can be also found at http://www.ltesimulator.com/
// The above file and the present reference are part of the software itself, 
// and cannot be removed from it.
// 


package lte.corenetwork.nodes;

import inet.linklayer.ppp.PppInterface;
import inet.mobility.static.StationaryMobility;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.ipv4.Ipv4RoutingTable;
import inet.networklayer.contract.IRoutingTable;
import inet.networklayer.contract.INetworkLayer;
import inet.applications.contract.IApp;
import inet.transportlayer.contract.ISctp;
import inet.transportlayer.contract.ITcp;
import inet.transportlayer.contract.IUdp;

//import inet.applications.contract.IUDPApp;
//import inet.applications.contract.ITCPApp;
//import inet.applications.contract.ISCTPApp;
import inet.transportlayer.udp.Udp;
import inet.transportlayer.tcp.Tcp;
import inet.transportlayer.sctp.Sctp;
import lte.x2.LteX2App;
import lte.stack.ILteNic;
import lte.corenetwork.lteCellInfo.LteCellInfo;
import lte.epc.gtp.GtpUserSimplified;
import lte.epc.TrafficFlowFilterSimplified;
import lte.epc.gtp.GtpUserX2;
import inet.common.MessageDispatcher;

// 
// eNodeB Module
//
module eNodeB
{
    parameters:
        @networkNode();
        @display("i=device/antennatower;bgb=814,437");
        //# Num apps
        int numApps = default(0);
        int numX2Apps = default(0);   // no of X2 apps. Specify the app types in INI file with x2App[0..1].typename="X2AppClient" syntax
        
        //# Node specs
        string nodeType = "ENODEB";        // DO NOT CHANGE
        int macNodeId = default(0); // TODO: this is not a real parameter
        int macCellId = default(0);  // TODO: this is not a real parameter
        double txPower @unit(mw) = default(100mw);
        string nicType = default("LteNicEnb");

        bool hasUdp = default(firstAvailableOrEmpty("Udp") != "");
        bool hasTcp = default(firstAvailableOrEmpty("Tcp", "TcpLwip", "TcpNsc") != "");
        bool hasSctp = default(numX2Apps != 0);

        //# Network Layer specs
        //string networkLayerType = default("Ipv4NetworkLayer");
        //string routingTableType = default("Ipv4RoutingTable");
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        //*.routingTableModule = default(routingTableType != "" ? absPath(".routingTable") : "");
        *.routingTableModule = default("^.ipv4.routingTable");

        //int numTcpApps = default(0);  // no of TCP apps. Specify the app types in INI file with tcpApp[0..1].typename="TCPEchoApp" syntax
       // int numUdpApps = default(0);  // no of UDP apps. Specify the app types in INI file with udpApp[0..1].typename="UDPVideoStreamCli" syntax
        

    gates:
        inout ppp;                // connection to the Core Network
        input radioIn @directIn;  // connection to the radio interface
        inout x2[];               // connection to the X2 interface

    submodules:

        interfaceTable: InterfaceTable {
            @display("p=50,75;is=s");
        }
        // TODO: should not include a routing table
        // routing table
        //routingTable: <routingTableType> like IRoutingTable if routingTableType != "" {
        //    parameters:
        //        @display("p=50,125;is=s");
        //}
        mobility: StationaryMobility {
            @display("p=50,175;is=s");
        }
        cellInfo: LteCellInfo {
            @display("p=50,225;is=s");
        }

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

        //#
        //# wlanLteNic modules
        //#
        wlanLteNic: <nicType> like ILteNic {
            nodeType = nodeType;
            //d2dCapable = d2dCapable;
            @display("p=255.625,374.23502");
        }

        pppIf: PppInterface {
            @display("p=501,363");
        }

        x2ppp[sizeof(x2)]: PppInterface {
            @display("p=683,363");
        }

        //#
        //# Network layer module
        //#

        ipv4: <default("Ipv4NetworkLayer")> like INetworkLayer {
            parameters:
                @display("p=435.585,240.2875;q=queue");
        }

       // networkLayer: <networkLayerType> like INetworkLayer {
       //     parameters:
       //         @display("p=501,269;q=queue");
       // }

        //#
        //# Transport layer modules and corresponding application modules
        //#
        app[numApps]: <> like IApp {
            parameters:
                @display("p=551.1275,47.035,row,150");
        }

        udp: <default(firstAvailableOrEmpty("Udp"))> like IUdp if hasUdp {
            parameters:
                @display("p=429.45,151.33");
        }
        tcp: <default(firstAvailableOrEmpty("Tcp", "TcpLwip", "TcpNsc"))> like ITcp if hasTcp {
            parameters:
                @display("p=531.7,151.33");
        }
        sctp: <default(firstAvailableOrEmpty("Sctp"))> like ISctp if hasSctp {
            parameters:
                @display("p=608.3875,151.33");
        }

		// =============== UDP ============== //
       // udpApp[numUdpApps]: <> like IUDPApp {
       //     @display("p=555,48,row");
       // }
       // udp: Udp {
       //     @display("p=501,148");
       // }

        // =============== TCP ============== //
       // tcpApp[numTcpApps]: <> like ITCPApp {
       //     @display("p=683,48,row");
       // }
       // tcp: Tcp if numTcpApps>0 {
       //     @display("p=683,148");
       // }

	    // =============== X2AP ============== //
        x2App[numX2Apps]: LteX2App {
            @display("p=155,48,row");
        }
       // sctp: Sctp {
       //     @display("p=275,148");
       // }

        //# 
        //# Simplified EPC
        //# 
        trafficFlowFilter: TrafficFlowFilterSimplified {
            ownerType = nodeType;
            @display("p=295.5025,143.15001");
        }
        gtpUser: GtpUserSimplified {
            @display("p=435.585,42.945");
        }

        gtpUserX2: GtpUserX2 {
            @display("p=268.9175,18.405");
        }

    connections allowunconnected:


        udp.ipOut --> tn.in++ if hasUdp;
        udp.ipIn <-- tn.out++ if hasUdp;

        tcp.ipOut --> tn.in++ if hasTcp;
        tcp.ipIn <-- tn.out++ if hasTcp;

        sctp.ipOut --> tn.in++ if hasSctp;
        tn.out++ --> sctp.ipIn if hasSctp;

        tn.out++ --> ipv4.transportIn;
        tn.in++ <-- ipv4.transportOut;

        at.out++ --> udp.appIn if hasUdp;
        at.in++ <-- udp.appOut if hasUdp;

        at.out++ --> tcp.appIn if hasTcp;
        at.in++ <-- tcp.appOut if hasTcp;

        at.out++ --> sctp.appIn if hasSctp;
        at.in++ <-- sctp.appOut if hasSctp;

        ipv4.ifIn <-- nl.out++;
        ipv4.ifOut --> nl.in++;

        //#
        //# LTE stack to PHY and network layer
        //#        
        wlanLteNic.radioIn <-- radioIn;
        wlanLteNic.upperLayerOut --> trafficFlowFilter.internetFilterGateIn;
        wlanLteNic.upperLayerIn <-- gtpUser.pppGate;
        trafficFlowFilter.gtpUserGateOut --> gtpUser.trafficFlowFilterGate;
        //gtpUser.udpOut --> udp.appIn++;
        //gtpUser.udpIn <-- udp.appOut++;

        gtpUser.socketOut --> at.in++;
        gtpUser.socketIn <-- at.out++;

        for i=0..numApps-1 {
            app[i].socketOut --> at.in++;
            app[i].socketIn <-- at.out++;
        }



        //#
        //# PPP interface to network layer connections
        //#    
        pppIf.phys <--> ppp;
        //pppIf.upperLayerOut --> networkLayer.ifIn++;
        //pppIf.upperLayerIn <-- networkLayer.ifOut++;

        pppIf.upperLayerOut --> nl.in++;
        pppIf.upperLayerIn <-- nl.out++;



        gtpUserX2.lteStackOut --> wlanLteNic.x2$i++;
        gtpUserX2.lteStackIn <-- wlanLteNic.x2$o++;
        //gtpUserX2.udpOut --> udp.appIn++;
        //gtpUserX2.udpIn <-- udp.appOut++;
        gtpUserX2.socketOut --> at.in++;
        gtpUserX2.socketIn <-- at.out++;


        //#
        //# X2 interface to network layer connections
        //#    
        for i=0..sizeof(x2)-1 {
            x2ppp[i].phys <--> x2[i];
            //x2ppp[i].upperLayerOut --> networkLayer.ifIn++;
            //x2ppp[i].upperLayerIn <-- networkLayer.ifOut++;
            x2ppp[i].upperLayerOut --> nl.in++;
            x2ppp[i].upperLayerIn <-- nl.out++;
        }

		//# 
		//# Apps to transport layer connections
		//#
        //for i=0..numTcpApps-1 {
        //    tcpApp[i].tcpOut --> tcp.appIn++;
        //    tcpApp[i].tcpIn <-- tcp.appOut++;
        //}
        //for i=0..numUdpApps-1 {
        //    udpApp[i].udpOut --> udp.appIn++;
        //    udpApp[i].udpIn <-- udp.appOut++;
        // }
        for i=0..numX2Apps-1 {
            //x2App[i].sctpOut[1] --> sctp.from_appl++;
            //x2App[i].sctpIn[1] <-- sctp.to_appl++;
            x2App[i].socketOut[0] --> at.in++;
            x2App[i].socketIn[0] <-- at.out++;

            x2App[i].socketOut[1] --> at.in++;
            x2App[i].socketIn[1] <-- at.out++;

            x2App[i].x2ManagerIn <-- wlanLteNic.x2$o++;
            x2App[i].x2ManagerOut --> wlanLteNic.x2$i++;
        }

        //# 
        //# Transport layer to network layer connections
        //#
        //udp.ipOut --> networkLayer.transportIn++;
        //udp.ipIn <-- networkLayer.transportOut++;
        //tcp.ipOut --> networkLayer.transportIn++ if numTcpApps>0;
        //tcp.ipIn <-- networkLayer.transportOut++ if numTcpApps>0;
        //sctp.to_ip --> networkLayer.transportIn++ if numX2Apps>0;
        //sctp.from_ip <-- networkLayer.transportOut++ if numX2Apps>0;


        at.out++ --> tn.in++;
        at.in++ <-- tn.out++;
        tn.out++ --> nl.in++;
        tn.in++ <-- nl.out++;
}
