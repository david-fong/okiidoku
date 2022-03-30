

- https://github.com/vduan/parallel-sudoku-solver
  - a project implementing data-parallel backtracking?

- https://community.khronos.org/t/backtracking-algorithm-and-opencl/1627
  - gpu threading not good for algorithms that don't have high data-parallelism. backtracking algorithm has lots of control flow branching. ("instruction divergence" / "kernel divergence")

- https://www.khronos.org/opencl/assets/CXX_for_OpenCL.html
- https://www.nersc.gov/assets/pubs_presos/MattsonTutorialSC14.pdf
- [Docs on OpenGL and emscripten](https://emscripten.org/docs/porting/multimedia_and_graphics/OpenGL-support.html). I can't find much about OpenCL and emscripten. I found that there's a WebCL, but according to wikipedia, no major browsers support it, and instead, there's a future WebGPU api being worked on? If I want to port to the web with GPU parallelization, I will probably need to look into trying to do it with GLSL shaders or something like gpu.js or turbo.js.