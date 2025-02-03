

#include <iostream>
#include <fstream>

#include <direct.h>
#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#include <string.h>

#include <vector>
#include "CImg.h"


#include "itkImage.h"
#include "itkPoint.h"
#include "itkResampleImageFilter.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkVersorRigid3DTransform.h"
#include "itkCenteredTransformInitializer.h"
#include "itkImageDuplicator.h"

#include <stdio.h>
#include <iostream>

using namespace std;


const	unsigned int	spaceDimension = 3;
typedef float			PixelType;
typedef itk::Image< PixelType, spaceDimension >	ImageType;
typedef itk::Image< PixelType, 2 >	mipImageType;


using namespace std;
using namespace cimg_library;



int main(){
	cout << " This program transforms a 3D MIP Mask into a 3D Volume Mask into PET space. " << endl;
	cout << " It accepts 3D MIPs with number of projections = 30. " << endl << endl;
	cout << " Claudia Constantino " << endl << endl;

	//interface with user
	char dirImages[1024];
	cout<<"Introduce the name of the folder where the PET images are: ";
	cin>>dirImages;

	char dirMasks[1024];
	cout<<"Introduce the name of the 3D MIP segmentation masks: ";
	cin>>dirMasks;

	int threshold;
	cout << "Introduce the threshold value for how many intersections is needed to perform segmentation (default is 15): ";
	cin >> threshold;

	//number of projections: 
	//int nProj = 4;
	int nProj = 30;
	

	vector<string> imagesNameList, maskNameList;

	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	TCHAR szDir[MAX_PATH];
	size_t length_of_arg;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError=0;



	if(_chdir(dirImages) == -1){//Change to the specified folder, if it exists
		std::cerr <<endl<< "The directory "<<dirImages<<" does not exist!!"<<endl; 
		return 1;
	}
	else _chdir("..");//return to base directory

	std::string dirname(dirImages);
	TCHAR *DIRname = new TCHAR[dirname.size()+1]; //TCHAR stands for char (simple character of 1 byte); "Type + char"= TCHAR
	DIRname[dirname.size()]=0;
	std::copy(dirname.begin(),dirname.end(),DIRname);

	StringCchLength(DIRname, MAX_PATH, &length_of_arg);

	if (length_of_arg > (MAX_PATH - 3)){
		_tprintf(TEXT("\nDirectory path is too long.\n"));
	}
	
	_tprintf(TEXT("Target directory is: %s\n"), DIRname);
	 
	// Prepare string for use with FindFile functions.  
	StringCchCopy(szDir, MAX_PATH, DIRname);
	StringCchCat(szDir, MAX_PATH, TEXT("\\*")); 
	 
	// Find the first file in the directory.
	hFind = FindFirstFile(szDir, &ffd);
	
	// List all the files in the directory with some info about them.
	int validFiles = 0;
	int nFiles = 0;

	do{ 
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {//. and .. directories
			//_tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
		}
		else {
			++nFiles;
			filesize.LowPart = ffd.nFileSizeLow;
			filesize.HighPart = ffd.nFileSizeHigh;
			//_tprintf(TEXT("%s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);
			//_tprintf(TEXT("\n\n\n(%i)  %s\n"), nFiles, ffd.cFileName);
			char name[1024];
			int n=0;
			do {
				name[n] = (char)ffd.cFileName[n];
				//cout<<ffd.cFileName[n]<<"  ";
				++n;
			} while((char) ffd.cFileName[n] != '\0');
			name[n] = '\0';

			int nameLength = strlen(name); 
			if((toupper(name[nameLength-3]) == 'D' && toupper(name[nameLength-2]) == 'C' && toupper(name[nameLength-1]) == 'M') ||
				(toupper(name[nameLength-3]) == 'N' && toupper(name[nameLength-2]) == 'I' && toupper(name[nameLength-1]) == 'I')||
				(toupper(name[nameLength-4]) == 'N' && toupper(name[nameLength-3]) == 'R' && toupper(name[nameLength-2]) == 'R' &&
				 toupper(name[nameLength-1]) == 'D' ||
				(toupper(name[nameLength - 6]) == 'N' && toupper(name[nameLength - 5]) == 'I' && toupper(name[nameLength - 4]) == 'I') &&
				 toupper(name[nameLength - 3]) == '.' && toupper(name[nameLength - 2]) == 'G' && toupper(name[nameLength - 1]) == 'Z')) {
				++ validFiles;
				char * url;
				url = new char[dirname.length() + 1];
				strcpy(url, dirname.c_str()); 
				char filename[1024];
				strcpy(filename, url); 
				strcat(filename, "/"); 
				strcat(filename, name);
				imagesNameList.push_back(filename); 
				delete[] url;
			}
		}
	}while (FindNextFile(hFind, &ffd) != 0); 
	cout<<"Total files: "<<nFiles<<";  Medical image files (.dcm, .nii and .nrrd): "<<validFiles<<endl;

	if(validFiles == 0){ 
		printf("\nNone valid file! Close the window.\n");
		char aux;
		cin>>aux; 
		return 2;
	}

	//Masks directory
	int validFiles2 = 0; 
	if(_chdir(dirMasks) == -1){ //Change to the specified folder, if it exists
		std::cerr <<endl<< "The directory "<<dirMasks<<" does not exist!!"<<endl;
		cout<<"Do you want to do the segmentation in the full image (y/n)? "<<endl; //cout used to display the output to the monitor
		char option;
		cin>>option;
		if(option == 'n' || option == 'N') return 3;
	}
	else{
		_chdir("..");//return to base directory
		std::string dirname2(dirMasks);
		TCHAR *DIRname2 = new TCHAR[dirname2.size()+1];
		DIRname2[dirname2.size()]=0;
		std::copy(dirname2.begin(),dirname2.end(),DIRname2);

		StringCchLength(DIRname2, MAX_PATH, &length_of_arg);

		if (length_of_arg > (MAX_PATH - 3)){
			_tprintf(TEXT("\nDirectory path is too long.\n"));
		}
		
		_tprintf(TEXT("Auxiliary directory is: %s\n"), DIRname2);
		 
		// Prepare string for use with FindFile functions.  First, copy the
		// string to a buffer, then append '\*' to the directory name. As was done for the directory of the images, here is the same to the masks
		StringCchCopy(szDir, MAX_PATH, DIRname2);
		StringCchCat(szDir, MAX_PATH, TEXT("\\*"));
		 
		// Find the first file in the directory.
		hFind = FindFirstFile(szDir, &ffd);
		
		// List all the files in the directory with some info about them.
		int nFiles2 = 0;

		do{
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {//. and .. directories
				//_tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
			}
			else {
				++nFiles2;
				filesize.LowPart = ffd.nFileSizeLow;
				filesize.HighPart = ffd.nFileSizeHigh;
				//_tprintf(TEXT("%s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);
				//_tprintf(TEXT("\n\n\n(%i)  %s\n"), nFiles, ffd.cFileName);
				char name[1024];
				int n=0;
				do {
					name[n] = (char)ffd.cFileName[n];
					//cout<<ffd.cFileName[n]<<"  ";
					++n;
				} while((char) ffd.cFileName[n] != '\0');
				name[n] = '\0';

				int nameLength = strlen(name); 
				if((toupper(name[nameLength-3]) == 'D' && toupper(name[nameLength-2]) == 'C' && toupper(name[nameLength-1]) == 'M') ||
						(toupper(name[nameLength-3]) == 'N' && toupper(name[nameLength-2]) == 'I' && toupper(name[nameLength-1]) == 'I') ||
						(toupper(name[nameLength-4]) == 'N' && toupper(name[nameLength-3]) == 'R' && toupper(name[nameLength-2]) == 'R' &&
						 toupper(name[nameLength-1]) == 'D') ||
						(toupper(name[nameLength - 6]) == 'N' && toupper(name[nameLength - 5]) == 'I' && toupper(name[nameLength - 4]) == 'I') &&
						 toupper(name[nameLength - 3]) == '.' && toupper(name[nameLength - 2]) == 'G' && toupper(name[nameLength - 1]) == 'Z'){
					++ validFiles2;		
					char * url;
					url = new char[dirname2.length() + 1];
					strcpy(url, dirname2.c_str()); 
					char filename[1024];
					strcpy(filename, url);
					strcat(filename, "/");
					strcat(filename, name);
					maskNameList.push_back(filename); 
					delete[] url;
				}
			}
		}while (FindNextFile(hFind, &ffd) != 0);
		cout<<"Total files: "<<nFiles2<<";  Mask image files (.dcm, .nii and .nrrd): "<<validFiles2<<endl;
	}

	if(validFiles2 == 0){ 
		cout<<endl<<"!!There is no masks!!"<<endl;
		cout<<"Do you want ignore them and continue (y/n)? ";
		char option;
		cin>>option;
		if(option == 'n' || option == 'N') return 4;
	}
	else if(validFiles != validFiles2){ 
		cout<<endl<<"!!The number of masks is different from the number of images!!"<<endl;
		cout<<"Do you want ignore the masks and continue (y/n)? ";
		char option;
		cin>>option;
		if(option == 'n' || option == 'N') return 5;
	}

	std::sort(imagesNameList.begin(), imagesNameList.end());
	std::sort(maskNameList.begin(), maskNameList.end());

	//this is to help on the creation of images names
	stringstream stream;
	stream << validFiles;
	string str2;
	stream >> str2;
	const char* str3 = str2.c_str();
	int len = strlen(str3);

	for (int i = 0; i < validFiles; ++i) {

		printf("\n\nPET Image: %d Name: %s\n", i, imagesNameList[i].c_str());
		printf(" Mask: %d Name: %s\n", i, maskNameList[i].c_str());

		ImageType::Pointer inputImage = ImageType::New();
		ImageType::Pointer inputMask = ImageType::New();

		//Read PET image
		char imageName[1024];
		strcpy(imageName, imagesNameList[i].c_str());
		typedef itk::ImageFileReader<ImageType>	ImageReaderType;
		ImageReaderType::Pointer reader1 = ImageReaderType::New();
		reader1->SetFileName(imageName);
		try {
			reader1->Update();
		}
		catch (itk::ExceptionObject& err) {
			std::cerr << "ExceptionObject caught !" << std::endl;
			std::cerr << err << std::endl;
			return 6;
		}
		inputImage = reader1->GetOutput(); 


		//Read segmentation mask
		char maskName[1024];
		strcpy(maskName, maskNameList[i].c_str());
		ImageReaderType::Pointer reader2 = ImageReaderType::New();
		reader2->SetFileName(maskName);
		try {
			reader2->Update();
		}
		catch (itk::ExceptionObject& err) {
			std::cerr << "ExceptionObject caught !" << std::endl;
			std::cerr << err << std::endl;
			return 7;
		}
		inputMask = reader2->GetOutput(); 


		cout << endl << inputImage->GetDirection() << endl;
		cout << inputImage->GetSpacing() << endl;

		/*
		Just an experience to check for reconstruction artifacts
		ImageType::SizeType  sizeMask = inputMask->GetLargestPossibleRegion().GetSize();
		for (int x = 0; x < sizeMask[0]; ++x) {
			for (int y = 0; y < sizeMask[1]; ++y) {
				for (int z = 0; z < sizeMask[2]; ++z) {
					ImageType::IndexType index;
					index[0] = x; index[1] = y; index[2] = z;
					inputMask->SetPixel(index, 1);
				}
			}
		}
		*/


		// Mask
		ImageType::SizeType  size = inputImage->GetLargestPossibleRegion().GetSize();
		typedef itk::ImageDuplicator<ImageType> DuplicatorType;
		DuplicatorType::Pointer duplicator = DuplicatorType::New();
		duplicator->SetInputImage(inputImage);
		duplicator->Update();
		ImageType::Pointer mask3D = duplicator->GetOutput();
		//And get empty mask3D
		for (int x = 0; x < size[0]; ++x) {
			for (int y = 0; y < size[1]; ++y) {
				for (int z = 0; z < size[2]; ++z) {
					ImageType::IndexType index;
					index[0] = x; index[1] = y; index[2] = z;
					mask3D->SetPixel(index, 0);
				}
			}
		}

		//Apply rigid transformation
		typedef itk::VersorRigid3DTransform< double > RigidTransformType;
		typedef itk::CenteredTransformInitializer<RigidTransformType, ImageType, ImageType>	TransformInitializerType;
		typedef itk::ImageFileReader<ImageType>	ImageFileReaderType;
		typedef itk::LinearInterpolateImageFunction<ImageType, double> LinearInterpolatorType;
		LinearInterpolatorType::Pointer	linearinterpolator = LinearInterpolatorType::New();
		typedef itk::NearestNeighborInterpolateImageFunction<ImageType, double> InterpolatorType;
		InterpolatorType::Pointer nninterpolator = InterpolatorType::New();


		//RigidTransformType::Pointer rigidTransform = RigidTransformType::New();
		//rigidTransform->SetIdentity();

		typedef itk::ResampleImageFilter< ImageType, ImageType > ResampleFilterType;


		//ImageType::Pointer auxMask = ImageType::New();
		//double angle = 0; 
		//For each projection, get maxmimum intensity projection 
		for (int proj = 0; proj < nProj; ++proj) {
			//cout<<proj<<endl;

			//Get an auxiliary empty volumetric mask 
			DuplicatorType::Pointer duplicator = DuplicatorType::New();
			duplicator->SetInputImage(mask3D);
			duplicator->Update();
			ImageType::Pointer auxMask = duplicator->GetOutput();
			// Get each MIP projection into a 3D volume
			ImageType::SizeType  size3dMip = inputMask->GetLargestPossibleRegion().GetSize();
			for (int x = 0; x < size3dMip[0]; ++x) {
				for (int z = 0; z < size3dMip[2]; ++z) {
					ImageType::IndexType indexMIP;
					indexMIP[0] = x; indexMIP[1] = proj; indexMIP[2] = z;
					for (int y = 0; y < size[1]; ++y) { //this size is from inputImage
						ImageType::IndexType index;
						index[0] = x; index[1] = y; index[2] = z; //percorre todos os y, fixando o x e z
						auxMask->SetPixel(index, inputMask->GetPixel(indexMIP));
					}
				}
			}

			//Now, rotate it to 0 degrees
			RigidTransformType::Pointer rigidTransform = RigidTransformType::New();
			rigidTransform->SetIdentity();

			TransformInitializerType::Pointer initializer = TransformInitializerType::New();
			initializer->SetTransform(rigidTransform);
			initializer->SetFixedImage(auxMask);
			initializer->SetMovingImage(auxMask);
			initializer->GeometryOn();
			//initializer->MomentsOn();
			initializer->InitializeTransform();

			typedef RigidTransformType::VersorType  VersorType;
			typedef VersorType::VectorType     VectorType;
			VersorType     rotation;
			VectorType     axis;
			axis[0] = 0.0; axis[1] = 0.0; axis[2] = 1.0;

			const double angle = -(-1.571 + proj * 3.142 / nProj); //angle corresponding to each projection
			//const double angle = -1.571 + proj * 3.142 / nProj;
			//const double angle = proj*3.142/nProj;
			rotation.Set(axis, angle);
			rigidTransform->SetRotation(rotation);
			ResampleFilterType::Pointer resample = ResampleFilterType::New();
			resample->SetInput(auxMask);
			resample->SetOutputOrigin(inputImage->GetOrigin());
			resample->SetSize(inputImage->GetLargestPossibleRegion().GetSize());
			resample->SetOutputSpacing(inputImage->GetSpacing());
			resample->SetOutputDirection(inputImage->GetDirection());
			resample->SetDefaultPixelValue(0);
			resample->SetInterpolator(nninterpolator);
			resample->SetTransform(rigidTransform);
			resample->Update();
			auxMask = resample->GetOutput();
			//ImageType::Pointer auxMask2 = resample->GetOutput();

			//sum auxMask to mask3D
			for (int x = 0; x < size[0]; ++x) {
				for (int y = 0; y < size[1]; ++y) {
					for (int z = 0; z < size[2]; ++z) {
						ImageType::IndexType index;
						index[0] = x; index[1] = y; index[2] = z;
						mask3D->SetPixel(index, mask3D->GetPixel(index) + auxMask->GetPixel(index));
					}
				}
			}

		}

	
		
		//Leave only labels with 15 intersections, and voxels with more than 1
		for (int x = 0; x < size[0]; ++x) {
			for (int y = 0; y < size[1]; ++y) {
				for (int z = 0; z < size[2]; ++z) {
					ImageType::IndexType index;
					index[0] = x; index[1] = y; index[2] = z;
					if (mask3D->GetPixel(index) >= threshold && inputImage->GetPixel(index)>1) mask3D->SetPixel(index, 1);
					//if (mask3D->GetPixel(index) >= threshold) mask3D->SetPixel(index, 1);
					else mask3D->SetPixel(index, 0);
				}
			}
		}


		
		typedef itk::ImageFileWriter<ImageType>	ImageFileWriterType;
		ImageFileWriterType::Pointer writer2 = ImageFileWriterType::New();
		mkdir("OutputMasks_3DSegmentationFromMIPs");
		char outNameBin2[1024];
		int lenBin1 = strlen(imageName);
		int pos1 = lenBin1 - 1;
		while (imageName[pos1] != '/' && pos1 > 0) --pos1;
		strcpy(outNameBin2, "OutputMasks_3DSegmentationFromMIPs/");
		int lenBin22 = strlen(outNameBin2);
		while (pos1 < lenBin1 - 7) {
			outNameBin2[lenBin22] = imageName[pos1];
			++pos1;
			++lenBin22;
		}
		outNameBin2[lenBin22] = '\0';

		strcat(outNameBin2, "_3DsegFromMIP.nrrd");

		writer2->SetFileName(outNameBin2);
		writer2->SetInput(mask3D);

		try {
			writer2->Update();
		}
		catch (itk::ExceptionObject& err) {
			std::cerr << "ExceptionObject caught !" << std::endl;
			std::cerr << err << std::endl;
			return 11;
		}
		
	}
	


	return 0;
	
}
