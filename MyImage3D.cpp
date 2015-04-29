/**************************************************************************
 3D Image class for unsigned short (16-bit unsigned) values

 Author: Danilo Babin
 File name: "MyImage3D.cpp"
***************************************************************************/

#include "MyImage3D.h"

int MyImage3D::IsEmpty()
{
	int dims_xyz[3];
	this->vtk_image_data->GetDimensions(dims_xyz);

	if (dims_xyz[0] == 0 || dims_xyz[1] == 0 || dims_xyz[2] == 0) return 1;
	else return 0;
}

void MyImage3D::Set(unsigned int _slices, unsigned int _rows, unsigned int _columns)
{
	assert((_slices > 0) && (_rows > 0) && (_columns > 0));

	int dims_xyz[3];
	this->vtk_image_data->GetDimensions(dims_xyz);

	// If the size is already the required size, just return
	if((_slices==dims_xyz[2]) && (_rows==dims_xyz[1]) && (_columns==dims_xyz[0])) return;

	// Allocate memory in a sequential way using VTK
	// this->vtk_image_data->SetExtent(0,_columns-1,0,_rows-1,0,_slices-1);
	this->vtk_image_data->SetDimensions(_columns,_rows,_slices);
	vtkInformation *info = this->vtk_image_data->GetInformation();//vtkInformation::New();//IZMENA!!!	
	this->vtk_image_data->SetPointDataActiveScalarInfo(info,VTK_UNSIGNED_SHORT,1);//IZMENA!!!
	//this->vtk_image_data->SetScalarTypeToUnsignedShort();
	//this->vtk_image_data->SetNumberOfScalarComponents(1);
	this->vtk_image_data->AllocateScalars(VTK_UNSIGNED_SHORT,1);
	this->vtk_image_data->SetSpacing(1,1,1);//Set spacing to default 	
	this->vtk_image_data->SetOrigin(0,0,0);//Set origin to default 
}

ushort& MyImage3D::Index(unsigned int _slices, unsigned int _rows, unsigned int _columns)
{
	int dims_xyz[3];
	this->vtk_image_data->GetDimensions(dims_xyz);

	assert((((unsigned int)_slices) < dims_xyz[2]) && (((unsigned int)_rows) < dims_xyz[1]) && (((unsigned int)_columns) < dims_xyz[0]));

	// Get the pointer to data
	ushort *pointer = (ushort*)(this->vtk_image_data->GetScalarPointer());

	return (pointer[(dims_xyz[0]*dims_xyz[1]*_slices + dims_xyz[0]*_rows + _columns)]);
}

