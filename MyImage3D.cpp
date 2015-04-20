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

vtkSmartPointer<vtkActor> MyImage3D::LoadDataImage()
{
	string vesselsDataFile = "vessels_data.vtk";

	reader->SetFileName(vesselsDataFile.c_str());
	reader->Update();
 
	geometryFilter->SetInputConnection(reader->GetOutputPort());
	geometryFilter->Update();

	mapper->SetInputConnection(geometryFilter->GetOutputPort());
	actor->SetMapper(mapper);

	return actor;
}

vtkSmartPointer<vtkActor> MyImage3D::LoadSegmentedImage()
{
	string vesselsSegFile = "vessels_seg.vtk";

	reader->SetFileName(vesselsSegFile.c_str());
	reader->Update();

	iso->SetInputConnection(reader->GetOutputPort()); 
	iso->SetValue(0, 128.0f);
	iso->Update();
	isoMapper->SetInputConnection(iso->GetOutputPort()); 
	isoMapper->ScalarVisibilityOff();

	actor->SetMapper(isoMapper);

	return actor;
}

vtkSmartPointer<vtkActor> MyImage3D::LoadSkeletonImage()
{
	string vesselsSegFile = "vessels_skel.vtk";

	reader->SetFileName(vesselsSegFile.c_str());
	reader->Update();

	return actor;
}