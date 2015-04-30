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
#include <stack>
#include <vector>

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

// Volume visualization
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>

#include <vtkVolumeRayCastMapper.h>
#include <vtkVolumeRayCastFunction.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkVolumeRayCastIsosurfaceFunction.h>
#include <vtkVolumeRayCastMIPFunction.h>
#include <vtkPiecewiseFunction.h>

#include <vtkLODActor.h>
#include <vtkVolume.h>
#include <vtkMaskPoints.h>

#include <vtkTubeFilter.h>
#include <vtkStructuredPoints.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyLine.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>

using namespace std;

typedef unsigned short ushort;

enum VesselFile {
	NotLoaded,
	Skeleton,
	SkeletonTubed,
	SkeletonColored,
	SkeletonVaryingRadii,
	Volume,
	Segmented,
	SegmentedTransparent
};

// don't change the order, main.cpp:togglePlane depends on it
enum ImagePlane {
	Sagittal,
	Transversal,
	Coronal,
	None,
};

class MyImage3D
{
	vtkSmartPointer<vtkStructuredPointsReader> dataReader, segmReader, skelReader;
	vtkSmartPointer<vtkStructuredPoints> structuredPoints;
	vtkSmartPointer<vtkPolyDataMapper> dataMapper, segmMapper, skelMapper, outlineMapper;
	vtkSmartPointer<vtkActor> dataActor = NULL, segmActor = NULL, skelActor = NULL, outlineActor = NULL, tubedSkeletonActor = NULL;
	vtkSmartPointer<vtkVolume> raycastVolume = NULL;

	// Help functions for the public function GetTubedSkeleton()
	bool isVisited(ushort x, ushort y, ushort z);
	bool isVisited(vector<ushort> * voxel);
	void setVisited(ushort x, ushort y, ushort z);
	void setVisited(vector<ushort> * voxel);
	vector<ushort> makeVector(ushort a, ushort b, ushort c);
	bool copyVoxelValues(vector<ushort> * from, vector<ushort> * to);
	void testPrintingBranch(vector<vector<ushort> >* branch);

	void findVoxelNeighbors(vector<ushort> * currentVoxel, vector<vector<ushort> > * neighbors);
	void findEndOfBranch(vector<ushort> * currentVoxel, vector<ushort> * endOfBranch);
	bool findNextVoxel(vector<ushort> * vox);
	void getBranch(vector<ushort> * currentVoxel, vector<vector<ushort>> * branch);
	vtkSmartPointer<vtkTubeFilter> makeTube(vector<vector<vector<ushort>>> * branches);

	stack<vector<ushort>> voxelsToVisit;
	vector<bool> visited;

	public:

		vtkSmartPointer<vtkImageData> vtk_image_data;
		VesselFile currentVessel;
		ImagePlane currentPlane;

		// Image planes
		vtkSmartPointer<vtkImagePlaneWidget> planes[3];
		int dimensions[3];

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
		ushort& Index(unsigned int _slices, unsigned int _rows, unsigned int _columns); 

		// Fill in the image with the given value
		void FillInWith(ushort _value);

		// Create an actor to control the level of detail in rendering
		vtkSmartPointer<vtkLODActor> SetLOD();

		// Loading VTK files
		vtkSmartPointer<vtkVolume> GetVolume();
		vtkSmartPointer<vtkActor> GetSegmentedImage();
		vtkSmartPointer<vtkActor> GetSkeletonImage();
		vtkSmartPointer<vtkActor> GetSegmentedOutline();
		vtkSmartPointer<vtkActor> GetTubedSkeleton();

		// Return access to the reader, for imagePlanesWidget
		vtkSmartPointer<vtkStructuredPointsReader> GetSegmentedImageReader();
};

#endif