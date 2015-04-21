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
#include <vtkPolyDataMapper.h>

#include <vtkTubeFilter.h>
#include <vtkPointSet.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkIdList.h>
#include <vtkCell.h>
#include <vtkCellArray.h>

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
vtkSmartPointer<vtkContourFilter> contourFilter = vtkSmartPointer<vtkContourFilter>::New();
vtkSmartPointer<vtkPolyDataMapper> pdMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
vtkSmartPointer<vtkTubeFilter> tubeFilter = vtkSmartPointer<vtkTubeFilter>::New();

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
	// Create reader
	reader->SetFileName(vesselsSkelFile.c_str());
	reader->Update();

	// Create contour filter
	contourFilter->SetInputConnection(reader->GetOutputPort()); 
	
	// setValue(): 1st parameter I don't understand.
	// 2nd parameter: The smaller it is, the more complete becomes the skeleton. Like it was a parameter describing
	// how small the smallest cube are allowed to be to be rendered.
	contourFilter->SetValue(0, 128.0f);
	contourFilter->Update();

	// TODO: Doesn't work. The tube filter needs other type of data than the vesselsSkelFile. I tried 
	// with contourFilter variable as input but then nothing was rendered..

	// FROM API: input line must not have duplicate points, or normals at points that are parallel to the incoming/outgoing 
	//line segments. (Duplicate points can be removed with vtkCleanPolyData.) 
	//If a line does not meet this criteria, then that line is not tubed.
	tubeFilter->SetInputConnection(contourFilter->GetOutputPort());
	tubeFilter->SetRadius(.5);
	tubeFilter->SetNumberOfSides(8);
	tubeFilter->Update();

	pdMapper->SetInputConnection(contourFilter->GetOutputPort()); 
	pdMapper->ScalarVisibilityOff();

	

	
	
	vtkSmartPointer<vtkPolyData> polyData = pdMapper->GetInput();
	vtkSmartPointer<vtkCellArray> cellArray = polyData->GetPolys(); 

	cout << "Indata: #Points: " << polyData->GetNumberOfPoints() << ", #Cells: " << polyData->GetNumberOfCells() << endl; // TODO just a test. >16000 points
	//cout << "Coordinate x of point 2: " << polyData->GetPoints()->GetPoint(2)[0] << endl;



    
    polyData->BuildLinks();

    vtkSmartPointer<vtkIdList> idlist = vtkSmartPointer<vtkIdList>::New();
    vtkSmartPointer<vtkIdList> cellIds = vtkSmartPointer<vtkIdList>::New();
    
    cellArray->InitTraversal();
    /*for(int j = 0; j < 20; j++) 
    {
    	cellArray->GetNextCell(idlist);
    	cout << "idlist size " << idlist->GetNumberOfIds() << endl;

    	for(int k = 0; k < idlist->GetNumberOfIds(); k++)
    	{

    		int pointID = idlist->GetId(k);
    		polyData->GetPointCells(pointID, cellIds);
    		
    		cout << "Cells that share point " << pointID << ": ";
    		for(int l = 0; l < cellIds->GetNumberOfIds(); l++) { cout << cellIds->GetId(l) << ", "; }
    		cout << endl;

    	}
    }
    
    cellArray->GetCell(0, idlist);
    cout << idlist->GetNumberOfIds() << endl;
    polyData->GetCellNeighbors(0, idlist, cellIds);
    cout << cellIds->GetNumberOfIds() << " neighbors of cell 0: ";
    for(int l = 0; l < cellIds->GetNumberOfIds(); l++) { cout << cellIds->GetId(l) << ", "; }
    cout << endl;*/
   

	/*cellArray->InitTraversal();
    for (int j = 0; j < 9; j++)
    {
    	cellArray->GetNextCell(idlist);
    	cout << "idlist size " << idlist->GetNumberOfIds() << endl;

    	cout << "-----Cell " << j << "-----" << endl;
    	for(int k = 0; k < idlist->GetNumberOfIds(); k++)
    	{
    		double p[3];
    		polyData->GetPoint(idlist->GetId(k), p);
    		cout << "		" << "point " << k << ", (x, y, z): (" << p[0] << ", " << p[1] << ", " << p[2] << ")" << endl;
    	}
    	
    }*/


    
                  
              


	// Create renderer
	renderer->SetBackground(1.0, 1.0, 1.0);
	
	// Create actors
	actor->SetMapper(pdMapper);
	renderer->AddActor(actor);

	textActor->GetTextProperty()->SetFontSize(16);
	textActor->SetDisplayPosition(10, 10);
	textActor->SetInput("Skeleton path");
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
}

/*
 * Use of marching cube algorithm to render the segemented image as instructed (...Or an effort to do it).
 * Note to self: use segment data
 * TODO: Figure out if I should have the skeleton image and segment image in same window or not?
 *       What should I reuse in the functions? Readers? mappers?
 */
void renderSegmentedImage() 
{
	// Create reader
	reader->SetFileName(vesselsSegFile.c_str());
	reader->Update();

	// Create contour filter
	contourFilter->SetInputConnection(reader->GetOutputPort()); 
	contourFilter->SetValue(0, 128.0f);
	contourFilter->Update();
	pdMapper->SetInputConnection(contourFilter->GetOutputPort()); 
	pdMapper->ScalarVisibilityOff();

	// Create renderer
	renderer->SetBackground(1.0, 1.0, 1.0);
	
	// Create actors
	actor->SetMapper(pdMapper);
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
	//renderDataImage();
	//renderSegmentedImage();
	renderSkeletonImage();
  
	return EXIT_SUCCESS;
}

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