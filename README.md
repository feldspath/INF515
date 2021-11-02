# INF515 project: discretized signed distance fields in OpenGL.

Author: Marius Debussche and Clément Jambon

This project is the application of our research in the domain of signed distance fields rendering. 
We first implemented a basic ray marching algorithm on the GPU, and we now try to find efficient ways to reconstruct a signed distance field that has been discretized.

This project is currently work in progress and should continue until March 2021.

## Build the project

To build the project, execute the following commands:

```
mkdir build && cd build
cmake ..
make
```

The executable file is named `ray-tracing-tutorial`.
