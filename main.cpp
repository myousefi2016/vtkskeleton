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

// 
#include <vtkCellPicker.h>
#include <vtkStructuredPointsReader.h>
#include <vtkStructuredPoints.h>

#include "MyImage3D.h"

using namespace std;

// Functions
void initVTK();
void renderVTK();
void prepareMenu();
void loadVessels();
void loadFile(VesselFile type);
void setupSegmentedImagePlanes();

// UI functions
void toggleMenu();
void toggleLoading();
void togglePlane(ImagePlane planeToShow);
void refreshWindow();

void KeypressCallbackFunction (vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);

// VTK window
vtkSmartPointer<vtkRenderer> renderer;
vtkSmartPointer<vtkRenderWindow> renderWindow;
vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;
vtkSmartPointer<vtkImagePlaneWidget> planeWidget;

vtkSmartPointer<vtkVolume> volume;
vtkSmartPointer<vtkActor> actor, outlineActor;

// Menu
vtkSmartPointer<vtkTextActor> menuInfo, menuCommands, menuVessels, menuLoading;

// Configuration
const int windowSizeX = 1000;
const int windowSizeY = 700;

// Flags
bool menuInfoVisible = false;
bool loadingData = false;

MyImage3D image;

int main(int, char *[])
{
	initVTK();

	// Example of creating a new 3-D image and initializing it with 0:
	//image.Set(230, 256, 256);
	//image.FillInWith(0);

	//Put some voxel values in the image:
	//image.Index(1,2,3) = 15;
	//image.Index(4,2,2) = 5;
	//image.Index(8,6,3) = 7;
	//image.Index(5,6,8) = 20000;
	//image.Index(3,3,3) = 1;
	//image.Index(1,1,1) = 15;

	// Example of going through the whole 3-D image and finding the maximum value:
	/*unsigned short maximum;
	maximum = image.Index(0,0,0);
	int dims_xyz[3];
	image.vtk_image_data->GetDimensions(dims_xyz);
	for (int s = 0; s < dims_xyz[2]; s++)
	{
		for (int r = 0; r < dims_xyz[1]; r++)
		{
			for (int c = 0; c<dims_xyz[0]; c++)
			{
				if (image.Index(s,r,c) > maximum)
					maximum = image.Index(s,r,c);
			}
		}
	}*/
	
	// The first one to load
	loadFile(Segmented);
	//loadFile(RayCast);

	renderVTK();

	return EXIT_SUCCESS;
}

void initVTK()
{
	renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderer = vtkSmartPointer<vtkRenderer>::New();

	vtkSmartPointer<vtkInteractorStyleTrackballCamera> interactorStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
	
	renderer->SetBackground(1.0, 1.0, 1.0);

	renderWindow->AddRenderer(renderer);
	renderWindow->SetSize(windowSizeX, windowSizeY);
	
	// Handle key press
	vtkSmartPointer<vtkCallbackCommand> keypressCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	keypressCallback->SetCallback(KeypressCallbackFunction);
	renderWindowInteractor->AddObserver(vtkCommand::KeyPressEvent, keypressCallback);

	// Prepare menu
	prepareMenu();
	toggleMenu();

	renderWindowInteractor->SetRenderWindow(renderWindow);
	renderWindowInteractor->SetInteractorStyle(interactorStyle);
}

void renderVTK()
{
	renderWindow->Render();
	renderWindowInteractor->Start();
}


/*
 * Set the update rate (level of detail of the image) in 'lod' frames/second
 */
void setLOD(int lod)
{
	//if(lodActor == NULL || renderWindow == NULL) return 0;
	renderWindow->SetDesiredUpdateRate(lod);
}


/*
 * Prepare menu and info messages
 */
