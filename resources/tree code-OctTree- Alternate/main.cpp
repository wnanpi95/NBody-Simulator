//
//  main.cpp
//  TreeCode
//
//  Created by Will An on 3/18/17.
//  Copyright © 2017 Will An. All rights reserved.
//

#include <stdio.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>

#define G 1.0
#define THETA 0.5
#define NUM_OBJ 32000
#define NUM_TSTEPS 100
#define E 0//pow(NUM_OBJ,-0.3)/10
#define DT 0.001
#define GALAXY_WIDTH 100000

using namespace std;

struct Coord {
    double x;
    double y;
    double z;
};


std::string writeCoord(Coord &coord) {
    return to_string(coord.x) + '\t' + to_string(coord.y) + '\t' + to_string(coord.z);
}

Coord calc_separation(const Coord &ref, const Coord &pos) {
    double x = pos.x - ref.x;
    double y = pos.y - ref.y;
    double z = pos.z - ref.z;
    return {x,y,z};
}

double mag_separation(const Coord &ref, const Coord &pos) {
    Coord sep = calc_separation(ref, pos);
    return sqrt( sep.x*sep.x + sep.y*sep.y + sep.z*sep.z );
}

Coord calc_accel(const Coord &ref, const Coord &pos, const double &mass) {
    Coord sep = calc_separation(ref, pos);
    double mag = mag_separation(ref, pos);
    
    if (mag == 0.0)
        return {0,0,0};
    
    double x_a = ( -G*mass / (mag*mag+E)) * cos( atan2(sep.y, sep.x) ) * sin( acos(sep.z/mag) );
    double y_a = ( -G*mass / (mag*mag+E)) * sin( atan2(sep.y, sep.x) ) * sin( acos(sep.z/mag) );
    double z_a = ( -G*mass / (mag*mag+E)) *                              cos( acos(sep.z/mag) );
    
    return {x_a, y_a, z_a};
}

struct Body {
    double mass;
    Coord pos;
};

struct Node {
    
    Node* _n111;            // pointers to octant nodes
    Node* _n000;            // xyz, 0->negative, 1->positive
    Node* _n100;
    Node* _n010;
    Node* _n001;
    Node* _n110;
    Node* _n011;
    Node* _n101;
    
    Body* _child;     // final pointer to a body
    
    double _width;           // spacial width
    
    Coord _centerM;         // center of mass
    double _mass;            // total mass representative of the node
    
