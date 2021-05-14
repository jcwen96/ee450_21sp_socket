#ifndef HOSPITAL_H
#define HOSPITAL_H

#include <unordered_map>

class Hospital {
  public:
    
    // PRE: occupancy <= capacity
    Hospital(unsigned int location, unsigned int capacity, unsigned int occupancy);

    double calcu_score(int location) const;
    double calcu_dist(int location) const;
    void assign_client();

    inline double get_availability() const {return ((double)this->capacity - this->occupancy) / (double)this->capacity;}
    inline int get_capacity() const {return this->capacity;}
    inline int get_occupancy() const {return this->occupancy;}
  

  private:
    unsigned int location;
    unsigned int capacity;
    unsigned int occupancy;
    std::unordered_map<int, double> distance_map;

    std::unordered_map<int, std::unordered_map<int, double> > build_graph();
};

void usage(std::string name, std::string errMsg);
void parse_commandline(std::string name, int argc, char const *argv[], int &location, int &capacity, int &occupancy);
void hospital_routine(int sockfd, Hospital &hosp, std::string name);

#endif