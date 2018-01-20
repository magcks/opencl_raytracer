# OpenCL Raytracer
[![Build Status](https://travis-ci.org/magcks/opencl_raytracer.svg?branch=master)](https://travis-ci.org/magcks/opencl_raytracer)
An OpenCL raytracer that renders triangle meshes in OFF format.
## Build instructions
```bash
git clone https://github.com/magcks/opencl_raytracer
cd opencl_raytracer
mkdir build && cd $_
cmake ..
make -j
```
## Example
```bash
./render ../meshes/bunny.off out.pgm
```
## License
This software is licensed under the GPL 3.0 license included as `LICENSE.md`. The authors are:
- kdex ([@kdex](https://github.com/kdex))
- Max von Buelow ([@magcks](https://github.com/magcks))
- Graphics, Capture and Massively Parallel Computing, TU Darmstadt ([GCC](https://www.gcc.tu-darmstadt.de))