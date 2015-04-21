#include <iostream>
#include <sstream>

#include <vtkPolyData.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>

// file legacy load
#include <vtkStructuredPointsReader.h>
#include <vtkImageDataGeometryFilter.h>

// textActor
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

// key pressed
#include <vtkCallbackCommand.h>
#include <vtkCommand.h>

#include <vtkCamera.h>

#include "MyImage3D.h"

using namespace std;

// Functions
void initVTK();
void renderVTK();
void prepareMenu();
void toggleMenu();
void KeypressCallbackFunction (vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);

// VTK window
vtkSmartPointer<vtkRenderer> renderer;
vtkSmartPointer<vtkRenderWindow> renderWindow;
vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;

// Menu
vtkSmartPointer<vtkTextActor> menuInfo, menuCommands;

// Configuration
const int windowSizeX = 1000;
const int windowSizeY = 700;
const int menuPositionX = 10;
const int menuPositionY = 10; // padding from bottom

// Flags
bool menuInfoVisible = false;

int main(int, char *[])
{
	initVTK();

	// Example of creating a new 3-D image and initializing it with 0:
	MyImage3D image;
	image.Set(230, 256, 256);
	image.FillInWith(0);

	//Put some voxel values in the image:
	image.Index(1,2,3) = 15;
	image.Index(4,2,2) = 5;
	image.Index(8,6,3) = 7;
	image.Index(5,6,8) = 20000;
	image.Index(3,3,3) = 1;
	image.Index(1,1,1) = 15;

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

	vtkSmartPointer<vtkActor> dataActor;
	dataActor = image.LoadDataImage();
	//dataActor = image.LoadSegmentedImage();
	//dataActor = image.LoadSkeletonImage();
	renderer->AddActor(dataActor);

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
	renderWindow->SetWindowName("Skeleton Visualisation");
	
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
 * Prepare menus and show info message
 */
void prepareMenu() {
	menuInfo = vtkSmartPointer<vtkTextActor>::New();
	menuCommands = vtkSmartPointer<vtkTextActor>::New();
	
	menuInfo->GetTextProperty()->SetFontFamilyToCourier();
	menuInfo->GetTextProperty()->SetFontSize(14);
	menuInfo->GetTextProperty()->SetColor(0.0, 0.0, 0.0);
	menuInfo->SetDisplayPosition(menuPositionX, menuPositionY);
	menuInfo->SetInput("[i] see available commands");
	
	menuCommands->GetTextProperty()->SetFontFamilyToCourier();
	menuCommands->GetTextProperty()->SetFontSize(14);
	menuCommands->GetTextProperty()->SetColor(0.0, 0.0, 0.0);
	menuCommands->SetDisplayPosition(menuPositionX, menuPositionY);
	menuCommands->SetInput("[s] sagittal view\n[t] transversal view\n[c] coronal view\n[+/-] zoom in/out\n[r] reset zoom\n[e/q] exit\n[i] close commands info");
}

void toggleMenu() {
	if (menuInfoVisible) { // we will see list of commands
		renderer->RemoveActor2D(menuInfo);
		renderer->AddActor2D(menuCommands);
		menuInfoVisible = false;
	} else {
		renderer->RemoveActor2D(menuCommands);
		renderer->AddActor2D(menuInfo);
		menuInfoVisible = true;
	}
	renderWindowInteractor->Render();
}

void KeypressCallbackFunction(vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(clientData), void* vtkNotUsed(callData))
{
	vtkRenderWindowInteractor *iren = static_cast<vtkRenderWindowInteractor*>(caller);
 
	string key = iren->GetKeySym();

	if (key == "i") {
		toggleMenu();
	}
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
		renderer->GetActiveCamera()->Zoom(1.25);
		renderWindowInteractor->Render();
	}
	if (key == "minus") {
		renderer->GetActiveCamera()->Zoom(0.8);
		renderWindowInteractor->Render();
	}
	if (key == "r") {
		renderer->ResetCamera();
	}
}