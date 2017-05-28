//
//  NBodySystem.swift
//  NBodyCode(Metal,GPGPU)
//
//  Created by Will An on 4/10/17.
//  Copyright © 2017 Will An. All rights reserved.
//

import Foundation
import Metal

class NBodySystem {
    let _count: Int = 4096
    
    var defaultLibrary: MTLLibrary! = nil
    var commandQueue: MTLCommandQueue! = nil
    
    var kernelFunctionAccelUpdate: MTLFunction!
    var pipelineStateAccelUpdate: MTLComputePipelineState!
    
    var kernelFunctionPosUpdate: MTLFunction!
    var pipelineStatePosUpdate: MTLComputePipelineState!
    
    var bodySetArray: [KernelBodySet]
    var accelSet: KernelAccelSet
    var openSetIndex: Int = 2
    var _system: [[Body]] = []
    
    init(device: MTLDevice, set0: [Body], set1: [Body]) {
        
        defaultLibrary = device.newDefaultLibrary()
        commandQueue = device.makeCommandQueue()
        
        kernelFunctionAccelUpdate = defaultLibrary.makeFunction(name: "nbody_accel_shader")
        do {
            pipelineStateAccelUpdate = try device.makeComputePipelineState(function: kernelFunctionAccelUpdate!)
        } catch {
            fatalError("Couldn't set kernel")
        }
        
        kernelFunctionPosUpdate = defaultLibrary.makeFunction(name: "nbody_pos_shader")
        do {
            pipelineStatePosUpdate = try device.makeComputePipelineState(function: kernelFunctionPosUpdate!)
        } catch {
            fatalError("Couldn't set kernel")
        }
        
        let numObj = set0.count
        let kernelBodySetAlpha = KernelBodySet(set: set0)
        let kernelBodySetBeta = KernelBodySet(set: set1)
        let kernelBodySetGamma = KernelBodySet(setCount: numObj)
        bodySetArray = [kernelBodySetAlpha,
                        kernelBodySetBeta,
                        kernelBodySetGamma]
    
        accelSet = KernelAccelSet(setCount: numObj)
    }
    
    func subDivAccelUpdate(primary: KernelBodySubDiv,
                           secondary: KernelBodySubDiv,
                           accel: KernelAccelSubDiv,
                           last: Bool) {
        
        let commandBuffer = commandQueue.makeCommandBuffer()
        let commandEncoder = commandBuffer.makeComputeCommandEncoder()
        
        commandEncoder.setComputePipelineState(pipelineStateAccelUpdate)
        
        commandEncoder.setBuffer(primary.metalBuffer, offset: 0, at: 0)
        commandEncoder.setBuffer(secondary.metalBuffer, offset: 0, at: 1)
        commandEncoder.setBuffer(accel.metalBuffer, offset: 0, at: 2)
        
        commandEncoder.dispatchThreadgroups(nbody_threadGroups, threadsPerThreadgroup: nbody_threadGroupCount)
        commandEncoder.endEncoding()
        commandBuffer.commit()
        
        if last {
            //commandQueue.insertDebugCaptureBoundary()
            commandBuffer.waitUntilCompleted()
        }
    }
    
    func subDivPosUpdate(former: KernelBodySubDiv,
                         current: KernelBodySubDiv,
                         new: KernelBodySubDiv,
                         accel: KernelAccelSubDiv,
                         last: Bool) {
        
        let commandBuffer = commandQueue.makeCommandBuffer()
        let commandEncoder = commandBuffer.makeComputeCommandEncoder()
        commandEncoder.setComputePipelineState(pipelineStatePosUpdate)
        
        commandEncoder.setBuffer(former.metalBuffer, offset: 0, at: 0)
        commandEncoder.setBuffer(current.metalBuffer, offset: 0, at: 1)
        commandEncoder.setBuffer(new.metalBuffer, offset: 0, at: 2)
        commandEncoder.setBuffer(accel.metalBuffer, offset: 0, at: 3)
        
        commandEncoder.dispatchThreadgroups(nbody_threadGroups, threadsPerThreadgroup: nbody_threadGroupCount)
        commandEncoder.endEncoding()
        commandBuffer.commit()
    
        if last {
            //commandQueue.insertDebugCaptureBoundary()
            commandBuffer.waitUntilCompleted()
        }
    }
    
    func updateAccel() {
        
        accelSet.clear()
        let currentSetIndex = (openSetIndex+2)%3
        var last = false
        var index = 0
        var counter = 0
        let numSubDivs = bodySetArray[currentSetIndex].subDivs.count
                  * bodySetArray[currentSetIndex].subDivs.count
        for primary in bodySetArray[currentSetIndex].subDivs {
            for secondary in bodySetArray[currentSetIndex].subDivs {
                if counter == (numSubDivs - 1) {
                    last = true
                }
                subDivAccelUpdate(primary: primary,
                                  secondary: secondary,
                                  accel: accelSet.subDivs[index],
                                  last: last)
                counter += 1
            }
            index += 1
        }
    }
    
    func updatePos() {
        let formerSetIndex = (openSetIndex+1)%3
        let currentSetIndex = (openSetIndex+2)%3
        var last = false
        let numSubDivs = bodySetArray[openSetIndex].subDivs.count
        for index in 0..<numSubDivs {
            if index == (numSubDivs - 1) {
                last = true
            }
            subDivPosUpdate(former: bodySetArray[formerSetIndex].subDivs[index],
                            current: bodySetArray[currentSetIndex].subDivs[index],
                            new: bodySetArray[openSetIndex].subDivs[index],
                            accel: accelSet.subDivs[index],
                            last: last)
        }
    }
    
    func newStep() -> [Body] {
        var bodyArray: [Body] = []
        let zeroVec = Vector(x: 0, y: 0, z: 0)
        let zeroBody = Body(mass: 0, vector: zeroVec)
        
        for subDiv in bodySetArray[openSetIndex].subDivs {
            var finalResultArray = [Body](repeating: zeroBody, count: _count)
            let data = NSData(bytesNoCopy: subDiv.memory!,
                              length: _count*MemoryLayout<Body>.size,
                              freeWhenDone: false)
            data.getBytes(&finalResultArray, length: _count*MemoryLayout<Body>.size)
            bodyArray += finalResultArray
        }
        return bodyArray
    }
    
    func update() {
        updateAccel()
        updatePos()
        append()
        
        openSetIndex += 1
        openSetIndex %= 3
    }
    
    func append() {                             // doesnt actually append file at the moment
        let newTimeStep = newStep()
        _system.append(newTimeStep)
    }
    
    func writeSystem() {
        let path = "/Users/willan/Documents/PHYS_141/FINAL_DATA/output"
        do {
            var formattedText = ""
            var bodyString = ""
            for i in 0..<_system.count {
                for body in _system[i] {
                    let mass = String(body.mass)
                    let x = String(body.x)
                    let y = String(body.y)
                    let z = String(body.z)
                    bodyString = mass + "\t" + x + "\t" + y + "\t" + z + "\n"
                    formattedText += bodyString
                }
            }
            try formattedText.write(toFile: path, atomically: false, encoding: String.Encoding.utf8)
        } catch {print("Couldn't print the file")}
    }
    
}
