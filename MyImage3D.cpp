/**************************************************************************
 3D Image class for unsigned short (16-bit unsigned) values

 Author: Eric Klaesson, Damian Tarnacki
 File name: "MyImage3D.cpp"
***************************************************************************/

#include "MyImage3D.h"

// Ref: https://github.com/Kitware/VTK/blob/master/Examples/Medical/Cxx/Medical4.cxx
vtkSmartPointer<vtkVolume> MyImage3D::GetVolume()
{
	if (raycastVolume != NULL)
		return raycastVolume;

	dataReader = vtkSmartPointer<vtkStructuredPointsReader>::New();
	dataReader->SetFileName("vessels_data.vtk");
	dataReader->Update();
 
	vtkSmartPointer<vtkVolumeRayCastMapper> rayCastMapper = vtkSmartPointer<vtkVolumeRayCastMapper>::New();
	rayCastMapper->SetInputConnection(dataReader->GetOutputPort());

	vtkSmartPointer<vtkVolumeRayCastMIPFunction> rcMipFun = vtkSmartPointer<vtkVolumeRayCastMIPFunction>::New();

	rayCastMapper->SetVolumeRayCastFunction(rcMipFun);

	// The gradient opacity function is used to decrease the opacity
	// in the "flat" regions of the volume while maintaining the opacity
	// at the boundaries between tissue types.  The gradient is measured
	// as the amount by which the intensity changes over unit distance.
	// For most medical data, the unit distance is 1mm.
	vtkSmartPointer<vtkPiecewiseFunction> volumeGradientOpacity = vtkSmartPointer<vtkPiecewiseFunction>::New();
	volumeGradientOpacity->AddPoint(0,   0.0);
	volumeGradientOpacity->AddPoint(90,  0.5);
	volumeGradientOpacity->AddPoint(100, 1.0);

	vtkSmartPointer<vtkColorTransferFunction> volumeColor = vtkSmartPointer<vtkColorTransferFunction>::New();
	volumeColor->AddRGBPoint(10, 1.0, 1.0, 1.0); // white
	volumeColor->AddRGBPoint(20, 1.0, 1.0, 1.0); // white
	volumeColor->AddRGBPoint(25, 1.0, 1.0, 1.0); // white
	volumeColor->AddRGBPoint(50, 1.0, 1.0, 1.0); // white
	volumeColor->AddRGBPoint(60, 1.0, 1.0, 1.0); // white
	volumeColor->AddRGBPoint(75,  1.0, 0.9, 0.8);
	volumeColor->AddRGBPoint(150, 1.0, 0.4, 0.02);
	volumeColor->AddRGBPoint(175,  1.0, 0.6, 0.0);
	volumeColor->AddRGBPoint(200, 1.0, 0.7, 0.0);
	
	vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	volumeProperty->SetColor(volumeColor);
	volumeProperty->SetGradientOpacity(volumeGradientOpacity);
	volumeProperty->SetInterpolationTypeToLinear();
	volumeProperty->ShadeOn();
	volumeProperty->SetAmbient(0.4);
	volumeProperty->SetDiffuse(0.6);
	volumeProperty->SetSpecular(0.2);
	
	raycastVolume = vtkSmartPointer<vtkVolume>::New();
	raycastVolume->SetMapper(rayCastMapper);
	raycastVolume->SetProperty(volumeProperty);
	
	return raycastVolume;
}

vtkSmartPointer<vtkActor> MyImage3D::GetSegmentedImage()
{
	if (segmActor != NULL)
		return segmActor;

	segmReader = vtkSmartPointer<vtkStructuredPointsReader>::New();
	segmReader->SetFileName("vessels_seg.vtk");
	segmReader->Update();
	
	vtkSmartPointer<vtkContourFilter> contourFilter = vtkSmartPointer<vtkContourFilter>::New();
	contourFilter->SetInputConnection(segmReader->GetOutputPort()); 
	contourFilter->SetValue(0, 16.0f);
	contourFilter->Update();
	
	segmMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	segmMapper->SetInputConnection(contourFilter->GetOutputPort()); 
	segmMapper->ScalarVisibilityOff();
	
	segmActor = vtkSmartPointer<vtkActor>::New();
	segmActor->SetMapper(segmMapper);

	return segmActor;
}

