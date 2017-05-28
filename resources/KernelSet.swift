//
//  KernelSet.swift
//  NBodyCode(Metal,GPGPU)
//
//  Created by Will An on 4/10/17.
//  Copyright © 2017 Will An. All rights reserved.
//

import Foundation
import Metal

var nbody_threadGroupCount:MTLSize = MTLSize(width:32,height:1,depth:1)
var nbody_threadGroups:MTLSize = MTLSize(width:(4096 + 31) / 32, height:1, depth:1)

class KernelSubDiv<T> {
    let _alignment:Int = 0x4000
    let _count: Int = 4096
    
    var memory:UnsafeMutableRawPointer? = nil
    var voidPtr: OpaquePointer!
    var ptr: UnsafeMutablePointer<T>!
    var bufferPtr: UnsafeMutableBufferPointer<T>!
    
    var metalBuffer: MTLBuffer
    
    init() {
        let memoryByteSize:Int = 4096 * MemoryLayout<T>.size
        posix_memalign(&memory, _alignment, memoryByteSize)
        voidPtr = OpaquePointer(memory)
        ptr = UnsafeMutablePointer<T>(voidPtr)
        bufferPtr = UnsafeMutableBufferPointer(start: ptr, count: _count)
        
        metalBuffer = device.makeBuffer(bytesNoCopy: memory!, length: memoryByteSize, options: [], deallocator: nil)
    }
    
    func clear() {}
}

class KernelBodySubDiv: KernelSubDiv<Body> {
    override func clear() {
        let zeroVec = Vector(x: 0, y: 0, z: 0)
        let zeroBody = Body(mass: 0, vector: zeroVec)
        for index in bufferPtr.startIndex ..< bufferPtr.endIndex
        {
            bufferPtr[index] = zeroBody
        }
    }
}

class KernelAccelSubDiv: KernelSubDiv<Accel> {
    override func clear() {
        let zeroAccel = Accel(x: 0, y: 0, z: 0)
        for index in bufferPtr.startIndex ..< bufferPtr.endIndex
        {
            bufferPtr[index] = zeroAccel
        }
    }
}

class KernelBodySet {
    let _count: Int = 4096
    var subDivs: [KernelBodySubDiv]
    
    init(set: [Body]) {
        subDivs = []
        let numSubDivs = set.count / _count
        for i in 0..<numSubDivs {
            let newSubDiv = KernelBodySubDiv()
            for index in newSubDiv.bufferPtr.startIndex ..< newSubDiv.bufferPtr.endIndex {
                let setIndex = index + i * _count
                let body = set[setIndex]
                newSubDiv.bufferPtr[index] = body
            }
            subDivs.append(newSubDiv)
        }
    }
    
    init(setCount: Int) {
        subDivs = []
        let numSubDivs = setCount / _count
        let zeroVec = Vector(x: 0, y: 0, z: 0)
        let zeroBody = Body(mass: 0, vector: zeroVec)
        for _ in 0..<numSubDivs {
            let newSubDiv = KernelBodySubDiv()
            for index in newSubDiv.bufferPtr.startIndex ..< newSubDiv.bufferPtr.endIndex {
                newSubDiv.bufferPtr[index] = zeroBody
            }
            subDivs.append(newSubDiv)
        }
    }
    
    
}

class KernelAccelSet {
    let _count: Int = 4096
    var subDivs: [KernelAccelSubDiv]
    
    init(setCount: Int) {
        subDivs = []
        let numSubDivs = setCount / _count
        let zeroAccel = Accel(x: 0, y: 0, z: 0)
        for _ in 0..<numSubDivs {
            let newSubDiv = KernelAccelSubDiv()
            for index in newSubDiv.bufferPtr.startIndex ..< newSubDiv.bufferPtr.endIndex {
                newSubDiv.bufferPtr[index] = zeroAccel
            }
            subDivs.append(newSubDiv)
        }
    }
    
    func clear() {
        for subDiv in subDivs {
            subDiv.clear()
        }
    }
    
}





















//
