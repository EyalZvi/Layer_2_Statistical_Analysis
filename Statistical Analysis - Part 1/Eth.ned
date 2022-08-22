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

//
// TODO auto-generated module
//
simple Eth
{
    parameters:
        volatile double arp_ttl @unit(s);
        int test_arp_requests;
    	@signal[interarrivalTime*](type=simtime_t); // note an asterisk and the type of emitted values
		@statisticTemplate[interarrivalTimeTemplate](record=vector,stats; interpolationmode=none);
	gates:
        inout ip;
        inout switch;
}