vtkSmartPointer<vtkActor> MyImage3D::GetSegmentedOutline()
{
	if (outlineActor != NULL)
		return outlineActor;

	vtkSmartPointer<vtkOutlineFilter> outlineFilter = vtkSmartPointer<vtkOutlineFilter>::New();
	outlineFilter->SetInputData(segmReader->GetOutput());

	outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	outlineMapper->SetInputConnection(outlineFilter->GetOutputPort());
  
	outlineActor = vtkSmartPointer<vtkActor>::New();
	outlineActor->SetMapper(outlineMapper);
	outlineActor->GetProperty()->SetColor(0.2, 0.2, 0.2);

	return outlineActor;
}

vtkSmartPointer<vtkActor> MyImage3D::GetSkeletonImage()
{
	if (skelActor != NULL)
		return skelActor;

	if (skelReader == NULL)
	{
		skelReader = vtkSmartPointer<vtkStructuredPointsReader>::New();
		skelReader->SetFileName("vessels_skel.vtk");
		skelReader->Update();
	}
	
	vtkSmartPointer<vtkContourFilter> contourFilter = vtkSmartPointer<vtkContourFilter>::New();
	contourFilter->SetInputConnection(skelReader->GetOutputPort()); 
	contourFilter->SetValue(0, 8.0f);
	contourFilter->Update();

	skelMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	skelMapper->SetInputConnection(contourFilter->GetOutputPort()); 
	skelMapper->ScalarVisibilityOff();
	
	skelActor = vtkSmartPointer<vtkActor>::New();
	skelActor->SetMapper(skelMapper);

	return skelActor;
}

// Ref: http://public.kitware.com/pipermail/vtkusers/2003-October/020423.html
vector<vtkSmartPointer<vtkActor>> MyImage3D::GetTubedSkeleton(double radius, bool varyTubeRadiusByScalar, bool colorByScalar)
{	
	if (skelReader == NULL)
	{
		skelReader = vtkSmartPointer<vtkStructuredPointsReader>::New();
		skelReader->SetFileName("vessels_skel.vtk");
		skelReader->Update();
	}
	structuredPoints = skelReader->GetOutput();
	
	if(branches.size() == 0) { getImageData(); }

	vector<vtkSmartPointer<vtkActor>> actors;

	for(vector<vector<ushort>> branch : branches)
	{
		vtkSmartPointer<vtkPolyData> polyData = makePolyData(&branch);
		vtkSmartPointer<vtkTubeFilter> tubeFilter = makeTube(polyData, radius, true);

		// vary radius?
		if (varyTubeRadiusByScalar)
		{
			tubeFilter->SetVaryRadiusToVaryRadiusByScalar();
			tubeFilter->SetRadiusFactor(10);	
		}
		else
		{
			tubeFilter->SetVaryRadiusToVaryRadiusOff();
		}
		tubeFilter->Update();

		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetScalarRange(0,100);
		mapper->SetInputConnection(tubeFilter->GetOutputPort());

		// colored tubes?
		if (colorByScalar)
			mapper->ScalarVisibilityOn();
		else
			mapper->ScalarVisibilityOff();

		tubedSkeletonActor = vtkSmartPointer<vtkActor>::New();
		tubedSkeletonActor->SetMapper(mapper);

		actors.push_back(tubedSkeletonActor);

	}
	
	return actors;
}

/*
 * Read all voxel positions, branch by branch into 'branches'
 */
void MyImage3D::getImageData()
{
	// -------------Initializing------------//
	while (!voxelsToVisit.empty()) { voxelsToVisit.pop(); }
	structuredPoints->GetDimensions(dimensions);
	
	visited.clear();
	unsigned int imageSize = dimensions[0]*dimensions[1]*dimensions[2];
	for (unsigned int i = 0; i < imageSize; i++) visited.push_back(0);

	vector<ushort> voxel, endOfBranch, nextVoxel = makeVector(0, 0, 0);
	//---------------------------------------//

	while (true)
	{
		if (voxelsToVisit.empty() == false)
		{
			endOfBranch = voxelsToVisit.top();
			voxelsToVisit.pop();
		}
		
		else if (findNextVoxel(&nextVoxel) == true) 
		{ 
			// Important: ONLY change 'nextVoxel' in function 'findNextVoxel'!
			copyVoxelValues(&nextVoxel, &voxel);
			findEndOfBranch(&voxel, &endOfBranch);	 
		} 

		else { return; } // End of image

		vector<vector<ushort>> branch;
		getBranch(&endOfBranch, &branch);
			
		if (branch.size() > 1)
			branches.push_back(branch);
	}
}

/* Parameters: 'vox' should contain {0, 0, 0} the very first time this function is
 *             executed. It is then important that the content on the address of 'vox'
 *             is only changed in this function.
 * Returns: FALSE when the search is at the last voxel of the image, TRUE otherwise.
 */ 
