#include <iostream>
#include <sstream>

#include <vtkPolyData.h>
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>

// textActor
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

// key pressed
#include <vtkCommand.h>
#include <vtkCallbackCommand.h>

// image planes
#include <vtkCellPicker.h>
#include <vtkStructuredPointsReader.h>
#include <vtkStructuredPoints.h>

// point Picker
#include <vtkPointPicker.h>
#include <vtkRendererCollection.h>
#include <vtkObjectFactory.h>

#include "MyImage3D.h"
#include "functions.h"

using namespace std;

// VTK window
vtkSmartPointer<vtkRenderer> renderer = NULL;
vtkSmartPointer<vtkRenderWindow> renderWindow;
vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;
vtkSmartPointer<vtkImagePlaneWidget> planeWidget;

vtkSmartPointer<vtkLODActor> lodActor = NULL;
vtkSmartPointer<vtkVolume> volume;
vtkSmartPointer<vtkActor> segmActor, outlineActor;
vtkSmartPointer<vtkActor> skelActor, skelTubedActor, skelColoredActor, skelVaryingRadiiActor;

// Menu
vtkSmartPointer<vtkTextActor> menuCommands, menuVessels, menuLoading, menuDistance;

// Configuration
const int windowSizeX = 1000;
const int windowSizeY = 700;

// Flags
bool menuInfoVisible = false;
bool loadingData = false;
bool segmentedTransparentVisible = false;

// Drawing a line for distance
bool startSet = false;
bool stopSet = false;

// Menu commands
int infoCurrentPage = 0;
string infoCommands[] = {
	"[i] see available commands",
	"# Volume rendering:\n[+/-] rotate view\n\n[i] more commands",
	"# Segmented image:\n[s] sagittal view\n[t] transversal view\n[c] coronal view\n[+/-] scroll through the slices\n\n[i] more commands",
	"# Skeleton image:\n[0] transparent segmented mesh\n[click] compute distance between points\n[x] reset selected points\n\n[i] more commands",
	"# Other:\n[z] reset zoom\n[arrows] move mesh\n[e/q] exit\n\n[i] close commands info"
};
int infoCommandsSize = sizeof(infoCommands) / sizeof(infoCommands[0]);

MyImage3D image;

double distanceStart[3];
double distanceStop[3];

// Define interaction style
// Ref: http://www.vtk.org/Wiki/VTK/Examples/Cxx/Interaction/PointPicker
class MouseInteractorStyle : public vtkInteractorStyleTrackballCamera
{
	public:
		static MouseInteractorStyle* New();
		vtkTypeMacro(MouseInteractorStyle, vtkInteractorStyleTrackballCamera);
 
		virtual void OnLeftButtonDown()
		{
			if (image.currentVessel == Volume)
			{
				renderWindowInteractor->SetDesiredUpdateRate(20); // Update rate in moving the camera view
				renderWindowInteractor->SetStillUpdateRate(20); // Update rate in the still view
				vtkInteractorStyleTrackballCamera::OnLeftButtonDown(); // forward events
				return;
			}
		}
 
		virtual void OnLeftButtonUp() 
		{
			if (image.currentVessel == Volume)
			{
				renderWindowInteractor->SetDesiredUpdateRate(15); // Default value == 15
				renderWindowInteractor->SetStillUpdateRate(0.0001); // Default value == 0.0001
				vtkInteractorStyleTrackballCamera::OnLeftButtonUp(); // forward events
				return;
			}

			// only for skeleton images
			if (!isSkeleton(image.currentVessel)) {
			
				vtkInteractorStyleTrackballCamera::OnLeftButtonUp(); // forward events
				return;
			}

			// pick a point
			this->Interactor->GetPicker()->Pick(
				this->Interactor->GetEventPosition()[0], 
				this->Interactor->GetEventPosition()[1],
				this->Interactor->GetEventPosition()[2],
				this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer()
			);

			// get value of picked point
			double picked[3];
			this->Interactor->GetPicker()->GetPickPosition(picked);
			//cout << "Picked value: " << picked[0] << " " << picked[1] << " " << picked[2] << std::endl;

			// if not empty = belongs to the skeleton
			if (!isEmpty(picked))
				addToDistance(this->Interactor->GetEventPosition());

			// forward events
			vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
		}
};

vtkStandardNewMacro(MouseInteractorStyle);

int main(int, char *[])
{
	initVTK();
	//setLOD(20);
	
	// The first one to load
	loadFile(SkeletonTubed);

	renderVTK();

	return EXIT_SUCCESS;
}