    Node(Coord center, double width, vector<Body*> candidate) {
        // all points are passed in a vector
        _width = width;
        _centerM = {0.0,0.0,0.0};
        _mass = 0.0;
        
        for (int i=0; i!=candidate.size(); ++i) {   // find center of mass/ mass of node
            float massC = candidate.at(i)->mass;
            float xC = candidate.at(i)->pos.x;
            float yC = candidate.at(i)->pos.y;
            float zC = candidate.at(i)->pos.z;
            
            _mass += massC;
            _centerM.x += massC*xC;
            _centerM.y += massC*yC;
            _centerM.z += massC*zC;
        }
        _centerM.x /= _mass;
        _centerM.y /= _mass;
        _centerM.z /= _mass;
        
        if (candidate.size() == 1) {                // assign body to node if it is the only one
            _child = candidate.at(0);
            _n111 = nullptr;
            _n000 = nullptr;
            _n100 = nullptr;
            _n010 = nullptr;
            _n001 = nullptr;
            _n110 = nullptr;
            _n011 = nullptr;
            _n101 = nullptr;
        } else {                                    // else sub-divide vector data set into octants
            _child = nullptr;
            
            vector<Body*> subCanditate111;
            vector<Body*> subCanditate000;
            vector<Body*> subCanditate100;
            vector<Body*> subCanditate010;
            vector<Body*> subCanditate001;
            vector<Body*> subCanditate110;
            vector<Body*> subCanditate011;
            vector<Body*> subCanditate101;
            for (int i=0; i!=candidate.size(); ++i) {
                double x = candidate.at(i)->pos.x;
                double y = candidate.at(i)->pos.y;
                double z = candidate.at(i)->pos.z;
                if (x >= center.x && y >= center.y && z >= center.z)
                    subCanditate111.push_back(candidate.at(i));
                else if (x <  center.x && y <  center.y && z <  center.z)
                    subCanditate000.push_back(candidate.at(i));
                else if (x >= center.x && y <  center.y && z <  center.z)
                    subCanditate100.push_back(candidate.at(i));
                else if (x <  center.x && y >= center.y && z <  center.z)
                    subCanditate010.push_back(candidate.at(i));
                else if (x <  center.x && y <  center.y && z >= center.z)
                    subCanditate001.push_back(candidate.at(i));
                else if (x >= center.x && y >= center.y && z <  center.z)
                    subCanditate110.push_back(candidate.at(i));
                else if (x <  center.x && y >= center.y && z >= center.z)
                    subCanditate011.push_back(candidate.at(i));
                else if (x >= center.x && y <  center.y && z >= center.z)
                    subCanditate101.push_back(candidate.at(i));
            }
            
            double subWidth = width/4.0;
            double newWidth = width/2.0;
            Coord subCenter111 = {center.x + subWidth, center.y + subWidth, center.x + subWidth};
            Coord subCenter000 = {center.x - subWidth, center.y - subWidth, center.x - subWidth};
            Coord subCenter100 = {center.x + subWidth, center.y - subWidth, center.x - subWidth};
            Coord subCenter010 = {center.x - subWidth, center.y + subWidth, center.x - subWidth};
            Coord subCenter001 = {center.x - subWidth, center.y - subWidth, center.x + subWidth};
            Coord subCenter110 = {center.x + subWidth, center.y + subWidth, center.x - subWidth};
            Coord subCenter011 = {center.x - subWidth, center.y + subWidth, center.x + subWidth};
            Coord subCenter101 = {center.x + subWidth, center.y - subWidth, center.x + subWidth};
            // recursively divide each candidate into further octants
            if (subCanditate111.size()!=0)
                _n111 = new Node(subCenter111, newWidth, subCanditate111);
            else
                _n111 = nullptr;
            
            if (subCanditate000.size()!=0)
                _n000 = new Node(subCenter000, newWidth, subCanditate000);
            else
                _n000 = nullptr;
            
            if (subCanditate100.size()!=0)
                _n100 = new Node(subCenter100, newWidth, subCanditate100);
            else
                _n100 = nullptr;
            
            if (subCanditate010.size()!=0)
                _n010 = new Node(subCenter010, newWidth, subCanditate010);
            else
                _n010 = nullptr;
            
            if (subCanditate001.size()!=0)
                _n001 = new Node(subCenter001, newWidth, subCanditate001);
            else
                _n001 = nullptr;
            
            if (subCanditate110.size()!=0)
                _n110 = new Node(subCenter110, newWidth, subCanditate110);
            else
                _n110 = nullptr;
            
            if (subCanditate011.size()!=0)
                _n011 = new Node(subCenter011, newWidth, subCanditate011);
            else
                _n011 = nullptr;
            
            if (subCanditate101.size()!=0)
                _n101 = new Node(subCenter101, newWidth, subCanditate101);
            else
                _n101 = nullptr;
            
        }
    }
    
    /*
     void clear() {
     
     if (_n111!=nullptr)
     _n111->clear();
     
     if (_n000!=nullptr)
     _n000->clear();
     
     if (_n100!=nullptr)
     _n100->clear();
     
     if (_n010!=nullptr)
     _n010->clear();
     
     if (_n001!=nullptr)
     _n001->clear();
     
     if (_n110!=nullptr)
     _n110->clear();
     
     if (_n011!=nullptr)
     _n011->clear();
     
     if (_n101!=nullptr)
     _n101->clear();
     
     delete this;
     }
     */
    
