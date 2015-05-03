/**************************************************************************
 3D Image class for unsigned short (16-bit unsigned) values

 Author: Danilo Babin, Eric Klaesson, Damian Tarnacki
 File name: "MyImage3D.h"
***************************************************************************/

#ifndef MY_IMAGE_3D
	#define MY_IMAGE_3D

#define vtkRenderingCore_AUTOINIT 4(vtkInteractionStyle, vtkRenderingFreeType, vtkRenderingFreeTypeOpenGL, vtkRenderingOpenGL)
#define vtkRenderingVolume_AUTOINIT 1(vtkRenderingVolumeOpenGL)

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
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkColorTransferFunction.h>

#include <vtkVolumeRayCastMapper.h>
#include <vtkVolumeRayCastFunction.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkVolumeRayCastIsosurfaceFunction.h>
#include <vtkVolumeRayCastMIPFunction.h>
#include <vtkPiecewiseFunction.h>

#include <vtkLODActor.h>
#include <vtkMaskPoints.h>

#include <vtkTubeFilter.h>
#include <vtkStructuredPoints.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkPolyLine.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkCleanPolyData.h>
#include <vtkSplineFilter.h>

#include <vtkSplineWidget.h>
#include <vtkSplineWidget2.h>
#include <vtkPropPicker.h>

using namespace std;

typedef unsigned short ushort;

enum VesselFile
{
	NotLoaded, Skeleton, SkeletonTubed, SkeletonColored, SkeletonVaryingRadii, Volume, Segmented
};

enum ImagePlane
{
	// don't change the order, main.cpp:togglePlane depends on it
	Sagittal, Transversal, Coronal, None
};

class MyImage3D
{
	vtkSmartPointer<vtkStructuredPointsReader> dataReader, segmReader, skelReader;
	vtkSmartPointer<vtkStructuredPoints> structuredPoints;
	vtkSmartPointer<vtkPolyDataMapper> dataMapper, segmMapper, skelMapper, outlineMapper;
	vtkSmartPointer<vtkActor> dataActor = NULL, segmActor = NULL, skelActor = NULL, outlineActor = NULL, tubedSkeletonActor = NULL;
	vtkSmartPointer<vtkVolume> raycastVolume = NULL;
	vtkSmartPointer<vtkLODActor> lodActor = NULL;
	vector<vector<vector<ushort>>> branches;

	// Help functions for the public function GetTubedSkeleton()
	void getImageData();
	bool findNextVoxel(vector<ushort> * vox);
	void findEndOfBranch(vector<ushort> * currentVoxel, vector<ushort> * endOfBranch);
	void findVoxelNeighbors(vector<ushort> * currentVoxel, vector<vector<ushort> > * neighbors);
	void getBranch(vector<ushort> * currentVoxel, vector<vector<ushort>> * branch);
	
	vtkSmartPointer<vtkPolyData> makePolyData(vector<vector<ushort> > * branch);
	vtkSmartPointer<vtkTubeFilter> makeTube(vtkSmartPointer<vtkPolyData> polyData, double radius, bool makeSmooth);
	
	// Additional helper functions
	bool copyVoxelValues(vector<ushort> * from, vector<ushort> * to);
	vector<ushort> makeVector(ushort a, ushort b, ushort c);
	bool isVisited(vector<ushort> * voxel);
	bool isVisited(ushort x, ushort y, ushort z);
	void setVisited(vector<ushort> * voxel);
	void setVisited(ushort x, ushort y, ushort z);

	stack<vector<ushort>> voxelsToVisit;
	vector<bool> visited;

	public:

		VesselFile currentVessel;
		ImagePlane currentPlane;

		// Image planes
		vtkSmartPointer<vtkImagePlaneWidget> planes[3];
		int dimensions[3];
		double tubeRadius = 0.2;

		MyImage3D()
		{
			currentVessel = NotLoaded;
			currentPlane = None;
		};

		// Loading VTK files
		vtkSmartPointer<vtkVolume> GetVolume();
		vtkSmartPointer<vtkActor> GetSegmentedImage();
		vtkSmartPointer<vtkActor> GetSegmentedOutline();
		vtkSmartPointer<vtkActor> GetSkeletonImage();
		vector<vtkSmartPointer<vtkActor>> GetTubedSkeleton(double tubeRadius, bool varyTubeRadiusByScalar, bool colorByScalar);

		// Return access to the reader, for imagePlanesWidget
		vtkSmartPointer<vtkStructuredPointsReader> GetSegmentedImageReader();
};

#endif