# Blur

### Overview
This is a sample project that showcases blurring an image with opengl, on windows and mac.
It has a Visual Studio and an Xcode project, referencing a common code base composed of the following modules:

* **main**: Platform dependent driver code.
* **app**: App control, called by *main*, it defines app logic independent of the platform. In this case, it makes use of the graphics module to blur an image.
* **graphics**: Graphics utilities backed by opengl 2.0.
* **images**: Image processing backed by STB.

### Usage
```
blur.exe <image_filename>
```
