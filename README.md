### MacOs OpenGL bootstrap

![capture](./capture.png)

before to build

```bash
brew install glfw3 glew glm freetype
```

### linux-deb bootstrap

before to build

```bash
sudo apt-get install libx11-dev mesa-common-dev libglu1-mesa-dev libglm-dev libglfw3-dev libglew-dev libfreetype6-dev
```

### windows

NOT WORK

### dependencies

- CoreVideo (only MacOS)
- Cocoa (only MacOS)
- IOkit (only MacOS)
- OpenGL
- glew
- glfw
- glm

### TODO

- multiple directional light shadow
- multiple spot light shadow
- CSM (cascade shadow map)
- PSSM (parallel-split shadow map)
- SSAO (screen space ambient occlusion
- deferred rendering
- parallex occlusion mapping
