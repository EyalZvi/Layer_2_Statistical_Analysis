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

#include "switch.h"

Define_Module(Switch);

void Switch::initialize()
{
}

void Switch::handleMessage(cMessage *msg)
{
    int num_of_hosts = par("num_of_hosts");
    if(!msg->isSelfMessage())
    {
        EthMsg* rcv = check_and_cast<EthMsg*>(msg);
        rcv->setPort(rcv->getArrivalGate()->getIndex());
        scheduleAt(simTime() + rcv->getBitLength()*3/1000000000,rcv);   // Reschedule to simulate forwarding latency
    }
    else
    {
        std::map<int,SwitchEntry>::iterator it;
            EthMsg* rcv_msg = check_and_cast <EthMsg*> (msg);           // Set variables
            int src_MAC = rcv_msg->getSrc_MAC();

        int port = rcv_msg->getPort();

        // UPDATE table entry

        it = SwitchTable.find(src_MAC);
        if(it != SwitchTable.end())
        {
            it->second.expire = simTime() + par("aging_time");
        }
        else                                                            // Update expiration date for tables
        {
            SwitchEntry new_entry = {port,simTime() + par("aging_time")};
            SwitchTable.insert(std::pair<int,SwitchEntry>(src_MAC,new_entry));
        }

        it = SwitchTable.find(rcv_msg->getDest_MAC());
        if(it != SwitchTable.end() && rcv_msg->getEtherType() != 1)
        {
            cGate* exit_gate = gate("gate$o",it->second.port);          // If entry at ARP table -> Forward packet to right dest
            cChannel* channel = exit_gate->getTransmissionChannel();
            if(channel->isBusy())
            {
                sendDelayed(rcv_msg,channel->getTransmissionFinishTime() - simTime(),"gate$o",it->second.port);
            }
            else send(rcv_msg,"gate$o",it->second.port);
        }
        else
        {
            for(int i=0; i<num_of_hosts; ++i)           // If entry not at ARP table -> Flood to all ports (but input port)
            {
                if(i != port)
                {
                    cGate* exit_gate = gate("gate$o",i);
                    cChannel* channel = exit_gate->getTransmissionChannel();
                    EthMsg* send_msg =  new EthMsg();
                    memcpy(send_msg,rcv_msg,sizeof(EthMsg));
                    if(channel->isBusy())
                    {
                        sendDelayed(send_msg,channel->getTransmissionFinishTime() - simTime(),"gate$o",i);
                    }
                    else send(send_msg,"gate$o",i);
                }
            }
            delete rcv_msg;
        }
    }
}












