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

network Lab3
{
    parameters:
        int num_of_hosts;

        @display("bgb=195.29333,145.5");
    types:
        channel Channel extends ned.DatarateChannel
        {
            datarate = 100Mbps;
        }
    submodules:

        host[num_of_hosts]: Host {

            @display("i=device/pc4");
        }
        switch: Switch {
            @display("p=93.12,72.42667;i=abstract/switch");
        }
    connections:
        for i=0..(num_of_hosts-1) {
            host[i].host_gate <--> Channel <--> switch.gate[i];
        }
}