void initVTK()
{
	renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderer = vtkSmartPointer<vtkRenderer>::New();

	renderer->SetBackground(1.0, 1.0, 1.0);
	
	renderWindow->AddRenderer(renderer);
	renderWindow->SetSize(windowSizeX, windowSizeY);
	
	// Handle key press
	vtkSmartPointer<vtkCallbackCommand> keypressCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	keypressCallback->SetCallback(KeypressCallbackFunction);
	renderWindowInteractor->AddObserver(vtkCommand::KeyPressEvent, keypressCallback);
	
	// Handle mouse events
	vtkSmartPointer<MouseInteractorStyle> style = vtkSmartPointer<MouseInteractorStyle>::New();
	renderWindowInteractor->SetInteractorStyle(style);

	// Prepare menu
	prepareMenu();

	renderWindowInteractor->SetRenderWindow(renderWindow);
}

void renderVTK()
{
	renderWindow->Render();
	renderWindowInteractor->Start();
}

/*
 * Prepare menu and info messages
 */
void prepareMenu()
{
	menuCommands = vtkSmartPointer<vtkTextActor>::New();
	menuVessels = vtkSmartPointer<vtkTextActor>::New();
	menuLoading = vtkSmartPointer<vtkTextActor>::New();
	menuDistance = vtkSmartPointer<vtkTextActor>::New();

	int menuPositionX = 10;
	int menuPositionY = 10; // padding from bottom
	
	menuCommands->GetTextProperty()->SetFontFamilyToCourier();
	menuCommands->GetTextProperty()->SetFontSize(14);
	menuCommands->GetTextProperty()->SetColor(0.0, 0.0, 0.0);
	menuCommands->SetDisplayPosition(menuPositionX, menuPositionY);
	menuCommands->SetInput(infoCommands[infoCurrentPage].c_str());
	renderer->AddActor(menuCommands);
	
	menuVessels->GetTextProperty()->SetFontFamilyToCourier();
	menuVessels->GetTextProperty()->SetFontSize(14);
	menuVessels->GetTextProperty()->SetColor(0.0, 0.0, 1.0);
	menuVessels->SetDisplayPosition(menuPositionX + 300, menuPositionY);
	menuVessels->SetInput("skeleton: [1] basic [2] tubed [3] colored [4] varying radii\n          [5] volume [6] segmented");
	renderer->AddActor(menuVessels);
	
	menuLoading->GetTextProperty()->SetFontFamilyToCourier();
	menuLoading->GetTextProperty()->SetFontSize(14);
	menuLoading->GetTextProperty()->SetColor(1.0, 0.0, 0.0);
	menuLoading->SetDisplayPosition(menuPositionX + 300, menuPositionY + 40);
	menuLoading->SetInput("Loading, it may take a while...");

	menuDistance->GetTextProperty()->SetFontFamilyToCourier();
	menuDistance->GetTextProperty()->SetFontSize(14);
	menuDistance->GetTextProperty()->SetColor(0.0, 0.0, 0.0);
	menuDistance->SetDisplayPosition(menuPositionX + 800, menuPositionY);
	menuDistance->SetInput(" ");
	renderer->AddActor(menuDistance);
}

/*
 * Handles loading different vessels on user's key press
 */
void loadVessels(VesselFile next)
{
	// already loading || multiple loads of the same
	if (loadingData || image.currentVessel == next)
		return;

	toggleLoading(); // Loading on

	// hide planes if any visible before changing to volume/skeleton view
	togglePlane(image.currentPlane);
	
	// don't remove segmented if switching between skeletons
	if ((segmentedTransparentVisible && !isSkeleton(next)) || image.currentVessel == Segmented)
		renderer->RemoveActor(segmActor);

	if (isSkeleton(image.currentVessel) && !isSkeleton(next))
		segmentedTransparentVisible = false;

	// remove actors and/or volumes
	renderer->RemoveActor(outlineActor);
	renderer->RemoveVolume(volume);

	// remove skeletons only if next one is volume/segmented
	renderer->RemoveActor(skelActor);
	renderer->RemoveActor(skelTubedActor);
	renderer->RemoveActor(skelColoredActor);
	renderer->RemoveActor(skelVaryingRadiiActor);

	loadFile(next); // Load file

	toggleLoading(); // Loading off
}

/*
 * Loads requested file and changes window title
 */
