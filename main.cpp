#include <iostream>
#include <sstream>

#include <vtkConeSource.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
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

// Marching cube algorithm
#include <vtkContourFilter.h>
#include <vtkPolyDataMapper.h>

#include "MyImage3D.h"

using namespace std;

// Functions
void initVTK();
void renderVTK();
void renderDataImage();
void renderSegmentedImage();
void renderSkeletonImage();
void KeypressCallbackFunction (vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);

// VTK window
vtkSmartPointer<vtkRenderer> renderer;
vtkSmartPointer<vtkRenderWindow> renderWindow;
vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor;

vtkSmartPointer<vtkContourFilter> iso = vtkSmartPointer<vtkContourFilter>::New();
vtkSmartPointer<vtkPolyDataMapper> isoMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

// Menu
vtkSmartPointer<vtkTextActor> textActor = vtkSmartPointer<vtkTextActor>::New();


int main(int, char *[])
{
	initVTK();

	// Example of creating a new 3-D image and initializing it with 0:
	MyImage3D image;
	image.Set(10,10,10);
	image.FillInWith(0);
	cout << "3-D image created and filled with 0." << endl;

	//Put some voxel values in the image:
	image.Index(1,2,3) = 15;
	image.Index(4,2,2) = 5;
	image.Index(8,6,3) = 7;
	image.Index(5,6,8) = 20000;
	image.Index(3,3,3) = 1;
	image.Index(1,1,1) = 15;

	// Example of going through the whole 3-D image and finding the maximum value:
	unsigned short maximum;
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
	}
	cout << "Maximum value in the 3D image is " << ((int)(maximum)) << "." << endl;

	//----- Print maximum value on the screen -----
	vtkSmartPointer<vtkTextActor> maximumText = vtkSmartPointer<vtkTextActor>::New();
	
	maximumText->GetTextProperty()->SetFontSize(16);
	maximumText->GetTextProperty()->SetColor(1.0, 1.0, 0.0);
	maximumText->SetDisplayPosition(15, 5);
	ostringstream text;
	text << "maximum = " << ((int)(maximum)) << endl;
	text << ends;

	maximumText->SetInput(text.str().c_str());
	renderer->AddActor2D(maximumText);

	vtkSmartPointer<vtkActor> dataImageActor = image.LoadDataImage();
	renderer->AddActor(dataImageActor);

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
	renderWindow->SetSize(700, 700);
	renderWindow->SetWindowName("Skeleton Visualisation");
	
	// Handle key press
	vtkSmartPointer<vtkCallbackCommand> keypressCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	keypressCallback->SetCallback(KeypressCallbackFunction);
	renderWindowInteractor->AddObserver(vtkCommand::KeyPressEvent, keypressCallback);

	renderWindowInteractor->SetRenderWindow(renderWindow);
	renderWindowInteractor->SetInteractorStyle(interactorStyle);
}

void renderVTK()
{
	renderWindow->Render();
	renderWindowInteractor->Start();
}


/*
 * Use of marching cube algorithm to render the segemented image as instructed (...Or an effort to do it).
 * Note to self: use segment data
 * TODO: Figure out if I should have the skeleton image and segment image in same window or not?
 *       What should I reuse in the functions? Readers? mappers?
 
void renderSegmentedImage() 
{
	// Create reader
	reader->SetFileName(vesselsSegFile.c_str());
	reader->Update();

	// Create contour filter
	iso->SetInputConnection(reader->GetOutputPort()); 
	iso->SetValue(0, 128.0f);
	iso->Update();
	isoMapper->SetInputConnection(iso->GetOutputPort()); 
	isoMapper->ScalarVisibilityOff();

	// Create renderer
	renderer->SetBackground(1.0, 1.0, 1.0);
	
	// Create actors
	actor->SetMapper(isoMapper);
	renderer->AddActor(actor);

	textActor->GetTextProperty()->SetFontSize(16);
	textActor->SetDisplayPosition(10, 10);
	textActor->SetInput("Segmented Image");
	textActor->GetTextProperty()->SetColor(0.0, 0.0, 0.0);
	renderer->AddActor2D(textActor);

	// Create render window
	renderWindow->AddRenderer(renderer);
	renderWindowInteractor->SetRenderWindow(renderWindow);

	keypressCallback->SetCallback(KeypressCallbackFunction);
	renderWindowInteractor->AddObserver(vtkCommand::KeyPressEvent, keypressCallback);

	//Start
	renderWindow->Render();
	renderWindowInteractor->Start();
}*/



void KeypressCallbackFunction(vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(clientData), void* vtkNotUsed(callData))
{
	vtkRenderWindowInteractor *iren = static_cast<vtkRenderWindowInteractor*>(caller);
 
	string key = iren->GetKeySym();

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
		cout << "scroll: zoom in" << endl;
	}
	if (key == "minus") {
		cout << "scroll: zoom out" << endl;
	}
}