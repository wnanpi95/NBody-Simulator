//
//  ComputeShader.metal
//  NBodyCode(Metal,GPGPU)
//
//  Created by Will An on 3/28/17.
//  Copyright © 2017 Will An. All rights reserved.
//

#include <metal_stdlib>
using namespace metal;

struct Body {
    float mass;
    float x;
    float y;
    float z;
};

struct Accel {
    float x;
    float y;
    float z;
};

kernel void nbody_accel_shader(const device Body *persistentSet [[ buffer(0) ]],
                               const device Body *swappedSet [[ buffer(1) ]],
                               device Accel *accelSet [[ buffer(2) ]],
                               uint id [[ thread_position_in_grid ]]) {
    float G = 1.0;
    float E = 0.0;
    
    float posX = persistentSet[id].x;
    float posY = persistentSet[id].y;
    float posZ = persistentSet[id].z;
    
    for (int i=0; i!=4096; ++i) {
        
        float swappedX = swappedSet[i].x;
        float swappedY = swappedSet[i].y;
        float swappedZ = swappedSet[i].z;
        
        float sepX = posX - swappedX;
        float sepY = posY - swappedY;
        float sepZ = posZ - swappedZ;
        
        float mag = sqrt(  (sepX)*(sepX)
                         + (sepY)*(sepY)
                         + (sepZ)*(sepZ) );
        
        if (mag > E) {
            float mass = swappedSet[i].mass;
        
            float accelMagnitude = -G*mass / (mag*mag+E);
            float theta = sepZ / mag;
            
            accelSet[id].x += accelMagnitude * cos( atan2(sepY, sepX) ) * sin( acos(theta) );
            accelSet[id].y += accelMagnitude * sin( atan2(sepY, sepX) ) * sin( acos(theta) );
            accelSet[id].z += accelMagnitude *                            cos( acos(theta) );
        }
    }
}

kernel void nbody_pos_shader(const device Body *formerSet [[ buffer(0) ]],
                             const device Body *currentSet [[ buffer(1) ]],
                             device Body *newSet [[ buffer(2) ]],
                             const device Accel *accelSet [[ buffer(3) ]],
                             uint id [[ thread_position_in_grid ]]) {
    float timeStep = 0.005;

    newSet[id].x = 2.0*currentSet[id].x - formerSet[id].x + accelSet[id].x * timeStep * timeStep;
    newSet[id].y = 2.0*currentSet[id].y - formerSet[id].y + accelSet[id].y * timeStep * timeStep;
    newSet[id].z = 2.0*currentSet[id].z - formerSet[id].z + accelSet[id].z * timeStep * timeStep;
}

/*
kernel void nbody_interaction_cross_shader(const device Body *persistentSet [[ buffer(0) ]],
                                           const device Body *swappedSet [[ buffer(1) ]],
                                           device Accel *accelSet1 [[ buffer(2) ]],
                                           device Accel *accelSet2 [[ buffer(3) ]],
                                           uint id [[ thread_position_in_grid ]]) {
    float G = 1.0;
    float E = 0.0;
    
    float posX = persistentSet[id].x;
    float posY = persistentSet[id].y;
    float posZ = persistentSet[id].z;
    
    for (int i=0; i!=4096; ++i) {
        
        float swappedX = swappedSet[i].x;
        float swappedY = swappedSet[i].y;
        float swappedZ = swappedSet[i].z;
        
        float sepX = posX - swappedX;
        float sepY = posY - swappedY;
        float sepZ = posZ - swappedZ;
        
        float mag = sqrt(  (sepX)*(sepX)
                         + (sepY)*(sepY)
                         + (sepZ)*(sepZ) );
        
        if (mag > E) {
            float mass = swappedSet[i].mass;
            
            accelSet1[id].x += ( -G*mass / (mag*mag+E)) * cos( atan2(sepY, sepX) ) * sin( acos(sepZ/mag) );
            accelSet1[id].y += ( -G*mass / (mag*mag+E)) * sin( atan2(sepY, sepX) ) * sin( acos(sepZ/mag) );
            accelSet1[id].z += ( -G*mass / (mag*mag+E)) *                            cos( acos(sepZ/mag) );
            
            accelSet2[id].x += ( -G*mass / (mag*mag+E)) * cos( atan2(sepY, sepX) ) * sin( acos(sepZ/mag) );
            accelSet2[id].y += ( -G*mass / (mag*mag+E)) * sin( atan2(sepY, sepX) ) * sin( acos(sepZ/mag) );
            accelSet2[id].z += ( -G*mass / (mag*mag+E)) *                            cos( acos(sepZ/mag) );
        }
    }
}
 */