void prepareMenu()
{
	menuInfo = vtkSmartPointer<vtkTextActor>::New();
	menuCommands = vtkSmartPointer<vtkTextActor>::New();
	menuVessels = vtkSmartPointer<vtkTextActor>::New();
	menuLoading = vtkSmartPointer<vtkTextActor>::New();

	int menuPositionX = 10;
	int menuPositionY = 10; // padding from bottom
	
	menuInfo->GetTextProperty()->SetFontFamilyToCourier();
	menuInfo->GetTextProperty()->SetFontSize(14);
	menuInfo->GetTextProperty()->SetColor(0.0, 0.0, 0.0);
	menuInfo->SetDisplayPosition(menuPositionX, menuPositionY);
	menuInfo->SetInput("[i] see available commands");
	
	menuCommands->GetTextProperty()->SetFontFamilyToCourier();
	menuCommands->GetTextProperty()->SetFontSize(14);
	menuCommands->GetTextProperty()->SetColor(0.0, 0.0, 0.0);
	menuCommands->SetDisplayPosition(menuPositionX, menuPositionY);
	menuCommands->SetInput("# Segmented image:\n[s] sagittal view\n[t] transversal view\n[c] coronal view\n[+/-] scroll through the slices\n\n# Volume rendering:\n[+/-] rotate view\n\n[p/m] zoom in/out\n[z] reset zoom\n[e/q] exit\n[i] close commands info");
	
	menuVessels->GetTextProperty()->SetFontFamilyToCourier();
	menuVessels->GetTextProperty()->SetFontSize(14);
	menuVessels->GetTextProperty()->SetColor(0.0, 0.0, 1.0);
	menuVessels->SetDisplayPosition(menuPositionX + 300, menuPositionY);
	menuVessels->SetInput("[6] volume rendering [7] segmented image [8] skeleton image");
	renderer->AddActor(menuVessels);
	
	menuLoading->GetTextProperty()->SetFontFamilyToCourier();
	menuLoading->GetTextProperty()->SetFontSize(14);
	menuLoading->GetTextProperty()->SetColor(1.0, 0.0, 0.0);
	menuLoading->SetDisplayPosition(menuPositionX + 300, menuPositionY + 20);
	menuLoading->SetInput("Loading, it may take a while...");
}

/*
 * Handles loading different vessels on user's key press
 */
void loadVessels(VesselFile type)
{
	// drop multiple loads
	if (loadingData || image.currentVessel == type)
		return;

	toggleLoading(); // Loading on

	// hide planes if any visible before changing to ray casting/skeleton view
	togglePlane(image.currentPlane);

	// remove actors and/or volumes
	renderer->RemoveActor(actor);
	renderer->RemoveActor(outlineActor);
	renderer->RemoveVolume(volume);

	loadFile(type); // Load file

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
		case RayCast:
			volume = image.GetRayCastingImage();
			renderWindow->SetWindowName("Skeleton Visualisation - Volume visualization (vessels_data.vtk)");
			renderer->AddVolume(volume);
			return; // not break, we don't want to add vtkActor, only vtkVolume
		case Segmented:
			actor = image.GetSegmentedImage();
			outlineActor = image.GetSegmentedOutline();
			setupSegmentedImagePlanes();
			renderer->AddActor(outlineActor);
			renderWindow->SetWindowName("Skeleton Visualisation - Segmented image (vessels_seg.vtk)");
			break;
		case Skeleton:
			actor = image.GetSkeletonImage();
			renderWindow->SetWindowName("Skeleton Visualisation - Skeleton image (vessels_skel.vtk)");
			break;
	}

	renderer->AddActor(actor);
}

/*
 * Function to setup image planes for segmented image
 * Ref: https://github.com/Kitware/VTK/tree/master/Examples/GUI/Qt/FourPaneViewer
 */
void setupSegmentedImagePlanes()
{
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

/*
 * Shows/hides available commands info
 */
void toggleMenu()
{
	if (menuInfoVisible)
	{ // we will see list of commands
		renderer->RemoveActor2D(menuInfo);
		renderer->AddActor2D(menuCommands);
	}
	else
	{
		renderer->RemoveActor2D(menuCommands);
		renderer->AddActor2D(menuInfo);
	}

	menuInfoVisible = !menuInfoVisible;
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
				renderer->AddActor(actor);
			}
			else
			{ // show chosen plane
				image.planes[i]->SetEnabled(1);
				image.currentPlane = planeToShow;

				// plane visible = remove Segmented image
				renderer->RemoveActor(actor);
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

	if (key == "i") {
		toggleMenu();
	}

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

			// vtkImagePlaneWidget takes care of too big/small dimensions
			if (key == "plus")
				image.planes[image.currentPlane]->SetSliceIndex(currentSlice + 1);
			else
				image.planes[image.currentPlane]->SetSliceIndex(currentSlice - 1);

			refreshWindow();
		}
	}

	if (image.currentVessel == RayCast)
	{
		if (key == "plus") {
			renderer->GetActiveCamera()->Azimuth(5.0);
			refreshWindow();
		}
		if (key == "minus") {
			renderer->GetActiveCamera()->Azimuth(-5.0);
			refreshWindow();
		}
	}

	// Zoom in/out
	if (key == "p") {
		renderer->GetActiveCamera()->Zoom(1.25);
		refreshWindow();
	}
	if (key == "m") {
		renderer->GetActiveCamera()->Zoom(0.8);
		refreshWindow();
	}
	if (key == "z") {
		renderer->ResetCamera();
	}

	// Vessel files
	if (key == "6") loadVessels(RayCast);
	if (key == "7") loadVessels(Segmented);
	if (key == "8") loadVessels(Skeleton);
}