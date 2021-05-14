#include "hospital.hpp"
#include "network.hpp"

#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <queue>

using namespace std;

// @para name: 'A', 'B', 'C'
void usage(string name, string errMsg) {
    cerr << "ERROR: malformed commandline - " << errMsg << endl;
    cerr << "USAGE: ./hospital" << name << " <location> <total capacity> <initial occupancy>" << endl;
}

// @para name: 'A', 'B', 'C'
void parse_commandline(string name, int argc, char const *argv[], int &location, int &capacity, int &occupancy) {
    if (argc != 4) {
        usage(name, "number of argument");
        exit(-1);
    }

    stringstream ss_location(argv[1]);
    stringstream ss_capacity(argv[2]);
    stringstream ss_occupancy(argv[3]);

    ss_location >> location;
    ss_capacity >> capacity;
    ss_occupancy >> occupancy;

    if (capacity <= 0 || occupancy <= 0) {
        usage(name, "capacity and occupancy have to be integer and positive");
        exit(-1);
    }

    if (capacity < occupancy) {
        usage(name, "capacity has to be no less than occupancy");
        exit(-1);
    }
}

// @para name: "Hospital A", "Hospital B", "Hospital C"
void hospital_routine(int sockfd, Hospital &hosp, string name) {
    cout << endl;
    string msg;

    // wait for client location info from scheduler
    if (server_udp_rec_msg(sockfd, msg)) 
        return;
    int location;
    {
        stringstream ss(msg);
        ss >> location;
    }
    cout << name << " has received input from client at location " << location << endl;

    // calculate the (distance, score) and send back
    double distance = hosp.calcu_dist(location);
    double score = hosp.calcu_score(location);
    {
        stringstream ss;
        ss << distance << " " << score;
        if (client_udp_send_msg_to(sockfd, "localhost", PORT_SCHEDULER_UDP, ss.str()))
            return;
    }

    if (distance < 0) {
        cout << name << " does not have the location " << location << " in map" << endl;
        cout << name << " has sent \"location not found\" to the Scheduler" << endl;
    } else {
        cout << name << " has capacity = " << hosp.get_capacity() << ", occupation = " 
             << hosp.get_occupancy() << ", availability = " << hosp.get_availability() << endl;
        if (distance == 0) {
            cout << name << " has found the shortest path to client, distance = None" << endl;
            cout << name << " has the score = None" << endl;
            cout << name << " has sent score = None and distance = None to the Scheduler" << endl;
        } else {
            cout << name << " has found the shortest path to client, distance = " << distance << endl;
            cout << name << " has the score = " << score << endl;
            cout << name << " has sent score = " << score << " and distance = " << distance << " to the Scheduler" << endl;
        }
    }

    // wait for the decision, if 1 assign, otherwise, do nothing
    if (server_udp_rec_msg(sockfd, msg)) 
        return;
    int decision;
    {
        stringstream ss(msg);
        ss >> decision;
    }
    if (decision) {
        hosp.assign_client();
        cout << name << " has been assigned to a client, occupation is updated to " << hosp.get_occupancy()
             << ", availability is updated to " << hosp.get_availability() << endl;
    }
}



typedef pair<double, int> pd;

/*
    using dijkstra to find the shortest distance from root to all the other nodes
    @para graph: is a map, where key is the index of node, value is another key where key is all the adjacent nodes, value is the weight
    PRE: graph has to be undirected and connected, and weight can only be positive
*/
unordered_map<int, double> find_shortest_distance_from(int root, unordered_map<int, unordered_map<int, double> > graph) {
    if (graph.find(root) == graph.end()) {
        cerr << "ERROR: Hospital.cpp: find_shortest_distance_from: root is not inside the graph" << endl;
        exit(-1);
    }
    priority_queue<pd, vector<pd>, greater<pd> > pq;
    unordered_map<int, double> res;
    pq.push(make_pair(0, root));
    while (!pq.empty() && res.size() < graph.size()) {
        double cur_dist = pq.top().first;
        int cur_idx = pq.top().second;
        pq.pop();
        
        // cout << "!!(" << cur_dist << ", " << cur_idx << ")" << endl;
        // cout << "!!!!!!" << res.size() << " " << graph.size() << endl;

        for (auto pair : graph[cur_idx]) {
            int neighbor_idx = pair.first;
            double new_dist = cur_dist + pair.second;
            // priority queue 没有update操作。。先这样凑活吧。。
            if (res.find(neighbor_idx) == res.end())
                pq.push(make_pair(new_dist, neighbor_idx));
        }
        if (res.find(cur_idx) == res.end())
            res[cur_idx] = cur_dist;
    }

    // for (auto i : res) {
    //     cout << i.first << " " << i.second << endl;
    // }

    return res;
}


Hospital::Hospital(unsigned int location, unsigned int capacity, unsigned int occupancy) {
    this->location = location;
    this->capacity = capacity;
    this->occupancy = occupancy;

    this->distance_map = find_shortest_distance_from(location, this->build_graph());
}

unordered_map<int, unordered_map<int, double> > Hospital::build_graph() {
    unordered_map<int, unordered_map<int, double> > graph;
    ifstream mapfile;
    mapfile.open("map.txt");
    if (!mapfile) {
        cerr << "ERROR: Hospital: cannot open file map.txt" << endl;
        exit(-1);
    }

    int node_1, node_2;
    double weight;

    while (mapfile >> node_1 >> node_2 >> weight) {
        if ((graph.find(node_1) != graph.end()) && 
            (graph[node_1].find(node_2) != graph[node_1].end()) && 
            (graph[node_1][node_2] != weight)) {
            cerr << "ERROR: Hospital: map.txt contains duplicate edge with different weight" << endl;
        }
        graph[node_1][node_2] = weight;
        graph[node_2][node_1] = weight;
    }

    mapfile.close();
    if (graph.size() < 3) {
        cerr << "ERROR: Hospital: the graph has to have at least 3 locations" << endl;
        exit(-1);
    }
    return graph;
}

double Hospital::calcu_dist(int location) const {
    if (this->distance_map.find(location) == this->distance_map.end())
        return -1;
    // if (location == this->location)
    //     return 0;
    return this->distance_map.at(location);
}

double Hospital::calcu_score(int location) const {
    double shortest_dist = this->calcu_dist(location);
    if (shortest_dist <= 0 || this->capacity <= this->occupancy)
        return 0;
    double availability = this->get_availability();
    return 1.0 / (shortest_dist * (1.1 - availability));
}

void Hospital::assign_client() {
    if (this->capacity == this->occupancy) {
        cerr << "WARN: Hospital already full (this should not happen)!" << endl;
        return;
    }
    this->occupancy++;
}