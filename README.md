# NBody-Simulator

  NBody code + 3D Rendering (w/ OpenGL)

  ## 0.0 Intro
   
   Hello! Welcome to the repo for my first programming project! This project is a derivative from a school project, where myself and three other students generate a simulation that could explain the current state of what we call the Mice Galaxies. Our simulation was based on the Toomre & Toomre paper on Galactic Bridges and Tails (https://courses.physics.ucsd.edu/2017/Winter/physics141/final/toomre.pdf). However, the focus of this repo will be on my own personal contributions, the challenges I faced, and changes and additions I made after the project was over. Thanks for taking a look! Many of parts of the code are probably implemented using very naive methods, and if you have advice on better implementations on any part of the code, I would love to hear it! You can contact me at wnan.pi.95@gmail.com. 
   
   At the moment, this repo is a series of incohesive code fragments, which may be cleaned up at a later time.

  ## 1.0 Model Generation
   
   The galaxy model we used was generated using Kuijken and Dubinski's model for a stable galaxy, using their galactics package (https://courses.physics.ucsd.edu/2017/Winter/physics141/final/galactics.tar.gz). Our models used 16k particles.

  ## 2.0 Simulation Computation
   
   ### 2.1 Algorithm Used
   
   The algorithm we used to simulate the evolution of our galaxies was the simple Verlet/ Leapfrog algorithm
   
   ### 2.2 Computation Overview
   
   In our original project, we used CUDA to in order to accelerate our computations so that they could be completed in reasonable time ( O(n) vs. O(n^2), for n = 32_000 ). Unfortunately, Apple chose Intel over Nvidia GPUs, thus my 2013 macbook air does not support the CUDA platform. However, Apple created a fairly new graphics api, Metal, which includes support for compute kernels. In this repo we will explore using this option. Additionally, I have created a basic implementation of the Barnes-Hut algorithm, which uses an approximation method using oct-trees, and has a complexity of O(log(n)), which I will briefly talk about at the end of this section.
   
   ### 2.3 Set Initialization
   
   As I mentioned above, I used the galactics package in order to generate galaxies of 16k particles. In order to be able to simulate any sort of galaxy collision, I need to be able to generate a set of initial conditions that:
   
   1. Contained an arbitrary number of galaxies
   2. Each galaxy could have any arbitrary initial position
   3. Each galaxy could have a net velocity in any direction
   4. Each galaxy could be rotated an arbitrary amount along an arbitrary axis
  
  In order to accomplish this, I used a method commonly used in computer graphics. I won't go into too much detail, but essentially, a set of affine transformation matrices (using homogeneous coordinates) was generated per galaxy, based on the requested initial position, initial velocity, rotation magnitude, and rotation axis. Each set of matrices was then concatenated, and applied to our original galaxy data. We now have our requested initial conditions.
   
   ### 2.4 Batch Initialization
   
   An amazing resource I learned a ton from was a talk by Simon Gladman (linked at bottom), who discussed the entire pipeline of using Metal to render and calculate the trajectory of many particles. Metal is a low level graphics api, and I faced a lot of frustrations using it until I found this talk. I won't go into too much detail about the fundamentals of the implementation, as Simon does a very good job talking about it. Instead, I will talk about how I adapted it to my use.
   
   In Simon's implementation, he calculated the movement of 4096 particles. I believe that this is the maximum number of threads that can be run in one batch on the iphone GPU. However, I would like to be able to use this code on an arbitrary number of particles. In order to do this, I created several classes, that separate our initial conditions generated above into sets of 4096 particles. Additionally, massless particles were inserted into the set as needed in order to create a set that contained a multiple of 4096 particles. This was done in order to avoid partially filled sets that would include garbage in the memory allocation. The main workload of these classes is to automate the process of allocating shared memory for each set between the CPU and GPU in order to avoid the slow process of copying memory back and forth. (At the moment this is not an advantage being used, as the rendering is generated with OpenGL in a separate program. In the future I may write the code and shaders necessary for a graphical rendering using Metal). 
   
   In order to save memory space, a set of three shuffling buffer sets were created (one to hold current position, one to hold former position, and one to be written into to update the position). This was done in order to save on allocated memory space (I believe the 2013 macbook air only has a combined RAM of 4GB between CPU and GPU).
   
   ### 2.5 Compute Kernels
   
   The expensive computation that occurs is the calculation of the gravitational force exerted on each particle by every other particle, which amounts to n(n-1) calculations. The calculations were done in batches of 4096, by passing buffers with the positions and masses of two sets of 4096 particles at a time. Because force obeys the superposition principle, an additional buffer containing an updating force value is passed in as well, and continuously updated (by summation). In reality, this method actually involves a complexity of O((n/4096)(n-1)) -> O(n^2), but for my uses (n ~ [16k, 32k]), it really is essentially O(n). After the graviational forces are fully calculated, the particle position update also occurs on the GPU.
   
   In the future, I would love to render this system in Metal, in order to create a fully cohesive system. However, the rendering is currently power by a separate program written in C++ using OpenGL, and thus the computed position are currently being written into a text file, to be read by this other program.
   
  ## 3.0 Graphical Rendering (OpenGL)
  
  The fun part! Tom Dalling created an awesome series of OpenGL tutorials (linked in credits), where I learned not just about OpenGL, but many of the fundamentals of computer graphics techniques. Again, Tom has already created a great resource, so I will only talk about the specific details to my implementation.
  
  ### Rendering
  
  A key adaptation that I had to implement was a difference in the data being rendered. Tom's example involved one set of vertices 
  
# Credits

Special Thanks to Shane Aldas, Kyle Lewis, and Kelsi Lund, my group members who made invaluable contributions to our original projects for our UCSD class PHYS 141.

## Source used for generation of the galaxy particle distribution
Konrad Kuijken and John Dubinski's Nearly Self-Consistent disc-bulge-halo models for galaxies
https://courses.physics.ucsd.edu/2017/Winter/physics141/final/kuijken-dubinski.pdf

## Tutorial and source of some code used in the OpenGL rendering
Tom Dalling
http://www.tomdalling.com

## Tutorial used in creating the GPU accelerated NBody code
Simon Gladman
https://news.realm.io/news/swift-summit-simon-gladman-metal/



