#include <iostream>

#include <vtkConeSource.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

// file legacy load
#include <vtkGenericDataObjectReader.h>
#include <vtkStructuredGrid.h>
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <string>

// textActor
#include <vtkTextActor.h>
#include <vtkTextProperty.h>


using namespace std;

int main(int, char *[])
{
	// Paths
	string dataDirPath = "data/";
	string vesselsDataPath = dataDirPath + "vessels_data.vtk";
	string vesselsSegPath = dataDirPath + "vessels_seg.vtk";
	string vesselsSkelPath = dataDirPath + "vessels_skel.vtk";

	// Reading legacy data
	//vtkSmartPointer<vtkGenericDataObjectReader> reader = vtkSmartPointer<vtkGenericDataObjectReader>::New();
	//reader->SetFileName(vesselsDataPath.c_str());
	//reader->Update();
 
	// All of the standard data types can be checked and obtained like this:
	//if(reader->IsFilePolyData())
 //   {
	//	cout << "output is a polydata" << endl;
	//	vtkPolyData* output = reader->GetPolyDataOutput();
	//	cout << "output has " << output->GetNumberOfPoints() << " points." << endl;
 //   }
	//else
	//{
	//	cout << "not poly data";
	//}

	//Create a cone
	vtkSmartPointer<vtkConeSource> coneSource =	vtkSmartPointer<vtkConeSource>::New();
	coneSource->Update();

	//Create a mapper and actor
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(coneSource->GetOutputPort());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	//Create a renderer, render window, and interactor
	vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkRenderWindow> renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
	renderWindow->AddRenderer(renderer);
	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetRenderWindow(renderWindow);

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
