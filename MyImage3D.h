/**************************************************************************
 3D Image class for unsigned short (16-bit unsigned) values

 Author: Danilo Babin
 File name: "MyImage3D.h"
***************************************************************************/

#ifndef MY_IMAGE_3D
	#define MY_IMAGE_3D

#define vtkRenderingCore_AUTOINIT 4(vtkInteractionStyle,vtkRenderingFreeType,vtkRenderingFreeTypeOpenGL,vtkRenderingOpenGL)
#define vtkRenderingVolume_AUTOINIT 1(vtkRenderingVolumeOpenGL)

#include <assert.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>

class MyImage3D
{
	public:

		vtkSmartPointer<vtkImageData> vtk_image_data;

		MyImage3D()
		{
			vtk_image_data = vtkSmartPointer<vtkImageData>::New();
		};

		//If the image is empty return true (1), else return fail (0).
		int IsEmpty();

		// Creates a new image of given size
		void Set(unsigned int _slices, unsigned int _rows, unsigned int _columns);
		
		// Return the voxel value at index: _slices, _rows, _columns. Note that in (real) world coordinates
		// the slice index corresponds to Z axis, rows to Y axis and columns to X axis.
		unsigned short& Index(unsigned int _slices, unsigned int _rows, unsigned int _columns); 

		// Fill in the image with the given value
		void FillInWith(unsigned short _value);
};

#endif