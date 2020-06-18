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

import inet.linklayer.ppp.PppInterface;
import inet.node.base.ApplicationLayerNodeBase;

//
// Implements an EPC Serving Gateway (S-GW)
//
module SgwStandard extends ApplicationLayerNodeBase
{
    @display("bgb=920,462;i=device/server2");
    @figure[applicationLayer](type=rectangle; pos=250,6; size=1000,130; lineColor=#808080; cornerRadius=5; fillColor=#ffff00; fillOpacity=0.1);
    @figure[applicationLayer.title](type=text; pos=1245,11; anchor=ne; text="application layer");
    
    submodules:
        gtp_user: GtpUser {
            @display("p=727,206");
            filter = false; // the sgw has no traffic flow filter
            tftFileName = "";
        }
    connections:
        gtp_user.socketOut --> at.in++;
        gtp_user.socketIn <-- at.out++;
}
