#include "network.hpp"

#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <unistd.h>

using namespace std;

void bootup() {
    cout << "The Scheduler is up and running" << endl;
}

int receive_hospitals_info(int sockfd_udp, vector<int> &vacancies) {
    int count = 0;
    string msg;
    
    while (count < NUM_HOSPITAL) {
        if (server_udp_rec_msg(sockfd_udp, msg))
            continue;
        int capacity, occupancy;
        {
            stringstream ss(msg);
            ss >> capacity >> occupancy;
        }    
        cout << "The Scheduler has received information from Hospital " << (char)('A' + count) 
             << ": total capacity is " << capacity << " and initial occupancy is " << occupancy << endl;
        vacancies.push_back(capacity - occupancy);
        count++;
    }
    return 0;
}

// @para: pair.first is distance, pair.second is score
bool comp_results(pair<double, double> i, pair<double, double> j) {
    if (i.second == j.second)
        return i.first < i.first;
    return i.second < j.second;
}

/*
@para: results: a vector of [distance, score]
                if distance = -1, location not in map
                              -2, hospital full 
                              0, location the same as hospital 
@return: -1, location not in map
         -2, location the same as hospital
         -3, all hospitals are already fully occupied
         0, hospitalA; 1, hospitalB; 2, hospitalC
*/
int make_decision(vector<pair<double, double> > results) {
    // check if distance is illegal
    for (auto pair : results) {
        if (pair.first == -1) return -1;
        if (pair.first == 0) return -2;
    }

    auto it = max_element(results.begin(), results.end(), comp_results);

    // if the max score is 0, then all hospitals are fully occupied
    if (it->second == 0) return -3;
    return distance(results.begin(), it);
}

void scheduler_routine(int sockfd_tcp, int sockfd_udp, vector<int> &vacancies) {
    cout << endl;
    string msg;

    int child_sockfd;
    if (server_tcp_get_child_sockfd(sockfd_tcp, child_sockfd))
        return;
    
    // wait for client location info from client over TCP
    if (tcp_rec_msg(child_sockfd, msg))
        return;
    int location;
    {
        stringstream ss(msg);
        ss >> location;
    }
    cout << "The Scheduler has received client at location " << location 
         << " from the client using TCP over port " << PORT_SCHEDULER_TCP << endl;

    // send location info to non-full hospitals over UDP and receive distance and score
    string port_nums[] = {PORT_HOSPITAL_A, PORT_HOSPITAL_B, PORT_HOSPITAL_C};
    vector<pair<double, double> > results;

    for (int i = 0; i < NUM_HOSPITAL; i++) {
        if (vacancies[i]) {
            // send location info to one non-full hospital over UDP
            {
                stringstream ss;
                ss << location;
                if (client_udp_send_msg_to(sockfd_udp, "localhost", port_nums[i], ss.str()))
                    return;
            }
            cout << "The Scheduler has sent client location to Hospital " << (char)('A' + i) 
                 << " using UDP over port " << PORT_SCHEDULER_UDP << endl;
            
            // receive distance and score
            if (server_udp_rec_msg(sockfd_udp, msg))
                return;

            double distance, score;
            {
                stringstream ss(msg);
                ss >> distance >> score;
            }
            results.push_back(make_pair(distance, score));
            
            if (distance <= 0 || score == 0)
                cout << "The Scheduler has received map information from Hospital " << (char)('A' + i)
                     << ", the score = None and the distance = None" << endl;    
            else
                cout << "The Scheduler has received map information from Hospital " << (char)('A' + i)
                     << ", the score = " << score << " and the distance = " << distance << endl;
        } else {
            results.push_back(make_pair(-2.0, 0.0));
        }
    }
    // make decision
    int decision = make_decision(results);
    if (decision < 0) {
        cout << "The Scheduler did not assign the client" << endl;
    } else {
        cout << "The Scheduler has assigned Hospital " << (char)('A' + decision) << " to the client" << endl;
    }

    // send result back to client
    {
        stringstream ss;
        ss << decision;
        if (tcp_send_msg(child_sockfd, ss.str()))
            return;
    }
    cout << "The Scheduler has sent the result to client using TCP over port " << PORT_SCHEDULER_TCP << endl;

    // send results to hospitals
    for (int i = 0; i < NUM_HOSPITAL; i++) {
        if (vacancies[i]) {
            {
                stringstream ss;
                ss << (i == decision ? 1 : 0);
                if (client_udp_send_msg_to(sockfd_udp, "localhost", port_nums[i], ss.str()))
                    return;
            }

            // 注意vacancies一定要和所有的hospital对应 consistent！！
            if (i == decision) {
                vacancies[i]--;
                cout << "The Scheduler has sent the result to Hospital " << (char)('A' + i) << " using UDP over port " << PORT_SCHEDULER_UDP << endl;
            }
        }
    }

    close(child_sockfd);
}

int main(int argc, char const *argv[]) {
    bootup();
    int sockfd_udp, sockfd_tcp;
    if (server_setup_socket(PORT_SCHEDULER_UDP, true, sockfd_udp) || 
        server_setup_socket(PORT_SCHEDULER_TCP, false, sockfd_tcp))
        return -1;

    vector<int> vacancies;
    if (receive_hospitals_info(sockfd_udp, vacancies))
        return -2;

    while (true)
        scheduler_routine(sockfd_tcp, sockfd_udp, vacancies);

    close(sockfd_tcp);
    close(sockfd_udp);
    return 0;
}