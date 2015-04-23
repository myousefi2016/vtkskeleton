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
#include <vtkImagePlaneWidget.h>

#include "MyImage3D.h"

using namespace std;

// Functions
void initVTK();
void renderVTK();
void prepareMenu();
void toggleMenu();
void toggleLoading();
void loadVessels();
void loadFile(VesselFile type);
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

	//planeWidget = vtkSmartPointer<vtkImagePlaneWidget>::New();
	//planeWidget->SetInteractor(renderWindowInteractor);
 
	//double origin[3] = {0, 1, 0};
	//planeWidget->SetOrigin(origin);
	//planeWidget->UpdatePlacement();
}

void renderVTK()
{
	renderWindow->Render();
	//planeWidget->On();
	renderWindowInteractor->Start();
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
	menuCommands->SetInput("# Segmented image:\n[s] sagittal view\n[t] transversal view\n[c] coronal view\n\n[p/m] zoom in/out\n[z] reset zoom\n[e/q] exit\n[i] close commands info");
	
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
 * Used to show/hide available commands
 */
void toggleMenu()
{
	if (menuInfoVisible) { // we will see list of commands
		renderer->RemoveActor2D(menuInfo);
		renderer->AddActor2D(menuCommands);
	} else {
		renderer->RemoveActor2D(menuCommands);
		renderer->AddActor2D(menuInfo);
	}
	menuInfoVisible = !menuInfoVisible;
	renderWindowInteractor->Render();
}

/*
 * Shows loading message, used for VTK files when the window freezes.
 */
void toggleLoading()
{
	if (loadingData) {
		renderer->RemoveActor2D(menuLoading);
	} else {
		renderer->AddActor2D(menuLoading);
	}
	loadingData = !loadingData;
	refreshWindow();
}

/*
 * Handles loading different vessels on user's key press
 */
void loadVessels(VesselFile type)
{
	// drop multiple loads
	if (image.currentVessel == Loading || image.currentVessel == type)
		return;
	
	// Loading on
	image.currentVessel = Loading;
	renderer->AddActor2D(menuLoading);
	refreshWindow();

	// Load file
	loadFile(type);
	
	// Loading off
	image.currentVessel = type;
	renderer->RemoveActor2D(menuLoading);
	refreshWindow();
}

/*
 * Loads demanded file and changes window title
 */
void loadFile(VesselFile type)
{
	renderer->RemoveActor(actor);
	renderer->RemoveActor(outlineActor);
	renderer->RemoveVolume(volume);

	switch (type) {
	case Data:
		actor = image.GetDataImage();
		renderWindow->SetWindowName("Skeleton Visualisation (vessels_data.vtk)");
		break;
	case Segmented:
		actor = image.GetSegmentedImage();
		outlineActor = image.GetSegmentedOutline();
		renderer->AddActor(outlineActor);
		renderWindow->SetWindowName("Skeleton Visualisation (vessels_seg.vtk)");
		break;
	case Skeleton:
		actor = image.GetSkeletonImage();
		renderWindow->SetWindowName("Skeleton Visualisation (vessels_skel.vtk)");
		break;
	case RayCast:
		volume = image.GetRayCastingImage();
		renderWindow->SetWindowName("Ray Casting (vessels_data.vtk)");
		renderer->AddVolume(volume);
		return;
	}
	renderer->AddActor(actor);
}

/*
 * Refreshes interactor after adding/removing actors
 */
void refreshWindow()
{
	renderWindowInteractor->Render();
	//renderer->Modified();
	//renderWindow->Render();
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
		if (key == "s") {
			cout << "sagittal, do something" << endl;
		}
		if (key == "t") {
			cout << "transversal, do something" << endl;
		}
		if (key == "c") {
			cout << "coronal, do something" << endl;
		}

		if (key == "plus") {

		}
		if (key == "minus") {

		}
	}

	if (image.currentVessel == RayCast)
	{
		if (key == "plus") {

		}
		if (key == "minus") {

		}
	}

	// Zoom in/out
	if (key == "p") {
		renderer->GetActiveCamera()->Zoom(1.25);
		renderWindowInteractor->Render();
	}
	if (key == "m") {
		renderer->GetActiveCamera()->Zoom(0.8);
		renderWindowInteractor->Render();
	}
	if (key == "z") {
		renderer->ResetCamera();
	}

	// Vessel files
	if (key == "6") loadVessels(RayCast); //loadVessels(Data);
	if (key == "7") loadVessels(Segmented);
	if (key == "8") loadVessels(Skeleton);
}