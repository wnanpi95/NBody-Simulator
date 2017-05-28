//
//  SetInit.swift
//  GalaxySim
//
//  Created by Will An on 4/16/17.
//  Copyright © 2017 Will An. All rights reserved.
//

import Foundation
import simd
import Accelerate

func generateSets(velocity: Vector?,
                  translation: Vector?,
                  axis: Vector?, theta: Float?) -> ([Body], [Body]) {
    let timeStep: Float = 0.005
    var firstStep: [Body] = []
    var secondStep: [Body] = []
    var matrix: matrix_float4x4?
    
    // Initialize seeds (initial conditions) of system
    let path = "/Users/willan/Documents/PHYS_141/FINAL_DATA/data"
    
    /*
     guard let assetURL = Bundle.main.url(forResource: "data", withExtension: "txt") else {
     fatalError("Asset data does not exist.")
     }
     */
    
    do {
        var ifstreamRaw: String = ""
        ifstreamRaw = try String(contentsOfFile: path, encoding: String.Encoding.utf8)
        let ifstreamArray = ifstreamRaw.components(separatedBy: "\n")
        let filler = 4096 - ((ifstreamArray.count-1) % 4096)
        
        if (translation != nil) || ((axis != nil) && (theta != nil)) {
            matrix = matrix_identity_float4x4
            if (translation != nil) {
                matrix = matrix?.translatedBy(displacement: translation!)
            }
            if (axis != nil) && (theta != nil) {
                matrix = matrix?.rotatedBy(rotationAngle: theta!, axis: axis!)
            }
        }
        
        for i in 0..<((ifstreamArray.count-1)) {
            
            let ifstreamRow = ifstreamArray[i].components(separatedBy: "\t")
            
            let mass = Float(ifstreamRow[0])!
            let xS = Float(ifstreamRow[1])!
            let yS = Float(ifstreamRow[2])!
            let zS = Float(ifstreamRow[3])!
            var second = Vector(x: xS, y: yS, z: zS)
            
            if (matrix != nil) {
                var affine = vector_float4(position: second)
                affine.transform(matrix: matrix!)
                second = affine.getVec()
            }
            
            //let type = typeArray[i]
            
            let newBodySecond = Body(mass: mass, vector: second)
            secondStep.append(newBodySecond)
            
            var vel = Vector(x: Float(ifstreamRow[4])!,
                             y: Float(ifstreamRow[5])!,
                             z: Float(ifstreamRow[6])!)
            
            if (matrix != nil) {
                var affine = vector_float4(velocity: vel)
                affine.transform(matrix: matrix!)
                vel = affine.getVec()
            }
            
            if (velocity != nil) {
                vel += velocity!
            }
            
            let first = second - vel * timeStep
            
            let newBodyFirst = Body(mass: mass, vector: first)
            firstStep.append(newBodyFirst)
        }
        
        let zeroVec = Vector(x: 0, y: 0, z: 0)
        let zeroBody = Body(mass: 0, vector: zeroVec)
        for _ in 0..<filler {
            firstStep.append(zeroBody)
            secondStep.append(zeroBody)
            
        }
    } catch {print("Couldn't read the file")}
    
    return (firstStep, secondStep)
}