void loadFile(VesselFile type)
{
	image.currentVessel = type;

	switch (type)
	{
		case Skeleton:
			renderWindow->SetWindowName("Skeleton Visualization - Skeleton");
			skelActor = image.GetSkeletonImage();
			renderer->AddActor(skelActor);
			break;

		case SkeletonTubed: 
			renderWindow->SetWindowName("Skeleton Visualization - Skeleton as tubes");
			skelTubedActor = image.GetTubedSkeleton(image.tubeRadius, false, false);
			renderer->AddActor(skelTubedActor);
			break;

		case SkeletonColored:
			renderWindow->SetWindowName("Skeleton Visualisation - Colored skeleton");
			skelColoredActor = image.GetTubedSkeleton(image.tubeRadius, false, true);
			renderer->AddActor(skelColoredActor);
			break;

		case SkeletonVaryingRadii:
			renderWindow->SetWindowName("Skeleton Visualisation - Skeleton with varying tube radii");
			skelVaryingRadiiActor = image.GetTubedSkeleton(image.tubeRadius, true, false);
			renderer->AddActor(skelVaryingRadiiActor);
			break;

		case Volume:
			renderWindow->SetWindowName("Skeleton Visualisation - Volume visualization");
			volume = image.GetVolume();
			renderer->AddVolume(volume);
			break;

		case Segmented:
			renderWindow->SetWindowName("Skeleton Visualization - Segmented image");
			segmActor = image.GetSegmentedImage();
			segmActor->GetProperty()->SetOpacity(1.0);
			outlineActor = image.GetSegmentedOutline();
			setupSegmentedImagePlanes();
			renderer->AddActor(segmActor);
			renderer->AddActor(outlineActor);
			break;
	}
}

/*
 * Function to setup image planes for segmented image
 * Ref: https://github.com/Kitware/VTK/tree/master/Examples/GUI/Qt/FourPaneViewer
 */
void setupSegmentedImagePlanes()
{
	if (image.planes[0] != NULL)
		return;

	vtkSmartPointer<vtkCellPicker> picker = vtkSmartPointer<vtkCellPicker>::New();
	picker->SetTolerance(0.005);

	vtkSmartPointer<vtkProperty> ipwProp = vtkSmartPointer<vtkProperty>::New();
	vtkSmartPointer<vtkStructuredPointsReader> reader = image.GetSegmentedImageReader();

	reader->GetOutput()->GetDimensions(image.dimensions);

	for (int i = 0; i < 3; i++)
    {
		image.planes[i] = vtkSmartPointer<vtkImagePlaneWidget>::New();
		image.planes[i]->SetInteractor(renderWindowInteractor);
		image.planes[i]->SetPicker(picker);
		image.planes[i]->RestrictPlaneToVolumeOn();

		// outline: Red/Green/Blue
		double color[3] = {0, 0, 0};
		color[i] = 1;
		image.planes[i]->GetPlaneProperty()->SetColor(color);

		image.planes[i]->SetTexturePlaneProperty(ipwProp);
		image.planes[i]->TextureInterpolateOff();
		image.planes[i]->SetResliceInterpolateToLinear();
		image.planes[i]->SetInputConnection(reader->GetOutputPort());
		image.planes[i]->SetPlaneOrientation(i);
		image.planes[i]->SetSliceIndex(image.dimensions[i]/2);
		image.planes[i]->DisplayTextOn();
		image.planes[i]->SetDefaultRenderer(renderer);
		image.planes[i]->SetWindowLevel(1358, -27); // changes the color of plane
	}
}

void addToDistance(int point[3])
{
	if (!startSet || (startSet && stopSet))
	{
		if (startSet && stopSet)
			resetDistancePoints();

		distanceStart[0] = (double) point[0];
		distanceStart[1] = (double) point[1];
		distanceStart[2] = (double) point[2];

		startSet = true;
		setDistanceText("Choose end point");
	}
	else if (!stopSet)
	{
		distanceStop[0] = (double) point[0];
		distanceStop[1] = (double) point[1];
		distanceStop[2] = (double) point[2];

		stopSet = true;

		double distance = computeDistance(distanceStart, distanceStop);

		stringstream stream;
		stream << "Distance: " << distance;
		setDistanceText(stream.str());
	}
}

void setDistanceText(string text)
{
	menuDistance->SetInput(text.c_str());
	refreshWindow();
}

void resetDistancePoints() 
{
	setDistanceText(" ");
	startSet = false;
	stopSet = false;
}

double computeDistance(double a[3], double b[3])
{
	return sqrt((a[0] - b[0])*(a[0] - b[0]) + (a[1] - b[1])*(a[1] - b[1]) + (a[2] - b[2])*(a[2] - b[2]));
}

bool isSkeleton(VesselFile type)
{
	return (type == Skeleton || type == SkeletonTubed || type == SkeletonColored || type == SkeletonVaryingRadii);
}

bool isEmpty(double pointVal[3])
{
	return (pointVal[0] == 0 && pointVal[1] == 0 && pointVal[2] == 0);
}

