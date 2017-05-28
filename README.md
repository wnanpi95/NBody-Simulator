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
   
   ### 2.6 Barnes-Hut version (alternate)
   
   I won't explain the algorith here (great resource is http://www.cs.princeton.edu/courses/archive/fall03/cs126/assignments/barnes-hut.html). Just an interesting program that I thought would be fun to create, and runs fully on the CPU.
   
  ## 3.0 Graphical Rendering (OpenGL)
  
  The fun part! Tom Dalling created an awesome series of OpenGL tutorials (linked in credits), where I learned not just about OpenGL, but many of the fundamentals of computer graphics techniques. Again, Tom has already created a great resource, so I will only talk about the specific details to my implementation.
  
 ### 3.1 Creating the Vertex Buffer
  
   A key adaptation that I had to implement was a difference in the data being rendered. Tom's example involved one set of vertices, and used instancing in order to create many objects (in his case, boxes). However, the data I used required a continuous updating of vertex positions based on the text file generated by the computation. After some trial and error (including accidently generating new vertex buffers every frame, leading to a massive memory leak about the size of the data set per frame, oops), I was finally able to generate a reasonable solution, writing a separate function in order to continuously update the buffer rather than generate a new one each frame. 
  
 ### 3.2 Shaders
  
   I won't go into to much detail about the shaders, again Tom has a great writeup. I will talk a bit about some changes I had to make.
  
  First, because of the nature of the rendering, I chose to use sprites rather than genuine three dimensional objects. The first major change occurs in the vertex shader. I thought it was important that particle size should change based on distance from camera. Using a bit of trig, the shader updates the particles size based on distance from the camera and the angle of the field of view.
  
  The rendered shape for a point particle in OpenGl is a square. To create our spherical, "star-looking" objects, I first discard all of the pixels greater than a constant distance from the center of the point of each sprite, in order to create a circle. Next, in order to create the effect of a spherical object, I used the Blinn-Phong lighting model. Again, Tom explains all of the concepts very well in his tutorial, but essentially uses the summation of ambient, diffuse, and specular lighting to generate the illusion of shape. Finally, a bit of alpha blending was used to create a ghostly, transparent effect on the stars.
  
  Uh oh. Due to the tranparency, there are now ugly artifacts that appear when the viewing has overlapping stars
  
### 3.3 Fixing the artifacts
  
 Perhaps the most challenging and frustrating part of the process. Many hours were spent figuring out the problem, as well as trying many different solutions.
  
 Unfortunately, artifacts due to transparency were out of the scope of Tom's tutorial. After some time examining the rendering for some time, from many different angles, I had the insight that perhaps it had to do with the order that the particles were being drawn, as the artifacts only appeared from some viewing angles. 
  
 With the problem now identified, I could properly explore solutions. The first attempt was using a painter's algorithm, which involves sorting the particles by their distance from the camera, and rendering the further ones first. Using a naive implementation, there issues with particles disappearing and reappearing. The next attempt involved turning off depth testing, which culls pixels that are "behind" already drawn pixels. While this looked significantly better than before, it created strange effects at certain angles that destroyed the illusion of a three dimensional object.
  
  The final solution I realized had to come from using the painter's algorithm, so I began a thorough investigation of the issue. After some more time investigating the rendering, I noticed an interesting effect. The rendering would come out perfectly in a small ring around the center of wherever the camera pointed. I then remembered that the view was rendered using a projected frustum rather than an orthogonal projection (for more details, visit Tom's blog), which was creating an "inverted" layer when using the naive painter's algorithm. After some thought, I realized the issue could be fixed by sorting the rendering order by the distance of the particles from the camera, projected on the camera's viewing ray (dot product of separation vector between camera and particle, and direction camera is pointing), as this sorting would preserve the proper order after undergoing the projection transformation. There is an unfortunate bottle neck in rendering time involving the sorting everytime the camera is shifted now, but it finally came out the way I envisioned!
  
### 3.4 Miscellaneous features
  
This part was a lot of fun for me. Here I explored the features of the GLFW windowing system, and had a lot of fun creating various features that really brought the project to another level, with comparatively smaller effort. A wonderful feature was being able to assign actions to various keys, which allowed real time input on the simulation
    
#### 3.41 Color Mapping
    
A very fun feature was being able to map "color modes" to various number keys. At the press of certain numbers, the particles in the system could be colored by various modes. One example was being able to color the particles by the magnitude of the gravitational acceleration exerted on it. This allows viewers to be able to visualize the tidal forces exerted across a galactic system when passing another, a very important phenomena in the final state of the collision. Another color mode allows different coloring for individual galaxies, in order to track the origination of various particles, allowing viewers to observe the blending and cannibalization that occurs during collision. Similarly, another mode allows the highlighting of individual galaxies, so that the full evolution of a particular galaxy could be observed.
   
#### 3.42 Time Indexing
   
The most tactilely satisfying feature was mapping control of the time index onto keys, allowing real time control of the simulation. Mapping the actions of run, stop, forward, and backward onto keys, allowed complete control of the viewing of the time evolution of the system. and with the freedom of the camera, allowed full freedom to analyze interesting configurations from various angles in real time. 
  
#### 3.43 Real Time Control (just for fun)

A very fun (but not particularly useful) feature was mapping full control of a massive object to keys, and being able to mess with and interact with the system in real time.

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



