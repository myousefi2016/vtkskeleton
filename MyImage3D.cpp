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

// Work in progress....
vtkSmartPointer<vtkLODActor> MyImage3D::SetLOD()
{
	vtkSmartPointer<vtkLODActor> lodActor = vtkSmartPointer<vtkLODActor>::New();

	//vtkSmartPointer<vtkOutlineFilter> lowResFilter = vtkSmartPointer<vtkOutlineFilter>::New();
	//vtkSmartPointer<vtkMaskPoints> medResFilter = vtkSmartPointer<vtkMaskPoints>::New();
	
	//lodActor->SetLowResFilter(lowResFilter);
	//lodActor->SetMediumResFilter(medResFilter);

	return lodActor;
}

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
	//tkSmartPointer<vtkVolumeRayCastIsosurfaceFunction> rcIsosurfaceFun = vtkSmartPointer<vtkVolumeRayCastIsosurfaceFunction>::New();
	//vtkSmartPointer<vtkVolumeRayCastCompositeFunction> rcCompositeFun = vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();
	vtkSmartPointer<vtkVolumeRayCastMIPFunction> rcMipFun = vtkSmartPointer<vtkVolumeRayCastMIPFunction>::New();

	rayCastMapper->SetVolumeRayCastFunction(rcMipFun);

	raycastVolume = vtkSmartPointer<vtkVolume>::New();
	raycastVolume->SetMapper(rayCastMapper);

	vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();

	vtkSmartPointer<vtkColorTransferFunction> volumeColor = vtkSmartPointer<vtkColorTransferFunction>::New();
	// TODO Haven't really figured this stuff out yet. Maps voxel intensity to colors.
	volumeColor->AddRGBPoint(5,    2.0, 1.0, 1.0);
	volumeColor->AddRGBPoint(100, 0.1, 0.5, 0.3);
	volumeColor->AddRGBPoint(200, 0.5, 0.9, 0.0);
	volumeColor->AddRGBPoint(300,  0.1, 0.5, 0.5);
	
	volumeProperty->SetColor(volumeColor);
	//volumeProperty->ShadeOn(); // Doesn't make any difference in our case.
	//volumeProperty->SetAmbient(100); // Can't see what this does either
	//volumeProperty->SetDiffuse(0.9); // Neither with this...
	//volumeProperty->SetSpecular(20);
	//volumeProperty->SetSpecularPower(10);

	raycastVolume->SetProperty(volumeProperty);
	
	//return vtkSmartPointer<vtkVolume>::New();
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

	//vtkSmartPointer<vtkPolyData> polyData = polydataMapper->GetInput();
	//for(vtkIdType i = 0; i < polyData->GetNumberOfPoints(); i++)
 //   {
	//	double p[3];
	//	polyData->GetPoint(i,p);
	//	// This is identical to:
	//	// polydata->GetPoints()->GetPoint(i,p);
	//	std::cout << "Point " << i << " : (" << p[0] << " " << p[1] << " " << p[2] << ")" << std::endl;
 //   }
	
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
	outlineActor->GetProperty()->SetColor(0, 0, 0);

	return outlineActor;
}

vtkSmartPointer<vtkStructuredPointsReader> MyImage3D::GetSegmentedImageReader()
{
	return segmReader;
}