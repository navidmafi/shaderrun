# ShaderRun

![](.github/demo1.jpg)

### Build:
```sh
git clone --recursive https://github.com/navidmafi/shaderrun

cd shaderrun

#---
# Windows 
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw.cmake

# Linux
cmake -B build 
#---

cmake --build build

# Run the included demo

./build/srun 1.frag

```
