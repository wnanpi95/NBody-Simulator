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
   In our original project, we used CUDA to in order to accelerate our computations so that they could be completed in reasonable time
  
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