/*
 * Commands info
 */
void toggleCommandsMenu()
{
	int infoNextPage = (infoCurrentPage + 1) % infoCommandsSize;

	menuCommands->SetInput(infoCommands[infoNextPage].c_str());

	infoCurrentPage = infoNextPage;
	renderWindowInteractor->Render();
}

/*
 * Shows/hides loading message, used for VTK files when the window freezes.
 */
void toggleLoading()
{
	if (loadingData)
		renderer->RemoveActor2D(menuLoading);
	else
		renderer->AddActor2D(menuLoading);

	loadingData = !loadingData;
	refreshWindow();
}

/*
 * Shows/hides transparent segmented mesh
 */
void toggleSegmentedTransparent()
{
	if (!isSkeleton(image.currentVessel))
		return;
	
	if (!segmentedTransparentVisible)
	{
		toggleLoading(); // Loading on

		segmActor = image.GetSegmentedImage();
		segmActor->GetProperty()->SetOpacity(0.3);
		renderer->AddActor(segmActor);

		toggleLoading(); // Loading off
	}
	else
	{
		renderer->RemoveActor(segmActor);
	}
	
	segmentedTransparentVisible = !segmentedTransparentVisible;
	refreshWindow();
}

/*
 * Shows/hides image planes (sagittal, transversal, coronal)
 */
void togglePlane(ImagePlane planeToShow)
{
	if (image.currentVessel != Segmented)
		return;

	for (int i = 0; i < 3; i++)
	{
		if (planeToShow == i)
		{
			if (image.currentPlane == planeToShow)
			{
				// the same key second time -> hide
				image.planes[i]->SetEnabled(0);
				image.currentPlane = None;

				// no planes = add Segmented image again
				renderer->AddActor(segmActor);
			}
			else
			{ // show chosen plane
				image.planes[i]->SetEnabled(1);
				image.currentPlane = planeToShow;

				// plane visible = remove Segmented image
				renderer->RemoveActor(segmActor);
			}
			refreshWindow();
		}
		else
		{ // hide the rest of the planes
			image.planes[i]->SetEnabled(0);
		}
	}
}

/*
 * Refreshes interactor after adding/removing actors
 */
void refreshWindow()
{
	renderWindowInteractor->Render();
}

/*
 * Function to handle pressed keys
 */
void KeypressCallbackFunction(vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(clientData), void* vtkNotUsed(callData))
{
	vtkRenderWindowInteractor *iren = static_cast<vtkRenderWindowInteractor*>(caller);
	string key = iren->GetKeySym();

	if (key == "i")
		toggleCommandsMenu();

	// Segmented image
	if (image.currentVessel == Segmented)
	{
		if (key == "s")
			togglePlane(Sagittal);
	
		if (key == "t")
			togglePlane(Transversal);
		
		if (key == "c")
			togglePlane(Coronal);

		if (key == "plus" || key == "minus")
		{
			if (image.currentPlane == None)
				return;

			int currentSlice = image.planes[image.currentPlane]->GetSliceIndex();

			// vtkImagePlaneWidget takes care of too big/small slice index
			if (key == "plus")
				image.planes[image.currentPlane]->SetSliceIndex(currentSlice + 1);
			else
				image.planes[image.currentPlane]->SetSliceIndex(currentSlice - 1);

			refreshWindow();
		}
	}

	// Volume
	if (image.currentVessel == Volume)
	{
		// Rotation
		if (key == "plus")
			renderer->GetActiveCamera()->Azimuth(10.0);
		if (key == "minus")
			renderer->GetActiveCamera()->Azimuth(-10.0);
	}

	// Skeleton
	if (isSkeleton(image.currentVessel))
	{
		if (key == "x")
			resetDistancePoints();
	}

	// Zoom reset
	if (key == "z")
		renderer->ResetCamera();

	// Arrows
	if (key == "Up")
		renderer->GetActiveCamera()->Elevation(-2.0);
	if (key == "Down")
		renderer->GetActiveCamera()->Elevation(2.0);
	if (key == "Left")
		renderer->GetActiveCamera()->Yaw(-2.0);
	if (key == "Right")
		renderer->GetActiveCamera()->Yaw(2.0);

	// Vessel files
	if (key == "1") loadVessels(Skeleton);
	if (key == "2") loadVessels(SkeletonTubed);
	if (key == "3") loadVessels(SkeletonColored);
	if (key == "4") loadVessels(SkeletonVaryingRadii);

	if (key == "5") loadVessels(Volume);
	if (key == "6") loadVessels(Segmented);
	if (key == "0") toggleSegmentedTransparent();

	refreshWindow();
}