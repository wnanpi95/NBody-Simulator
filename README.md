# NBody-Simulator

  NBody code + 3D Rendering (w/ OpenGL)

  ## 0.0 Intro
   
   Hello! Welcome to the repo for my first programming project! This project is a derivative from a school project, where myself and three other students generate a simulation that could explain the current state of what we call the Mice Galaxies. Our simulation was based on the Toomre & Toomre paper on Galactic Bridges and Tails (https://courses.physics.ucsd.edu/2017/Winter/physics141/final/toomre.pdf). However, the focus of this repo will be on my own personal contributions, the challenges I faced, and changes and additions I made after the project was over. Thanks for taking a look! Many of parts of the code are probably implemented using very naive methods, and if you have advice on better implementations on any part of the code, I would love to hear it! You can contact me at wnan.pi.95@gmail.com.

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
   2. Each galaxy could have a net velocity in any direction
   3. Each galaxy could be rotated an arbitrary amount along an arbitrary axis
  
  In order to accomplish this, I used a 
   
   ### 2.4 Batch Initialization
   
   An amazing resource I learned a ton from was a talk by Simon Gladman (linked at bottom), who discussed the entire pipeline of using Metal to render and calculate the trajectory of many particles. Metal is a low level graphics api, and I faced a lot of frustrations using it until I found this talk. I won't go into too much detail about the fundamentals of the implementation, as Simon does a very good job talking about it. Instead, I will talk about how I adapted it to my use.
   
   In Simon's implementation, he calculated the movement of 4096 particles. I believe that this is the maximum number of threads that can be run in one batch on the iphone GPU. However, I would like to be able to use this code on an arbitrary number of particles.   
   
  ## 3.0 Graphical Rendering (OpenGL)
  
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



