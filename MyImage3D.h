/**************************************************************************
 3D Image class for unsigned short (16-bit unsigned) values

 Author: Danilo Babin
 File name: "MyImage3D.h"
***************************************************************************/

#ifndef MY_IMAGE_3D
	#define MY_IMAGE_3D

#define vtkRenderingCore_AUTOINIT 4(vtkInteractionStyle, vtkRenderingFreeType, vtkRenderingFreeTypeOpenGL, vtkRenderingOpenGL)
#define vtkRenderingVolume_AUTOINIT 1(vtkRenderingVolumeOpenGL)

#include <assert.h>
#include <vtkSmartPointer.h>
#include <vtkImageData.h>
#include <string>

// VTK files
#include <vtkPolyDataMapper.h>
#include <vtkStructuredPointsReader.h>
#include <vtkImageDataGeometryFilter.h>
#include <vtkActor.h>
#include <vtkContourFilter.h>

using namespace std;

enum VesselFile {
	None,
	Data,
	Segmented,
	Skeleton,
	Loading
};

class MyImage3D
{
	vtkSmartPointer<vtkStructuredPointsReader> reader;
	vtkSmartPointer<vtkImageDataGeometryFilter> geometryFilter;
	vtkSmartPointer<vtkPolyDataMapper> mapper;
	vtkSmartPointer<vtkActor> actor;

	vtkSmartPointer<vtkContourFilter> contourFilter;
	vtkSmartPointer<vtkPolyDataMapper> polydataMapper;

	public:
		vtkSmartPointer<vtkImageData> vtk_image_data;
		bool dataLoading;
		VesselFile currentVessel;

		MyImage3D()
		{
			vtk_image_data = vtkSmartPointer<vtkImageData>::New();

			currentVessel = None;
			dataLoading = false;

			// Data VTK file
			reader = vtkSmartPointer<vtkStructuredPointsReader>::New();
			geometryFilter = vtkSmartPointer<vtkImageDataGeometryFilter>::New();
			mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
			actor = vtkSmartPointer<vtkActor>::New();

			// Segmented VTK file
			contourFilter = vtkSmartPointer<vtkContourFilter>::New();
			polydataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
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

		// Loading VTK files
		vtkSmartPointer<vtkActor> LoadDataImage();
		vtkSmartPointer<vtkActor> LoadSegmentedImage();
		vtkSmartPointer<vtkActor> LoadSkeletonImage();
};

#endif