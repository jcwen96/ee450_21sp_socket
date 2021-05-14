#include "network.hpp"

#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;


void bootup() {
    cout << "The Client is up and running" << endl;
}

void usage(string errMsg) {
    cerr << "ERROR: malformed commandline - " << errMsg << endl;
    cerr << "USAGE: ./client <location>" << endl;
}

void parse_commandline(int argc, char const *argv[], int &location) {
    if (argc != 2) {
        usage("number of argument");
        exit(-1);
    }

    stringstream ss_location(argv[1]);

    ss_location >> location;
}

int main(int argc, char const *argv[]) {
    bootup();
    int location;
    parse_commandline(argc, argv, location);

    int sockfd;
    if (client_tcp_setup_socket("localhost", PORT_SCHEDULER_TCP, sockfd))
        return -1;

    string msg;

    // send location to scheduler
    {
        stringstream ss;
        ss << location;
        if (tcp_send_msg(sockfd, ss.str()))
            return -1;
    }

    cout << "The client has sent query to Scheduler using TCP: client location " << location << endl;

    // receive result from scheduler
    if (tcp_rec_msg(sockfd, msg))
        return -2;
    int decision;
    {
        stringstream ss(msg);
        ss >> decision;
    }

    if (decision >= 0)
        cout << "The client has received results from the Scheduler: assigned to Hospital " << (char)('A' + decision) << endl;
    else {
        cout << "The client has received results from the Scheduler: assigned to Hospital None" << endl;
        if (decision == -1) {
            cout << "Location " << location << " not found" << endl;
        }
        if (decision == -2) {
            cout << "Location " << location << " the same as one hospital" << endl;
        }
        if (decision == -3) {
            cout << "Score = None, No assignment" << endl;
            cout << "All hospitals are fully occupied" << endl;
        }
    }

    close(sockfd);
    return 0;
}