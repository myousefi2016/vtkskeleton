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

using namespace std;

// Functions
void KeypressCallbackFunction (vtkObject* caller, long unsigned int eventId, void* clientData, void* callData);

int main(int, char *[])
{
	// Paths
	string dataDirPath = "data/";
	string vesselsDataPath = dataDirPath + "vessels_data.vtk";
	string vesselsSegPath = dataDirPath + "vessels_seg.vtk";
	string vesselsSkelPath = dataDirPath + "vessels_skel.vtk";

	// a) Load VTK files and make the basic menu
	// Read legacy data from .vtk files
	vtkSmartPointer<vtkStructuredPointsReader> reader = vtkSmartPointer<vtkStructuredPointsReader>::New();
	reader->SetFileName(vesselsDataPath.c_str());
	reader->Update();
 
	vtkSmartPointer<vtkImageDataGeometryFilter> geometryFilter = vtkSmartPointer<vtkImageDataGeometryFilter>::New();
	geometryFilter->SetInputConnection(reader->GetOutputPort());
	geometryFilter->Update();
 
	// Visualize
	// Create a mapper and actor
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(geometryFilter->GetOutputPort());
 
	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	//Create a renderer, render window, and interactor
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

	// b) Planes for different views
	// Handle key press
	vtkSmartPointer<vtkCallbackCommand> keypressCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	keypressCallback->SetCallback(KeypressCallbackFunction);
	renderWindowInteractor->AddObserver(vtkCommand::KeyPressEvent, keypressCallback);

	//Add the actors to the scene
	renderer->AddActor(actor);
	renderer->SetBackground(1.0, 1.0, 1.0);

	// Menu vtkTextActor
	vtkSmartPointer<vtkTextActor> textActor = vtkSmartPointer<vtkTextActor>::New();
	textActor->GetTextProperty()->SetFontSize(16);
	textActor->SetDisplayPosition(10, 10);
	renderer->AddActor2D(textActor);
	textActor->SetInput("Menu");
	textActor->GetTextProperty()->SetColor(0.0, 0.0, 0.0);

	//Render and interact
	renderWindow->Render();
	renderWindowInteractor->Start();
  
	return EXIT_SUCCESS;
}

void KeypressCallbackFunction ( vtkObject* caller, long unsigned int vtkNotUsed(eventId), void* vtkNotUsed(clientData), void* vtkNotUsed(callData) )
{
	vtkRenderWindowInteractor *iren = static_cast<vtkRenderWindowInteractor*>(caller);
 
	std::cout << "Pressed: " << iren->GetKeySym() << std::endl;
}