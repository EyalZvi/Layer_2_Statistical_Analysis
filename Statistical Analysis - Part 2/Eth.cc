//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "Eth.h"

Define_Module(Eth);

void Eth::initialize()
{
    num_of_ip_msgs = 0;
    src_MAC = this->getParentModule()->getIndex();      // Set ARP requests to 0
    for(int i=0; i<10; i++)
        last_pck_arrival[i] = 0;
    if(this->getParentModule()->getIndex() == 0 ){      // Set measurement variables
        char signalName[32];
        sprintf(signalName, "interarrivalTime");
        simsignal_t signal = registerSignal(signalName);
        cProperty *statisticTemplate = getProperties()->get("statisticTemplate", "interarrivalTimeTemplate");
        getEnvir()->addResultRecorders(this, signal, signalName,  statisticTemplate);
        interarrivalTimeSignals = signal;
    }


}

void Eth::handleMessage(cMessage *msg)
{
    cGate *arrive = msg->getArrivalGate();
    std::map<int,ARPEntry>::iterator it;
    bool delete_header = true;

    if (strcmp(arrive->getName(),"ip$i") == 0 ){        // If the message came from "ip" module act accordingly
        IPv4Msg* rcv = check_and_cast<IPv4Msg*>(msg);
        my_ip = rcv->getSrc_ip();
        EthMsg* Eth_pck;
        it = ArpTable.find(rcv->getDest_ip());

        if(it != ArpTable.end())
        {
            if(it->second.expire >= simTime())  // Generate EtherType: IP
            {
                Eth_pck = generateMessage(src_MAC,it->second.MAC,0,0,0);
                Eth_pck->encapsulate(rcv);
                Eth_pck->setSending_time(simTime());
            }
            else                                // Generate EtherType: ARP Request AND Refresh entry
            {
                ArpTable.erase(it);
                Eth_pck = generateMessage(src_MAC,-1,1,rcv->getSrc_ip(),rcv->getDest_ip());
                delete rcv;
            }

        }

        else                                    // Generate EtherType: ARP Request
        {
            Eth_pck = generateMessage(src_MAC,-1,1,rcv->getSrc_ip(),rcv->getDest_ip());
            delete rcv;
        }
            cGate* exit_gate = gate("switch$o");
            cChannel* channel = exit_gate->getTransmissionChannel();
            if(channel->isBusy())
            {
                sendDelayed(Eth_pck,channel->getTransmissionFinishTime() - simTime(),"switch$o");
            }
            else send(Eth_pck,"switch$o");
    }


    if (strcmp(arrive->getName(),"switch$i") == 0 )     // If the message came from "switch" module act accordingly
    {
        EthMsg* rcv = check_and_cast<EthMsg*>(msg);

        if(rcv->getEtherType() == 0)                    // IF Receive EtherType: IP
        {                                               // THEN Decapsulate

            if(rcv->getDest_MAC() == src_MAC)
            {

                // Collecting data:
                if(this->getParentModule()->getIndex() == 0){
                    simtime_t delay = simTime() - last_pck_arrival[rcv->getSrc_ip()];
                    emit(interarrivalTimeSignals, delay);
                    last_pck_arrival[rcv->getSrc_ip()] = simTime();
                    num_of_ip_msgs++;
                    if(num_of_ip_msgs == int(par("test_ip_msgs")))
                        endSimulation();
                }
                //

                IPv4Msg* IP_pck;
                IP_pck = (IPv4Msg*) rcv->decapsulate();
                send(IP_pck,"ip$o");
            }
        }

        if(rcv->getEtherType() == 1)                    // IF Receive EtherType: ARP Request
        {                                               // THEN Generate EtherType: ARP Reply
            if(rcv->getDest_ip() == my_ip)
            {
                delete_header = false;                  // Switch ARP Request to ARP Reply and send
                rcv->setEtherType(2);
                rcv->setDest_MAC(rcv->getSrc_MAC());
                rcv->setSrc_MAC(src_MAC);
                int new_src = rcv->getDest_ip();
                rcv->setDest_ip(rcv->getSrc_ip());
                rcv->setSrc_ip(new_src);
                cGate* exit_gate = gate("switch$o");
                cChannel* channel = exit_gate->getTransmissionChannel();
                if(channel->isBusy())
                {
                    sendDelayed(rcv,channel->getTransmissionFinishTime() - simTime(),"switch$o");
                }
                else send(rcv,"switch$o");
            }
        }

        if(rcv->getEtherType() == 2)                    // Receive EtherType: ARP Reply
        {
            if(rcv->getDest_MAC() == src_MAC)
            {
                it = ArpTable.find(rcv->getSrc_ip());       // Update ARP table
                if(it != ArpTable.end()){
                    it->second.expire = simTime() + par("arp_ttl");
                }
                else
                {
                    ARPEntry new_entry = {rcv->getSrc_MAC(),simTime() + par("arp_ttl")};
                    ArpTable.insert(std::pair<int,ARPEntry>(rcv->getSrc_ip(),new_entry));
                }
            }
        }
       if(delete_header) delete rcv;
    }
}

EthMsg* generateMessage(int src_MAC,int dest_MAC,int Ether,int src_ip,int dest_ip)
{    // Generate Eth message
    EthMsg *Eth_pck = new EthMsg();

    Eth_pck->setSrc_MAC(src_MAC);
    Eth_pck->setEtherType(Ether);


    if(Ether == 0)                  // IP EtherType:
    {
        Eth_pck->setDest_MAC(dest_MAC);
    }
    if(Ether == 1)                   // ARP Request EtherType:
    {
        Eth_pck->setSrc_ip(src_ip);
        Eth_pck->setDest_ip(dest_ip);
    }
    return Eth_pck;
}
