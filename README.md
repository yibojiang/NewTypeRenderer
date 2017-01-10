# pathtracer
a simple Monte Carlo Pathtracer with qt4 GUI in C++.

## Features
 - Mesh Rendering
 - Transform Hierarchy in Scene Management
 - BVH Structure Acceleration
 - a Progressively Rendering Renderview

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