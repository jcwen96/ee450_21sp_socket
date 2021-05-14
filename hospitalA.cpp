#include "network.hpp"
#include "hospital.hpp"

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;


int main(int argc, char const *argv[]) {

    int location, capacity, occupancy;
    parse_commandline("A", argc, argv, location, capacity, occupancy);

    Hospital h_a = Hospital(location, capacity, occupancy);
    
    cout << "Hospital A is up and running using UDP on port " << PORT_HOSPITAL_A << "." << endl;
    cout << "Hospital A has total capacity " << capacity << " and initial occupancy " << occupancy << "." << endl;

    int sockfd;
    if (server_setup_socket(PORT_HOSPITAL_A, true, sockfd))
        return -1;

    // send capacity and occupancy to scheduler
    ostringstream outstr;
    outstr << capacity << " " << occupancy;
    client_udp_send_msg_to(sockfd, "localhost", PORT_SCHEDULER_UDP, outstr.str());

    while (true)
        hospital_routine(sockfd, h_a, "Hospital A");

    close(sockfd);
    return 0;
}