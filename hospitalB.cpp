#include "network.hpp"
#include "hospital.hpp"

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;


int main(int argc, char const *argv[]) {

    int location, capacity, occupancy;
    parse_commandline("B", argc, argv, location, capacity, occupancy);

    Hospital h_b = Hospital(location, capacity, occupancy);
    
    cout << "Hospital B is up and running using UDP on port " << PORT_HOSPITAL_B << "." << endl;
    cout << "Hospital B has total capacity " << capacity << " and initial occupancy " << occupancy << "." << endl;

    int sockfd;
    if (server_setup_socket(PORT_HOSPITAL_B, true, sockfd))
        return -1;

    // send capacity and occupancy to scheduler
    ostringstream outstr;
    outstr << capacity << " " << occupancy;
    client_udp_send_msg_to(sockfd, "localhost", PORT_SCHEDULER_UDP, outstr.str());

    while (true)
        hospital_routine(sockfd, h_b, "Hospital B");

    close(sockfd);
    return 0;
}