    void clear(Node* node) {
        if (node->_child!=nullptr) {
            if (node->_n111!=nullptr)
                clear(node->_n111);
            if (node->_n000!=nullptr)
                clear(node->_n000);
            if (node->_n100!=nullptr)
                clear(node->_n100);
            if (node->_n010!=nullptr)
                clear(node->_n010);
            if (node->_n001!=nullptr)
                clear(node->_n001);
            if (node->_n110!=nullptr)
                clear(node->_n110);
            if (node->_n011!=nullptr)
                clear(node->_n011);
            if (node->_n101!=nullptr)
                clear(node->_n101);
        }
        delete node;
        //node = nullptr;
    }
    
    ~Node() {
        if (_n111!=nullptr)
            clear(_n111);
        if (_n000!=nullptr)
            clear(_n000);
        if (_n100!=nullptr)
            clear(_n100);
        if (_n010!=nullptr)
            clear(_n010);
        if (_n001!=nullptr)
            clear(_n001);
        if (_n110!=nullptr)
            clear(_n110);
        if (_n011!=nullptr)
            clear(_n011);
        if (_n101!=nullptr)
            clear(_n101);
    }
    
    
};

Coord accelFromTree(Node &node, Coord &pos) {
    
    Coord accel = {0.0,0.0,0.0};
    double mag = mag_separation(node._centerM, pos);
    if (node._width/mag < THETA) {
        Coord force = calc_accel(node._centerM, pos, node._mass);
        return force;
    } else if (node._child!=nullptr) {
        Coord force = calc_accel(node._child->pos, pos, node._child->mass);
        return force;
    } else {
        
        if (node._n000!=nullptr) {
            Coord force = accelFromTree(*node._n000, pos);
            accel.x += force.x;
            accel.y += force.y;
            accel.z += force.z;
        }
        if (node._n111!=nullptr) {
            Coord force = accelFromTree(*node._n111, pos);
            accel.x += force.x;
            accel.y += force.y;
            accel.z += force.z;
        }
        if (node._n100!=nullptr) {
            Coord force = accelFromTree(*node._n100, pos);
            accel.x += force.x;
            accel.y += force.y;
            accel.z += force.z;
        }
        if (node._n010!=nullptr) {
            Coord force = accelFromTree(*node._n010, pos);
            accel.x += force.x;
            accel.y += force.y;
            accel.z += force.z;
        }
        if (node._n001!=nullptr) {
            Coord force = accelFromTree(*node._n001, pos);
            accel.x += force.x;
            accel.y += force.y;
            accel.z += force.z;
        }
        if (node._n110!=nullptr) {
            Coord force = accelFromTree(*node._n110, pos);
            accel.x += force.x;
            accel.y += force.y;
            accel.z += force.z;
        }
        if (node._n011!=nullptr) {
            Coord force = accelFromTree(*node._n011, pos);
            accel.x += force.x;
            accel.y += force.y;
            accel.z += force.z;
        }
        if (node._n101!=nullptr) {
            Coord force = accelFromTree(*node._n101, pos);
            accel.x += force.x;
            accel.y += force.y;
            accel.z += force.z;
        }
    }
    
    return accel;
}


class System {
public:
    vector<vector<Body>> _system;
    vector<Coord> _backStep;
    
    System();
    void genPoint();
    void writeSystem();
};

