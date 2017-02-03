# pathtracer
a Monte Carlo Pathtracer with qt4 GUI in C++.

<img width="800" alt="lucy" src="https://cloud.githubusercontent.com/assets/1669501/22495278/f9d75e8e-e80c-11e6-9754-ffb459639cf0.png">`

<img width="800" alt="lucy" src="https://cloud.githubusercontent.com/assets/1669501/22495281/f9e14e6c-e80c-11e6-8f47-9748a8775fdc.png">

<img width="800" alt="lucy" src="https://cloud.githubusercontent.com/assets/1669501/22495283/f9e65a10-e80c-11e6-9100-ca6820dd44b0.png">

<img width="800" alt="lucy" src="https://cloud.githubusercontent.com/assets/1669501/22495280/f9e06970-e80c-11e6-9376-8f9b0fa9c4ba.png">

<img width="800" alt="lucy" src="https://cloud.githubusercontent.com/assets/1669501/22495279/f9dfb2c8-e80c-11e6-92ec-b8d43ac7c32a.png">


## Features
 - Mesh Rendering
 - Transform Hierarchy in Scene Management
 - BVH Structure Acceleration
 - a Progressively Rendering Renderview
 - Mutithread rendering on CPU
 - Scene description Json-format

## Compiling
Install clang-openmp
```
    brew install clang-omp
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

## Usage
- Create models/ foler and textures/ folder in the project path.
- Put all models models folder, textures and mtl in textures folder.
- Create scene json file.
- Load the scene and render.