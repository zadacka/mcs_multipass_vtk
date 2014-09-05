Hello, and welcome the Alex's VTK Rift project.

To build the code, you will need to have VTK 5.10. 
...the exact version number is specified in the CMakeLists.txt
...VTK 6.x won't work due to deprecation of key functions

If the dependencies are met, then you should be ready to go!

Please use an out-of-source build as follows:
mkdir build && cd build
cmake .. 
make 

You will find that the executable in ./build/src:
cd src/
./main


Other things to note:
* btain.vtk content is huge, so has been removed. 
* the shaders currently require GLSL 440 / OpenGL4.4
* current build works on Ubuntu 14.4