void MyImage3D::FillInWith(ushort _value)
{
	assert(!this->IsEmpty());

	for(int s = 0; s < this->vtk_image_data->GetDimensions()[2]; s++)
	{
		for(int r = 0; r < this->vtk_image_data->GetDimensions()[1]; r++)
		{
			for(int c = 0; c < this->vtk_image_data->GetDimensions()[0]; c++)
			{
				this->Index(s,r,c) = _value;
			}
		}
	}
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
bool MyImage3D::isVisited(ushort x, ushort y, ushort z)
{
	if(visited.at(z*dimensions[0]*dimensions[1]+x*dimensions[1]+y) == 0) return false;
	else return true;
}

bool MyImage3D::isVisited(ushort * voxel)
{ return isVisited(voxel[0], voxel[1], voxel[2]); }
bool MyImage3D::isVisited(vector<ushort> * voxel)
{ return isVisited(voxel->at(0), voxel->at(1), voxel->at(2)); }

/*
 * Parameters: visited is a bitset: one bit maps to one voxel.
 *             bit is set to 1 when corresponding voxel has been visited.
 */
void MyImage3D::setVisited(ushort x, ushort y, ushort z)
{ visited.at(z*dimensions[0]*dimensions[1]+x*dimensions[1]+y) = 1; }

void MyImage3D::setVisited(ushort * voxel) 
{ setVisited(voxel[0], voxel[1], voxel[2]); }
void MyImage3D::setVisited(vector<ushort> * voxel) 
{ setVisited(voxel->at(0), voxel->at(1), voxel->at(2)); }


/*
 * Parameters: the addresses of the neighboring voxels of 'currentVoxel' is
 *             put in vector 'neighbors'.
 */
void MyImage3D::findVoxelNeighbors(ushort * currentVoxel, vector<vector<ushort> > * neighbors)
{
	ushort a, b, c;
	a = currentVoxel[0]; b = currentVoxel[1]; c = currentVoxel[2];
	ushort* neighborVoxelScalar;
	short int x,y,z;

	for(x = -1; x <= 1; x++)
	{
		for(y = -1; y <= 1; y++)
		{
			for(z = -1; z <= 1; z++)
			{
				if(x == 0 && y == 0 && z == 0) { continue; }
				else if(a+x < 0 || a+x >= dimensions[0] || 
					    b+y < 0 || b+y >= dimensions[1] || 
					    c+z < 0 || c+z >= dimensions[2]) { continue; }

				neighborVoxelScalar = static_cast<ushort*>(structuredPoints->GetScalarPointer(a+x,b+y,c+z));

				if(neighborVoxelScalar[0] != 0) { neighbors->push_back(makeVector(a+x, b+y, c+z)); }
			}
		}
	}
}

/*
 * Parameters: when this function returns, the end voxel of the branch containing 'currentVoxel'
 *             is in 'endOfBranch'.
 */
void MyImage3D::findEndOfBranch(ushort * endOfBranch, ushort * currentVoxel)
{
	ushort parentVoxel[3];
	while(true)
	{
		vector<vector<ushort> > voxelNeighbors;
		findVoxelNeighbors(currentVoxel, &voxelNeighbors);

		if(voxelNeighbors.size() == 1) 
		{
			endOfBranch[0] = currentVoxel[0]; endOfBranch[1] = currentVoxel[1]; endOfBranch[2] = currentVoxel[2];
			return;
		}

		for(vector<ushort> neighbor : voxelNeighbors)
		{
			if((neighbor.at(0) == parentVoxel[0] && neighbor.at(1) == parentVoxel[1] && neighbor.at(2) == parentVoxel[2]) == false)
			{
				parentVoxel[0] = currentVoxel[0]; parentVoxel[1] = currentVoxel[1]; parentVoxel[2] = currentVoxel[2];
				currentVoxel[0] = neighbor.at(0); currentVoxel[1] = neighbor.at(1); currentVoxel[2] = neighbor.at(2);
			}
		}
	}
}

/* Parameters: 'vox' should contain {0, 0, 0} the very first time this function is
 *             executed. It is then important that the content on the address of 'vox'
 *             is only changed in this function.
 * Returns: FALSE when the search is at the last voxel of the image, TRUE otherwise.
 */ 
bool MyImage3D::findNextVoxel(ushort * vox)
{
	ushort * voxelScalarValue;
	ushort x, y, z;

	for(x = vox[0]; x < dimensions[0]; x++)
	{
		for(y = vox[1]; y < dimensions[1]; y++)
		{
			for(z = vox[2]; z < dimensions[2]; z++)
			{
				if(isVisited(x, y, z) == false)
				{
					voxelScalarValue = static_cast<ushort*>(structuredPoints->GetScalarPointer(x,y,z));
					if(voxelScalarValue[0] != 0)
					{
						//cout << "Found in func findNextVoxel: " << x << ", " << y << ", " << z << endl;
						vox[0] = x; vox[1] = y; vox[2] = z;
						return true;
					}
					else { setVisited(x, y, z); }
				}
			}
		}
	}
	return false; 
}

/*
 * Parameters: The complete branch is in vector 'branch' when the function returns
 *             'currentVoxel' must be a voxel at the end of a branch to start with.
 */
void MyImage3D::getBranch(vector<vector<ushort> > * branch, ushort * currentVoxel)
{
	while(true)
	{
		branch->push_back(makeVector(currentVoxel[0], currentVoxel[1], currentVoxel[2])); // Building branch
		setVisited(currentVoxel[0], currentVoxel[1], currentVoxel[2]);

		vector<vector<ushort> > neighbors;
		findVoxelNeighbors(currentVoxel, &neighbors);
		ushort neighborCount = neighbors.size();

		if(neighborCount == 0) { return; } // currentVoxel is an isolated voxel --> Ignore

		else if(neighborCount == 1) // currentVoxel is at the beginning or end of branch
		{
			vector<ushort> neighbor = neighbors.at(0);
			if(isVisited(&neighbor) == false)
			{
				branch->push_back(neighbor);
				currentVoxel[0] = neighbor[0]; currentVoxel[1] = neighbor[1]; currentVoxel[2] = neighbor[2];
				setVisited(&neighbor);
			}

			else { return; } // Only neighbor already visited == At the end of branch
		}

		else if(neighborCount == 2) // currentVoxel is in the middle of a branch
		{
			vector<ushort> neighbor = neighbors.at(0);
			if(isVisited(&neighbor) == true) 
				{ neighbor = neighbors.at(1); }

			branch->push_back(neighbor);
			currentVoxel[0] = neighbor[0]; currentVoxel[1] = neighbor[1]; currentVoxel[2] = neighbor[2];
			setVisited(&neighbor);
		}

		else if(neighborCount > 2) // This branch has met the ends of other branches == End of this branch
		{
			for(vector<ushort> neighbor : neighbors)
			{
				if(isVisited(&neighbor) == false)
				{
					voxelsToVisit.push(neighbor);
				}
			}
			return; 
		}
	}	
}

// TODO work in progress... Name of function is preliminary.
// should take a look here for tubefilter info: http://public.kitware.com/pipermail/vtkusers/2003-October/020423.html
void MyImage3D::PointC()
{
	// -------------Initializing------------//
	skelReader = vtkSmartPointer<vtkStructuredPointsReader>::New();
	string vessels_skel_file = "vessels_skel.vtk";
	skelReader->SetFileName(vessels_skel_file.c_str());
	skelReader->Update();
	structuredPoints = skelReader->GetOutput();

	while(!voxelsToVisit.empty()) { voxelsToVisit.pop(); }
	structuredPoints->GetDimensions(dimensions);
	 // TODO find other solution

	visited.clear();
	int imageSize = dimensions[0]*dimensions[1]*dimensions[2];
	for(int i = 0; i < imageSize; i++) visited.push_back(0);

	//cout << "Dimensions: " << dimensions[0] << " x " << dimensions[1] << " x " << dimensions[2] << endl;
	//cout << "Visited size: " << visited.size() << endl;

	ushort nextVoxel[3] = {0, 0, 0};
	ushort endOfBranch[3];
	ushort voxel[3];

	vtkSmartPointer<vtkTubeFilter> tubeFilter = vtkSmartPointer<vtkTubeFilter>::New();



	// testing...
	//findNextVoxel(nextVoxel);
	//cout << "pointC: Next voxel found: " << nextVoxel[0] << ", " << nextVoxel[1] << ", " << nextVoxel[2] << endl;

	/*vector<int*> neighbors;
	voxel[0] = nextVoxel[0]; voxel[1] = nextVoxel[1]; voxel[2] = nextVoxel[2];
	findVoxelNeighbors(voxel, &neighbors);

	
	cout << "#Neighbors: " << neighbors.size() << endl;
	
	findEndOfBranch(endOfBranch, voxel);

	cout << "End of branch: " << endOfBranch[0] << ", " << endOfBranch[1] << ", " << endOfBranch[2] << endl;

	for(int* neighbor : neighbors) { cout << "for-loop: " << neighbor[0] << ", " << neighbor[1] << ", " << neighbor[2] << endl; }
	cout << "for-loop2: " << neighbors.at(0)[0] << ", " << neighbors.at(0)[1]  << ", " << neighbors.at(0)[2]  << endl;
	for(int i = 0; i< neighbors.size(); i++) { cout << "for-loop3: " << neighbors.at(i)[0] << ", " << neighbors.at(i)[1]  << ", " << neighbors.at(i)[2]  << endl; }

	/////////////

	//---------------------------------------//
	
	/*while(true)
	{
		if(voxelsToVisit.empty() == false)
		{
			endOfBranch[0] = voxelsToVisit.top();
			voxelsToVisit.pop();
		}
		
		else if(findNextVoxel(nextVoxel, &visited) == true) 
		{ 
			// Important: ONLY change 'nextVoxel' in 'findNextVoxel'!
			voxel[0] = nextVoxel[0];voxel[1] = nextVoxel[1]; voxel[2] = nextVoxel[2];
			findEndOfBranch(endOfBranch, voxel);
			 
		} 

		else { break; } // If break == End of image

		vector<int*> branch;
		getBranch(&branch, endOfBranch, &visited);

		// TODO next I have to make some kind of line source out of every branch... and put it onto the filter.
	}*/
}


// Work in progress....
vtkSmartPointer<vtkLODActor> MyImage3D::SetLOD()
{
	vtkSmartPointer<vtkLODActor> lodActor = vtkSmartPointer<vtkLODActor>::New();

	vtkSmartPointer<vtkOutlineFilter> lowResFilter = vtkSmartPointer<vtkOutlineFilter>::New();
	vtkSmartPointer<vtkMaskPoints> medResFilter = vtkSmartPointer<vtkMaskPoints>::New();
	
	lodActor->SetLowResFilter(lowResFilter);
	
	lodActor->SetMediumResFilter(medResFilter);
	lodActor->SetNumberOfCloudPoints(20);

	return lodActor;
}

// Ref: https://github.com/Kitware/VTK/tree/Examples/Cxx/Visualization/RenderPassExistingContext
vtkSmartPointer<vtkVolume> MyImage3D::GetRayCastingImage()
{
	if (raycastVolume != NULL)
		return raycastVolume;

	string vesselsDataFile = "vessels_data.vtk";

	dataReader = vtkSmartPointer<vtkStructuredPointsReader>::New();
	dataReader->SetFileName(vesselsDataFile.c_str());
	dataReader->Update();
 
	vtkSmartPointer<vtkVolumeRayCastMapper> rayCastMapper = vtkSmartPointer<vtkVolumeRayCastMapper>::New();
	rayCastMapper->SetInputConnection(dataReader->GetOutputPort());

	// I may try these other two functions later...
	//vtkSmartPointer<vtkVolumeRayCastIsosurfaceFunction> rcIsosurfaceFun = vtkSmartPointer<vtkVolumeRayCastIsosurfaceFunction>::New();
	//vtkSmartPointer<vtkVolumeRayCastCompositeFunction> rcCompositeFun = vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();
	vtkSmartPointer<vtkVolumeRayCastMIPFunction> rcMipFun = vtkSmartPointer<vtkVolumeRayCastMIPFunction>::New();

	rayCastMapper->SetVolumeRayCastFunction(rcMipFun);

	raycastVolume = vtkSmartPointer<vtkVolume>::New();
	raycastVolume->SetMapper(rayCastMapper);
	
	vtkSmartPointer<vtkColorTransferFunction> volumeColor = vtkSmartPointer<vtkColorTransferFunction>::New();
	volumeColor->AddRGBPoint(0,   0.0, 1.0, 0.0);
	volumeColor->AddRGBPoint(10, 1.0, 1.0, 1.0); // the biggest group of points = white
	volumeColor->AddRGBPoint(20, 1.0, 1.0, 1.0); // the biggest group of points = white
	volumeColor->AddRGBPoint(25, 1.0, 1.0, 1.0); // the biggest group of points = white
	volumeColor->AddRGBPoint(75,  1.0, 0.9, 0.8);
	volumeColor->AddRGBPoint(150, 1.0, 0.2, 0.2);
	volumeColor->AddRGBPoint(200, 1.0, 0.7, 0.0);
	
	vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
	volumeProperty->SetColor(volumeColor);
	
	raycastVolume->SetProperty(volumeProperty);
	
	return raycastVolume;
}

vtkSmartPointer<vtkActor> MyImage3D::GetSegmentedImage()
{
	if (segmActor != NULL)
		return segmActor;

	string vesselsSegFile = "vessels_seg.vtk";
	
	segmReader = vtkSmartPointer<vtkStructuredPointsReader>::New();
	segmReader->SetFileName(vesselsSegFile.c_str());
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

vtkSmartPointer<vtkActor> MyImage3D::GetSkeletonImage()
{
	if (skelActor != NULL)
		return skelActor;

	string vesselsSegFile = "vessels_skel.vtk";
	
	skelReader = vtkSmartPointer<vtkStructuredPointsReader>::New();
	skelReader->SetFileName(vesselsSegFile.c_str());
	skelReader->Update();
	
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

vtkSmartPointer<vtkStructuredPointsReader> MyImage3D::GetSegmentedImageReader()
{
	return segmReader;
}



/*double p[3];
	structuredPoints->GetOrigin(p);
	std::cout << "Origin point (x,y,z)->(" << p[0] << "," << p[1] << "," << p[2] << ")" << std::endl; // Works
	int ID = structuredPoints->FindPoint(p);
	std::cout << "Origin ID : " << ID << std::endl;*/

	//vtkSmartPointer<vtkInformation> info = vtkSmartPointer<vtkInformation>::New();
	//structuredPoints->GetScalarType(info);
	
	//std::cout << "Scalar type: " << structuredPoints->GetScalarTypeAsString() << std::endl;
	//std::cout << "#Scalar components: " << structuredPoints->GetNumberOfScalarComponents() << std::endl;
	
	//structuredPoints->GetPoint(ID+1, p);
	//std::cout << "Next point (x,y,z)->(" << p[0] << "," << p[1] << "," << p[2] << ")" << std::endl; // Works

	//int dims[3];
	//structuredPoints->GetDimensions(dims);
	//std::cout << "Dimensions (x,y,z)->(" << dims[0] << "," << dims[1] << "," << dims[2] << ")" << std::endl; // Works