System::System() {
    
    ifstream data0;
    data0.open("/Users/willan/Documents/PHYS_141/combined1.data");
    if (data0.is_open()) {
        string in;
        data0 >> in;
        //data0 >> in;
        vector<Body> initialPositions;
        for (int j=0; j!=NUM_OBJ/2; ++j) {
            
            data0 >> in;
            double mass = stof(in);
            data0 >> in;
            double x_0 = stof(in);
            data0 >> in;
            double y_0 = stof(in);
            data0 >> in;
            double z_0 = stof(in);
            initialPositions.push_back({mass,x_0,y_0,z_0});
            
            data0 >> in;
            double vx_0 = stof(in);
            data0 >> in;
            double vy_0 = stof(in);
            data0 >> in;
            double vz_0 = stof(in);
            double xBack = x_0 - vx_0*DT;
            double yBack = y_0 - vy_0*DT;
            double zBack = z_0 - vz_0*DT;
            _backStep.push_back({xBack,yBack,zBack});
        }
        for (int j=0; j!=384; ++j) {
            for (int k=0; k!=7; ++k){
                data0 >> in;
            }
        }
        for (int j=0; j!=NUM_OBJ/2; ++j) {
            
            data0 >> in;
            double mass = stof(in);
            data0 >> in;
            double x_0 = stof(in);
            data0 >> in;
            double y_0 = stof(in);
            data0 >> in;
            double z_0 = stof(in);
            initialPositions.push_back({mass,x_0,y_0,z_0});
            
            data0 >> in;
            double vx_0 = stof(in);
            data0 >> in;
            double vy_0 = stof(in);
            data0 >> in;
            double vz_0 = stof(in);
            double xBack = x_0 - vx_0*DT;
            double yBack = y_0 - vy_0*DT;
            double zBack = z_0 - vz_0*DT;
            _backStep.push_back({xBack,yBack,zBack});
        }
        /*
         for (int j=0; j!=384; ++j) {
         for (int k=0; k!=7; ++k){
         data0 >> in;
         }
         }
         */
        _system.push_back(initialPositions);
    } else {cout << "Unable to open file"; }
    
    vector<Body> firstStep;
    vector<Body*> posAddresses;
    for (int k=0; k!=_system.at(0).size(); ++k)
        posAddresses.push_back(&_system.at(0).at(k));
    Node root({0.0,0.0,0.0}, GALAXY_WIDTH, posAddresses);
    for (int j=0; j!=NUM_OBJ; ++j) {
        Coord accel = accelFromTree(root, _system.back().at(j).pos);
        double x = _system.back().at(j).pos.x*2.0 - _backStep.at(j).x + accel.x*DT*DT;
        double y = _system.back().at(j).pos.y*2.0 - _backStep.at(j).y + accel.y*DT*DT;
        double z = _system.back().at(j).pos.z*2.0 - _backStep.at(j).z + accel.z*DT*DT;
        double mass = _system.back().at(j).mass;
        firstStep.push_back({mass,x,y,z});
    }
    _system.push_back(firstStep);
}

void System::genPoint() {
    vector<Body*> posAddresses;
    for (int k=0; k!=_system.at(0).size(); ++k)
        posAddresses.push_back(&_system.back().at(k));
    Node root({0.0,0.0,0.0}, GALAXY_WIDTH, posAddresses);
    vector<Body> step;
    int size = _system.size();
    for (int j=0; j!=NUM_OBJ; ++j) {
        Coord accel = accelFromTree(root, _system.back().at(j).pos);
        double x = _system.back().at(j).pos.x*2.0 - _system.at(size-2).at(j).pos.x + accel.x*DT*DT;
        double y = _system.back().at(j).pos.y*2.0 - _system.at(size-2).at(j).pos.y + accel.y*DT*DT;
        double z = _system.back().at(j).pos.z*2.0 - _system.at(size-2).at(j).pos.z + accel.z*DT*DT;
        double mass = _system.back().at(j).mass;
        step.push_back({mass,x,y,z});
    }
    _system.push_back(step);
}

void System::writeSystem() {
    ofstream system;
    system.open("/Users/willan/Documents/PHYS_141/system3.txt");
    if (system.is_open()) {
        for (int i=0; i!=NUM_TSTEPS; ++i) {
            for (int j=0; j!=NUM_OBJ; ++j) {
                Coord body_pos = _system.at(i).at(j).pos;
                system << writeCoord(body_pos) + '\t';
            }
            system << '\n';
        }
        system.close();
    }
    else { cout << "Unable to open file\n"; }
}


// Globals

int main() {
    System mySystem;
    
    for (int k=0; k!=NUM_TSTEPS-2; ++k) {
        mySystem.genPoint();
    }
    
    mySystem.writeSystem();
    
    return 0;
}