bool MyImage3D::findNextVoxel(vector<ushort> * vox)
{
	ushort * voxelScalarValue;
	ushort x, y, z;

	for (x = vox->at(0); x < dimensions[0]; x++)
	{
		for (y = vox->at(1); y < dimensions[1]; y++)
		{
			for (z = vox->at(2); z < dimensions[2]; z++)
			{
				if (isVisited(x, y, z) == false)
				{
					voxelScalarValue = static_cast<ushort*>(structuredPoints->GetScalarPointer(x,y,z));
					if (voxelScalarValue[0] != 0)
					{
						(*vox) = makeVector(x, y, z);
						return true;
					}
					else { setVisited(x, y, z); }
				}
			}
		}
	}
	return false; 
}

/* TODO HAVE TO SECURE FOR LOOPS??
 * Parameters: when this function returns, the end voxel of the branch containing 'currentVoxel'
 *             is in 'endOfBranch'.
 */
void MyImage3D::findEndOfBranch(vector<ushort> * currentVoxel, vector<ushort> * endOfBranch)
{
	vector<ushort> parentVoxel = makeVector(dimensions[0]+1, dimensions[1]+1, dimensions[2]+1);
	while (true)
	{
		vector<vector<ushort> > voxelNeighbors;
		findVoxelNeighbors(currentVoxel, &voxelNeighbors);

		if (voxelNeighbors.size() == 1 || voxelNeighbors.size() > 2)
		{
			copyVoxelValues(currentVoxel, endOfBranch);
			return;
		}

		for (vector<ushort> neighbor : voxelNeighbors)
		{
			// Cannot go back to the previous voxel
			if ((neighbor.at(0) == parentVoxel.at(0) && 
				neighbor.at(1) == parentVoxel.at(1) && 
				neighbor.at(2) == parentVoxel.at(2)) == false)
			{
				copyVoxelValues(currentVoxel, &parentVoxel);
				copyVoxelValues(&neighbor, currentVoxel); // Go to next voxel
				break;
			}
		}
	}
}

/*
 * Parameters: the addresses of the neighboring voxels of 'currentVoxel' is
 *             put in vector 'neighbors'.
 */
void MyImage3D::findVoxelNeighbors(vector<ushort> * currentVoxel, vector<vector<ushort> > * neighbors)
{
	ushort a, b, c;
	a = currentVoxel->at(0); b = currentVoxel->at(1); c = currentVoxel->at(2);
	ushort* neighborVoxelScalar;
	short int x,y,z;

	for (x = -1; x <= 1; x++) // A cubical voxel has 26 neighbors
	{
		for (y = -1; y <= 1; y++)
		{
			for (z = -1; z <= 1; z++)
			{
				if (x == 0 && y == 0 && z == 0) { continue; }

				// Checking if the neighbor is inside the image boundries
				else if (a+x < 0 || a+x >= dimensions[0] || 
					    b+y < 0 || b+y >= dimensions[1] || 
					    c+z < 0 || c+z >= dimensions[2]) { continue; }

				if (isVisited(a+x, b+y, c+z) == false)
				{ // If a voxel scalar == 0, it is empty and thus no it is no neighbor
					neighborVoxelScalar = static_cast<ushort*>(structuredPoints->GetScalarPointer(a+x,b+y,c+z));
					if (neighborVoxelScalar[0] != 0) { neighbors->push_back(makeVector(a+x, b+y, c+z)); }
				}	
			}
		}
	}
}

/*
 * Parameters: The complete branch is in vector 'branch' when the function returns
 *             'currentVoxel' must be a voxel at the end of a branch to start with.
 */
void MyImage3D::getBranch(vector<ushort> * currentVoxel, vector<vector<ushort> > * branch)
{
	while (true)
	{
		setVisited(currentVoxel);
		branch->push_back((*currentVoxel)); // Building branch

		vector<vector<ushort> > neighbors;
		findVoxelNeighbors(currentVoxel, &neighbors);
		ushort neighborCount = neighbors.size();

		if (neighborCount == 0) // Isolated voxel OR end of branch found
			return;

		else if (neighborCount == 1) // Beginning or in the middle of branch
			copyVoxelValues(&neighbors.at(0), currentVoxel); 

		else if (neighborCount > 1) // This branch has met the ends of other branches == End of this branch
		{
			for (vector<ushort> neighbor : neighbors)
			{ 
				voxelsToVisit.push((*currentVoxel)); 
			} // Revisit neighbors later
			copyVoxelValues(&neighbors.at(0), currentVoxel);
		}
	}	
}

