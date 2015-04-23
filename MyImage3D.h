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
#include <vtkOutlineFilter.h>
#include <vtkProperty.h>

// Image planes
#include <vtkImagePlaneWidget.h>

// Ray cast
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>

#include <vtkVolumeRayCastMapper.h>
#include <vtkVolumeRayCastFunction.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkVolumeRayCastIsosurfaceFunction.h>
#include <vtkVolumeRayCastMIPFunction.h>

#include <vtkLODActor.h>
#include <vtkVolume.h>

using namespace std;

enum VesselFile {
	NotLoaded,
	Data,
	Segmented,
	Skeleton,
	RayCast,
	Loading
};

enum ImagePlane {
	Sagittal,
	Transversal,
	Coronal,
	None,
};

class MyImage3D
{
	vtkSmartPointer<vtkStructuredPointsReader> dataReader, segmReader, skelReader;
	vtkSmartPointer<vtkPolyDataMapper> dataMapper, segmMapper, skelMapper, outlineMapper;
	vtkSmartPointer<vtkActor> dataActor = NULL, segmActor = NULL, skelActor = NULL, outlineActor = NULL;
	vtkSmartPointer<vtkVolume> raycastVolume = NULL;

	public:

		vtkSmartPointer<vtkImageData> vtk_image_data;
		VesselFile currentVessel;
		ImagePlane currentPlane;

		// Image planes
		vtkSmartPointer<vtkImagePlaneWidget> planes[3];

		MyImage3D()
		{
			vtk_image_data = vtkSmartPointer<vtkImageData>::New();
			currentVessel = NotLoaded;
			currentPlane = None;
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

		// Create an actor to control the level of detail in rendering
		vtkSmartPointer<vtkLODActor> SetLOD();

		// Loading VTK files
		vtkSmartPointer<vtkActor> GetDataImage();
		vtkSmartPointer<vtkVolume> GetRayCastingImage();
		vtkSmartPointer<vtkActor> GetSegmentedImage();
		vtkSmartPointer<vtkActor> GetSkeletonImage();
		vtkSmartPointer<vtkActor> GetSegmentedOutline();

		vtkSmartPointer<vtkStructuredPointsReader> GetSegmentedImageReader();
};

#endif