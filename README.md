Skeleton Visualization
---------------
Scientific visualization is one of the most important topics of computer graphics. Accurate representation of measurements or calculation results is crucial for interpreting the data and developing accurate mathematical models and theories, as well as prototypes in practice. Numerous applications are in disciplines such as medical image processing, mechanics of particles, fluid dynamics, molecular biology, protein structure examination, etc. In many cases the obtained results can be understood and well represented only by visualization (either 2-D or 3-D). An example of this is the segmentation and skeletonization of blood vessels in medical imaging. An example of this is illustrated in Fig. 1, where the blood vessels are represented by tubes with different color, which indicates different value of radius at that point. This simplified data is much easier to interpret.

![alt tag](http://i.imgur.com/FsPsYHs.jpg)
![alt tag2](http://i.imgur.com/CxYiCRO.jpg)

The goal of the project
---------------
The goal of the project is to construct a skeleton (as in Fig. 1) from skeleton image (image obtained by skeletonization the segmented image). For this purpose the Visualization Toolkit (VTK) will be used. VTK is an open-source object-oriented library for visualization in C++, Tcl, Python, Java, which is at higher level than other common rendering libraries (like OpenGL). 

The data set supplied represents a part of blood vessel structure. One data set is the original data (“vessels_data.vtk”), the second is a segmented blood vessel tree (“vessels_seg.vtk”), and the third is the skeleton image (“vessels_skel.vtk”) of the segmented data (segmented and skeleton data are binary, which means that pixels with value 0 are background and pixels with any other value represent the object). 

NOTICE: the interaction in VTK is implemented using keys. You have free choice of keys. In other words, to call certain functionality you can use any key you wish (however, what each of the keys does will have to be printed on the screen menu). 


Requirements
----------------
There should be placed 3 .vtk files in the main folder (for MacOS) or in 'build' folder (for Windows)
- vessels_data.vtk
- vessels_seg.vtk
- vessels_skel.vtk