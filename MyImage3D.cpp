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

unsigned short& MyImage3D::Index(unsigned int _slices, unsigned int _rows, unsigned int _columns)
{
	int dims_xyz[3];
	this->vtk_image_data->GetDimensions(dims_xyz);

	assert((((unsigned int)_slices) < dims_xyz[2]) && (((unsigned int)_rows) < dims_xyz[1]) && (((unsigned int)_columns) < dims_xyz[0]));

	// Get the pointer to data
	unsigned short *pointer = (unsigned short*)(this->vtk_image_data->GetScalarPointer());

	return (pointer[(dims_xyz[0]*dims_xyz[1]*_slices + dims_xyz[0]*_rows + _columns)]);
}

void MyImage3D::FillInWith(unsigned short _value)
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

// Problem: many voxels don't have neighbors. They are just hanging loosely.
// Many voxels are neighborless because the skeleton image is not continuous.
void MyImage3D::FindVoxelNeighbors(int * currentVoxel, int * parentVoxel)
{
	int dims[3];
	structuredPoints->GetDimensions(dims);

	unsigned short int a,b,c, counter = 0;
	a = currentVoxel[0]; b = currentVoxel[1]; c = currentVoxel[2];

	unsigned short* nextVoxelScalar;
	
	short int x,y,z;

	for(x = -1; x <= 1; x++)
	{
		for(y = -1; y <= 1; y++)
		{
			for(z = -1; z <= 1; z++)
			{
				if(x == 0 && y == 0 && z == 0) continue;
				else if(a+x < 0 || a+x >= dims[0] || b+y < 0 || b+y >= dims[1] || c+z < 0 || c+z >= dims[2]) continue;

				nextVoxelScalar = static_cast<unsigned short*>(structuredPoints->GetScalarPointer(a+x,b+y,c+z));

				if(nextVoxelScalar[0] != 0)
				{
					if(a+x == parentVoxel[0] && b+y == parentVoxel[1] && c+z == parentVoxel[2]) continue;
					parentVoxel[0] = a+x; parentVoxel[1] = y+b; parentVoxel[2] = c+z;
					counter++;
					std::cout << "Found neighbor voxel (x,y,z)->(" << a+x << "," << b+y << "," << c+z << ")" << std::endl;
					currentVoxel[0] = x; currentVoxel[1] = y; currentVoxel[2] = z;
					//return;
				}
			}
		}
	}
	std::cout << "#Neighbors: " << counter << std::endl;
}

void MyImage3D::FindFirstUsedVoxel(int * vox1)
{
	unsigned short * vox;

	int dims[3];
	structuredPoints->GetDimensions(dims);
	for(int x = 0; x < dims[0]; x++)
	{
		for(int y = 0; y < dims[1]; y++)
		{
			for(int z = 0; z < dims[2]; z++)
			{
				vox = static_cast<unsigned short*>(structuredPoints->GetScalarPointer(x,y,z));
				if(vox[0] != 0)
				{
					vox1[0] = x; vox1[1] = y; vox1[2] = z;
					return; 
				}
			}
		}
	}
}

// work in progress...
void MyImage3D::PointC()
{
	skelReader = vtkSmartPointer<vtkStructuredPointsReader>::New();
	string  vessels_skel_file = "vessels_skel.vtk";

	skelReader->SetFileName(vessels_skel_file.c_str());
	skelReader->Update();
	
	structuredPoints = skelReader->GetOutput();

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
	
	int parentVox[3] = {-1,-1,-1};
	int vox[3];
	FindFirstUsedVoxel(vox);

	std::cout << "Found 1st voxel (PointC) (x,y,z)->(" << vox[0] << "," << vox[1] << "," << vox[2] << ")" << std::endl; // Works
	
	for(int i = 0; i < 5; i++) FindVoxelNeighbors(vox, parentVox);



	vtkSmartPointer<vtkTubeFilter> tubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
	

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

	segmMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	segmMapper->SetInputConnection(contourFilter->GetOutputPort()); 
	segmMapper->ScalarVisibilityOff();
	
	skelActor = vtkSmartPointer<vtkActor>::New();
	skelActor->SetMapper(segmMapper);

	return skelActor;
}

vtkSmartPointer<vtkActor> MyImage3D::GetSegmentedOutline()
{
	if (outlineActor != NULL)
		return outlineActor;

	vtkSmartPointer<vtkOutlineFilter> outlineFilter = vtkSmartPointer<vtkOutlineFilter>::New();
	outlineFilter->SetInputData(segmMapper->GetInput());

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