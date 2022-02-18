- https://github.com/vduan/parallel-sudoku-solver
- https://community.khronos.org/t/backtracking-algorithm-and-opencl/1627
  - gpu threading not good for algorithms that don't have high data-parallelism. backtracking algorithm has lots of control flow branching. ("instruction divergence" / "kernel divergence")
- https://www.khronos.org/opencl/assets/CXX_for_OpenCL.html
- https://www.nersc.gov/assets/pubs_presos/MattsonTutorialSC14.pdf
- [Docs on OpenGL and emscripten](https://emscripten.org/docs/porting/multimedia_and_graphics/OpenGL-support.html). I can't find much about OpenCL and emscripten. I found that there's a WebCL, but according to wikipedia, no major browsers support it, and instead, there's a future WebGPU api being worked on? If I want to port to the web with GPU parallelization, I will probably need to look into trying to do it with GLSL shaders or something like gpu.js or turbo.js.

- Go back and try the old canonicalization by rel row prob, but break ties by doing some brute force: try each tied permutation and valuate it according to some reduction of how it pushes rarer rel counts to the top left. Just be careful to shift to get rid of the main diagonal seam.
  ```
  count[rel] = 
  exp[rel] = i + j
  scale[rel] = 1 - (p_binomial(count[rel]) ^ 1/O)
  score[rel] = 2 ^ (exp[rel]) * scale[rel]
  find labelling with maximum
    ```