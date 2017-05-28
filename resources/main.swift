
//  main.swift
//  NBodyCode(Metal,GPGPU)
//
//  Created by Will An on 3/28/17.
//  Copyright © 2017 Will An. All rights reserved.
//

import Foundation
import Metal

let timeStep: Float = 0.005
let numTimeSteps = 10                             // minimum is 2

var device: MTLDevice! = nil
device = MTLCreateSystemDefaultDevice()

let displacement = Vector(x: 0, y: 0, z: 0)
let rotation: Vector? = Vector(x: 1, y: 0, z: 0)
let theta: Float? = π / 2
let velocity: Vector? = Vector(x: 10, y: 0, z: 0)
let (firstStep, secondStep) = generateSets(velocity: velocity, translation: displacement, axis: rotation, theta: theta)
let firstStepTotal = firstStep
let secondStepTotal = secondStep
var mySystem = NBodySystem(device: device, set0: firstStepTotal, set1: secondStepTotal)
var running = true

DispatchQueue.global(qos: .userInitiated).async {
    for _ in 0..<(numTimeSteps) {
        mySystem.update()
    }
    running = false
}

while(running) {}

mySystem.writeSystem()




