# pathtracer
a Monte Carlo Pathtracer with qt4 GUI in C++.

<img width="800" alt="gui_2" src="https://cloud.githubusercontent.com/assets/1669501/21815040/5547d0ea-d729-11e6-954c-6841ae3f0c9b.png">

## Features
 - Mesh Rendering
 - Transform Hierarchy in Scene Management
 - BVH Structure Acceleration
 - a Progressively Rendering Renderview
 - Mutithread rendering on CPU

## Compiling
Install clang-openmp
```
    brew tap homebrew/boneyard
    brew install clang-mp
```

Install libiomp
```
    brew install libiomp
```

Install QT4
```
    brew install qt4
```

To build the source
```
    qmake tracer.pro
    make
```

Run
```
    ./tracer.app/Contents/MacOS/tracer
```