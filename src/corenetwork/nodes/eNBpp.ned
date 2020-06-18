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
import inet.transportlayer.udp.Udp;
import lte.corenetwork.lteCellInfo.LteCellInfo;
import lte.epc.TrafficFlowFilter;
import lte.epc.gtp.GtpUser;
import lte.stack.LteNicEnb;
import inet.common.MessageDispatcher;

//
// eNodeB module with support for EPC (GTP-U, etc).
//
module eNBpp
{
    parameters:
        @networkNode();
        @display("i=device/antennatower;bgb=751,360");
        bool hasUdp = default(firstAvailableOrEmpty("Udp") != "");
        bool hasTcp = default(firstAvailableOrEmpty("Tcp", "TcpLwip", "TcpNsc") != "");
        //# Node specs
        string nodeType = "ENODEB";        // DO NOT CHANGE
        int macNodeId = default(0); // TODO: this is not a real parameter
        int macCellId = default(0);  // TODO: this is not a real parameter
        double txPower @unit(mw) = default(100mw);
        string nicType = default("LteNicEnb");

        //# Network Layer specs
        //string networkLayerType = default("Ipv4NetworkLayer");
        //string routingTableType = default("Ipv4RoutingTable");
        *.interfaceTableModule = default("^.interfaceTable");
        *.routingTableModule = default("^.ipv4.routingTable");

    gates:
        inout ppp;     // connection to LteIP peer
        input radioIn @directIn;  // connection to slave

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
        // TODO: should not include a routing table
        // routing table
        //routingTable: <routingTableType> like IRoutingTable if routingTableType != "" {
        //    parameters:
        //        @display("p=50,125;is=s");
        //}
        mobility: StationaryMobility {
            @display("p=50,175;is=s");
        }
        // 
        cellInfo: LteCellInfo {
            @display("p=50,225;is=s");
        }
        // NOTE: instance must be named "wlanLteNic"
        wlanLteNic: LteNicEnb {
            nodeType = nodeType;
            @display("p=200,311");
        }
        pppIf: PppInterface {
            @display("p=500,324");
        }
        gtp_user: GtpUser {
            @display("p=381,72");
        }
        trafficFlowFilter: TrafficFlowFilter {
            ownerType = nodeType;
            @display("p=200,144");
        }
        udp: Udp {
            @display("p=539,155");
        }
        // network layer
        ipv4: <default("Ipv4NetworkLayer")> like INetworkLayer {
            parameters:
                @display("p=435.585,240.2875;q=queue");
        }
    connections allowunconnected:
        //#
        //# Internal LTE stack to PHY and
        //# Lte IP connections
        //#


        //#
        //# External connection to peer Lte Ip and air interface
        //#

        at.out++ --> udp.appIn if hasUdp;
        at.in++ <-- udp.appOut if hasUdp;

        udp.ipOut --> tn.in++ if hasUdp;
        udp.ipIn <-- tn.out++ if hasUdp;

        wlanLteNic.radioIn <-- radioIn;
        pppIf.phys <--> ppp;

        tn.out++ --> ipv4.transportIn;
        tn.in++ <-- ipv4.transportOut;

        //udp.ipOut --> networkLayer.transportIn++;
        //udp.ipIn <-- networkLayer.transportOut++;

        trafficFlowFilter.gtpUserGateOut --> gtp_user.trafficFlowFilterGate;

        gtp_user.socketOut --> at.in++;
        gtp_user.socketIn <-- at.out++;

        ipv4.ifIn <-- nl.out++;
        ipv4.ifOut --> nl.in++;


        pppIf.upperLayerOut --> nl.in++;
        pppIf.upperLayerIn <-- nl.out++;



       // networkLayer.ifOut++ --> pppIf.upperLayerIn;
      //  pppIf.upperLayerOut --> networkLayer.ifIn++;

        wlanLteNic.upperLayerOut --> trafficFlowFilter.internetFilterGateIn;
        gtp_user.pppGate --> wlanLteNic.upperLayerIn;

        at.out++ --> tn.in++;
        at.in++ <-- tn.out++;
        tn.out++ --> nl.in++;
        tn.in++ <-- nl.out++;
}
