# pathtracer
a simple Monte Carlo Pathtracer with qt4 GUI in C++.

## Features
 - Mesh Rendering
 - Obj Model Support
 - Transform Hierarchy in Scene Management

## Compiling
Install clang-openmp
```
    brew tap homebrew/boneyard
    brew install clang-mp
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