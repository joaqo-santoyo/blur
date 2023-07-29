# Blur

### Overview
This is a sample project that showcases blurring an image with opengl, on windows and mac.
It has a Visual Studio and an Xcode project, referencing a common code base composed of the following modules:
* **main**: Platform dependent driver code.
* **app**: App control, called by *main*, it configures shaders, resources, the renderer, and in general defines app logic independent of the platform.
* **graphics**: A simple renderer, working with a queue of render passes, backed by opengl 2.0.
* **images**: Image processing, backed by STB.

### Usage
```
blur.exe <image_filename>
```