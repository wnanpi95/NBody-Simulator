//
//  Types.swift
//  GalaxySim
//
//  Created by Will An on 4/16/17.
//  Copyright © 2017 Will An. All rights reserved.
//

import simd

struct Vertex {
    var position: float3
    var color: float4
    
}


struct SceneConstants {
    var projectionMatrix = matrix_identity_float4x4
}

struct Body {
    var mass: Float
    var x:    Float
    var y:    Float
    var z:    Float
    
    init(mass: Float, vector: Vector) {
        self.mass = mass
        self.x = vector.x
        self.y = vector.y
        self.z = vector.z
    }
}

struct Accel {
    var x: Float
    var y: Float
    var z: Float
}

struct Vector {             // yes this is dumb...
    var x: Float
    var y: Float
    var z: Float
}

func +(lhs: Vector, rhs: Vector) -> Vector {
    return Vector(x: lhs.x + rhs.x,
                  y: lhs.y + rhs.y,
                  z: lhs.z + rhs.z)
}

func += (lhs: inout Vector, rhs: Vector) {
    lhs = lhs + rhs
}

func -(lhs: Vector, rhs: Vector) -> Vector {
    return Vector(x: lhs.x - rhs.x,
                  y: lhs.y - rhs.y,
                  z: lhs.z - rhs.z)
}

func *(lhs: Vector, rhs: Float) -> Vector {
    return Vector(x: lhs.x * rhs,
                  y: lhs.y * rhs,
                  z: lhs.z * rhs)
}








