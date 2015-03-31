#include <iostream>
#include <string>

#include <vtkConeSource.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

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

using namespace std;

// Functions
void KeypressCallbackFunction (vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);

// Variables, to make code simple to read
vtkSmartPointer<vtkStructuredPointsReader> reader = vtkSmartPointer<vtkStructuredPointsReader>::New();
vtkSmartPointer<vtkImageDataGeometryFilter> geometryFilter = vtkSmartPointer<vtkImageDataGeometryFilter>::New();
vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
// Key press
vtkSmartPointer<vtkCallbackCommand> keypressCallback = vtkSmartPointer<vtkCallbackCommand>::New();
// Menu
vtkSmartPointer<vtkTextActor> textActor = vtkSmartPointer<vtkTextActor>::New();

// Paths
string vesselsDataFile = "vessels_data.vtk";
string vesselsSegFile = "vessels_seg.vtk";
string vesselsSkelFile = "vessels_skel.vtk";

/*
 * Use of marching cube algorithm to render skeleton image. Note to self: use skeleton data.
 * TODO: Figure out if I should have the skeleton image and segment image in same window or not?
 */
void renderSkeletonImage()
{
	// TODO I believe this function should be a lot like 'renderSegmentedImage()'.
}

/*
 * Use of marching cube algorithm to render the segemented image as instructed (...Or an effort to do it).
 * Note to self: use segment data
 * TODO: Figure out if I should have the skeleton image and segment image in same window or not?
 *       What should I reuse in the functions? Readers? mappers?
 */
void renderSegmentedImage() // TODO 2015-03-31: YIEALDS SEGMENTATION FAULT 11. FOR NOW UNSOLVED...
{
	// Create reader
	vtkSmartPointer<vtkStructuredPointsReader> spReader = vtkSmartPointer<vtkStructuredPointsReader>::New();
	spReader->SetFileName(vesselsSegFile.c_str());
	spReader->Update();

	// Create contour filter
	vtkSmartPointer<vtkContourFilter> contourFilter = vtkSmartPointer<vtkContourFilter>::New();
	contourFilter->SetInputConnection(spReader->GetOutputPort());
	contourFilter->Update();

	// TODO Can we use the same mapper for several filters?
	vtkSmartPointer<vtkPolyDataMapper> pdMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	pdMapper->SetInputConnection(contourFilter->GetOutputPort());

	vtkSmartPointer<vtkRenderer> segmentRenderer = vtkSmartPointer<vtkRenderer>::New();
	segmentRenderer->SetBackground(1.0, 1.0, 1.0);
	
	// Create actors
	vtkSmartPointer<vtkActor> renderSegmentActor = vtkSmartPointer<vtkActor>::New();
	renderSegmentActor->SetMapper(pdMapper);
	segmentRenderer->AddActor(renderSegmentActor);

	vtkSmartPointer<vtkTextActor> segmentTextActor = vtkSmartPointer<vtkTextActor>::New();
	segmentTextActor->GetTextProperty()->SetFontSize(16);
	segmentTextActor->SetDisplayPosition(10, 10);
	segmentTextActor->SetInput("Segmented Image");
	segmentTextActor->GetTextProperty()->SetColor(0.0, 0.0, 0.0);
	segmentRenderer->AddActor2D(segmentTextActor);

	vtkSmartPointer<vtkRenderWindow> segmentRenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	segmentRenderWindow->AddRenderer(segmentRenderer);
	//Start
	segmentRenderWindow->Render();

}

/*
 * Uses vessels data file.
 */
void renderDataImage()
{
	// a) Load VTK files and make the basic menu
	// Read legacy data from .vtk files
	reader->SetFileName(vesselsDataFile.c_str());
	reader->Update();
 
	geometryFilter->SetInputConnection(reader->GetOutputPort());
	geometryFilter->Update();
 
	// Visualize
	// Create a mapper and actor
	mapper->SetInputConnection(geometryFilter->GetOutputPort());
	actor->SetMapper(mapper);

	//Create a renderer, render window, and interactor
	renderWindow->AddRenderer(renderer);
	renderWindowInteractor->SetRenderWindow(renderWindow);

	// b) Planes for different views
	// Handle key press
	keypressCallback->SetCallback(KeypressCallbackFunction);
	renderWindowInteractor->AddObserver(vtkCommand::KeyPressEvent, keypressCallback);

	//Add the actors to the scene
	renderer->AddActor(actor);
	renderer->SetBackground(1.0, 1.0, 1.0);

	// Menu vtkTextActor
	textActor->GetTextProperty()->SetFontSize(16);
	textActor->SetDisplayPosition(10, 10);
	renderer->AddActor2D(textActor);
	textActor->SetInput("Menu");
	textActor->GetTextProperty()->SetColor(0.0, 0.0, 0.0);

	//Render and interact
	renderWindow->Render();
	renderWindowInteractor->Start();
}

int main(int, char *[])
{
	renderDataImage();
	//renderSegmentedImage();
	//renderSkeletonImage();
  
	return EXIT_SUCCESS;
}

void KeypressCallbackFunction(vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(clientData), void* vtkNotUsed(callData))
{
	vtkRenderWindowInteractor *iren = static_cast<vtkRenderWindowInteractor*>(caller);
 
	string key = iren->GetKeySym();

	cout << "Pressed" << key << endl;

	if (key == "s") {
		cout << "sagittal, do something" << endl;
	}
	if (key == "t") {
		cout << "transversal, do something" << endl;
	}
	if (key == "c") {
		cout << "coronal, do something" << endl;
	}
		cout << "scroll: zoom in" << endl;
	}
		cout << "scroll: zoom out" << endl;
	}
}