vtkSmartPointer<vtkPolyData> MyImage3D::makePolyData(vector<vector<ushort>> * branch)
{
	vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
	vtkSmartPointer<vtkPolyLine> polyLine;
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	vtkSmartPointer<vtkFloatArray> scalars = vtkSmartPointer<vtkFloatArray>::New();
	vector<ushort> vox;

	unsigned int b, l, v, branchSize, pointId = 0, scalarId = 0;
	double x, y, z; 
	ushort* scalar;

	branchSize = branch->size();

	// Make every branch its own polyline
	polyLine = vtkSmartPointer<vtkPolyLine>::New();
	polyLine->GetPointIds()->SetNumberOfIds(branchSize);

	for (l = 0; l < branchSize; l++)
   	{
		polyLine->GetPointIds()->SetId(l, pointId);
		pointId++;
	}
  	lines->InsertNextCell(polyLine);
   	//----------------------------------//

	for (int v = 0; v < branchSize; v++)
	{
		vox = branch->at(v);
		x = (double)vox.at(0); y = (double)vox.at(1); z = (double)vox.at(2);
			
		points->InsertNextPoint(x, y, z);
			
		scalar = static_cast<ushort*>(structuredPoints->GetScalarPointer(x, y, z));
		scalars->InsertTuple1(scalarId, (double) scalar[0]); scalarId++;
	}
		

	vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
	polyData->SetPoints(points);
	polyData->SetLines(lines);
	polyData->GetPointData()->SetScalars(scalars);

	return polyData;
}

vtkSmartPointer<vtkTubeFilter> MyImage3D::makeTube(vtkSmartPointer<vtkPolyData> polyData, double radius, bool makeSmooth)
{
	vtkSmartPointer<vtkTubeFilter> tube = vtkSmartPointer<vtkTubeFilter>::New();

	if (makeSmooth)
	{
		// Make the tubed skeleton smoother with the spline filter.
		vtkSmartPointer<vtkSplineFilter> splinedPolyData = vtkSmartPointer<vtkSplineFilter>::New(); 
		splinedPolyData->SetInputData(polyData);
		splinedPolyData->SetNumberOfSubdivisions(polyData->GetPoints()->GetNumberOfPoints()*4);
		splinedPolyData->Update();
		
		// Clean polylines who have coincident points. Otherwise some normals cannot be computed.
		vtkSmartPointer<vtkCleanPolyData> cleanPolyData = vtkSmartPointer<vtkCleanPolyData>::New();
		cleanPolyData->SetInputConnection(splinedPolyData->GetOutputPort());
		cleanPolyData->Update();
		tube->SetInputConnection(cleanPolyData->GetOutputPort());
	}
	else
	{ 
		tube->SetInputData(polyData);
	}

	tube->SetNumberOfSides(10);
	tube->SetRadius(radius);
	tube->CappingOn();
	return tube;
}

// --------------------------- //
// Additional helper functions //
// --------------------------- //

bool MyImage3D::copyVoxelValues(vector<ushort> * from, vector<ushort> * to)
{
	if (from->size() != 3) return false;
	to->clear();
	to->push_back(from->at(0)); to->push_back(from->at(1)); to->push_back(from->at(2));
	return true;
}

vector<ushort> MyImage3D::makeVector(ushort a, ushort b, ushort c)
{
	vector<ushort> v;
	v.push_back(a); v.push_back(b); v.push_back(c);
	return v;
}

/*
 *	Returns: TRUE if the voxel at image coordinated (x, y, z) already
 *           has been visited, otherwise FALSE.
 */
bool MyImage3D::isVisited(vector<ushort> * voxel)
{
	return isVisited(voxel->at(0), voxel->at(1), voxel->at(2));
}

bool MyImage3D::isVisited(ushort x, ushort y, ushort z)
{
	if (visited.at(z * dimensions[0] * dimensions[1] + x * dimensions[1] + y) == 0) return false;
	else return true;
}

/*
 * Parameters: visited is a bitset: one bit maps to one voxel.
 *             bit is set to 1 when corresponding voxel has been visited.
 */
void MyImage3D::setVisited(vector<ushort> * voxel) 
{
	setVisited(voxel->at(0), voxel->at(1), voxel->at(2));
}

void MyImage3D::setVisited(ushort x, ushort y, ushort z)
{
	visited.at(z * dimensions[0] * dimensions[1] + x * dimensions[1] + y) = 1;
}

vtkSmartPointer<vtkStructuredPointsReader> MyImage3D::GetSegmentedImageReader()
{
	return segmReader;
}