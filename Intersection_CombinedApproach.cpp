

#include <iostream>
#include <fstream>

#include <direct.h>
#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#include <string.h>

#include <vector>


#include "itkImage.h"
#include "itkPoint.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkImageDuplicator.h"

#include <stdio.h>
#include <iostream>

using namespace std;


const	unsigned int	spaceDimension = 3;
typedef float			PixelType;
typedef itk::Image< PixelType, spaceDimension >	ImageType;
typedef itk::Image< PixelType, 2 >	mipImageType;


using namespace std;



int main(){
	cout << " This program makes intersection between two masks. " << endl;
	cout << "  " << endl << endl;
	cout << " Claudia Constantino " << endl << endl;

	//interface with user
	char dirMasks1[1024];
	cout<<"Introduce the name of the folder where masks from 3D U-Net are:  ";
	cin>>dirMasks1;

	char dirMasks2[1024];
	cout<<"Introduce the name of the folder where masks from MIP 3D U-Net are: ";
	cin>>dirMasks2;
	

	vector<string> masks1NameList, masks2NameList, refMaskNameList;

	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	TCHAR szDir[MAX_PATH];
	size_t length_of_arg;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError=0;



	if(_chdir(dirMasks1) == -1){//Change to the specified folder, if it exists
		std::cerr <<endl<< "The directory "<<dirMasks1<<" does not exist!!"<<endl; //se a diretoria das imagens não existir
		return 1;
	}
	else _chdir("..");//return to base directory

	//Converter o nome da directoria de string para TCHAR
	std::string dirname(dirMasks1);
	TCHAR *DIRname = new TCHAR[dirname.size()+1]; //TCHAR stands for char (simple character of 1 byte); "Type + char"= TCHAR
	DIRname[dirname.size()]=0;
	std::copy(dirname.begin(),dirname.end(),DIRname);

	StringCchLength(DIRname, MAX_PATH, &length_of_arg);

	if (length_of_arg > (MAX_PATH - 3)){
		_tprintf(TEXT("\nDirectory path is too long.\n"));
	}
	
	_tprintf(TEXT("Target directory is: %s\n"), DIRname);
	 
	// Prepare string for use with FindFile functions.  First, copy the
	// string to a buffer, then append '\*' to the directory name.
	StringCchCopy(szDir, MAX_PATH, DIRname);
	StringCchCat(szDir, MAX_PATH, TEXT("\\*")); //data will be placed in the buffer, then placed in its final destination.
	 
	// Find the first file in the directory.
	hFind = FindFirstFile(szDir, &ffd);
	
	// List all the files in the directory with some info about them.
	int validFiles = 0;
	int nFiles = 0;

	//"do...while" loop checks its condition at the bottom of the loop.
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
			//Converter o nome dos ficheiros para string
			char name[1024];
			int n=0;
			do {
				name[n] = (char)ffd.cFileName[n];
				//cout<<ffd.cFileName[n]<<"  ";
				++n;
			} while((char) ffd.cFileName[n] != '\0');
			name[n] = '\0';

			int nameLength = strlen(name); //tipos de ficheiros válidos
			if((toupper(name[nameLength-3]) == 'D' && toupper(name[nameLength-2]) == 'C' && toupper(name[nameLength-1]) == 'M') ||
				(toupper(name[nameLength-3]) == 'N' && toupper(name[nameLength-2]) == 'I' && toupper(name[nameLength-1]) == 'I')||
				(toupper(name[nameLength-4]) == 'N' && toupper(name[nameLength-3]) == 'R' && toupper(name[nameLength-2]) == 'R' &&
				 toupper(name[nameLength-1]) == 'D' ||
				(toupper(name[nameLength - 6]) == 'N' && toupper(name[nameLength - 5]) == 'I' && toupper(name[nameLength - 4]) == 'I') &&
				 toupper(name[nameLength - 3]) == '.' && toupper(name[nameLength - 2]) == 'G' && toupper(name[nameLength - 1]) == 'Z')) {
				++ validFiles;
				//Convert the string to char
				char * url;
				url = new char[dirname.length() + 1];
				strcpy(url, dirname.c_str()); 
				char filename[1024];
				strcpy(filename, url); //Strcpy copies the character string url to the memory location pointed to by filename.
				strcat(filename, "/"); //Strcat appends a copy of the character "/" to the end of string filename
				strcat(filename, name);
				masks1NameList.push_back(filename); //criar uma lista com os ficheiros válidos
				delete[] url;
			}
		}
	}while (FindNextFile(hFind, &ffd) != 0); //enquanto existir um proximo ficheiro
	cout<<"Total files: "<<nFiles<<";  3D U-Net-based image files (.dcm, .nii and .nrrd): "<<validFiles<<endl;

	if(validFiles == 0){ //Caso não existam ficheiros válidos do tipo .dcm, .nii and .nrrd
		printf("\nNone valid file! Close the window.\n");
		char aux;
		cin>>aux; //cin = "character input" and it is defined in <iostream> header file.
		return 2;
	}

	//Para a diretoria das máscaras:
	int validFiles2 = 0; //validFiles2 é referente aos ficheiros das masks
	if(_chdir(dirMasks2) == -1){//Change to the specified folder, if it exists
		std::cerr <<endl<< "The directory "<<dirMasks2<<" does not exist!!"<<endl;
		cout<<"Do you want to do the segmentation in the full image (y/n)? "<<endl; //cout used to display the output to the monitor
		char option;
		cin>>option;
		if(option == 'n' || option == 'N') return 3;
	}
	else{
		_chdir("..");//return to base directory
		//Converter o nome da directoria de string para TCHAR
		std::string dirname2(dirMasks2);
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
				//Converter o nome dos ficheiros para string
				char name[1024];
				int n=0;
				do {
					name[n] = (char)ffd.cFileName[n];
					//cout<<ffd.cFileName[n]<<"  ";
					++n;
				} while((char) ffd.cFileName[n] != '\0');
				name[n] = '\0';

				int nameLength = strlen(name); //ficheiros das masks válidos: 
				if((toupper(name[nameLength-3]) == 'D' && toupper(name[nameLength-2]) == 'C' && toupper(name[nameLength-1]) == 'M') ||
						(toupper(name[nameLength-3]) == 'N' && toupper(name[nameLength-2]) == 'I' && toupper(name[nameLength-1]) == 'I') ||
						(toupper(name[nameLength-4]) == 'N' && toupper(name[nameLength-3]) == 'R' && toupper(name[nameLength-2]) == 'R' &&
						 toupper(name[nameLength-1]) == 'D') ||
						(toupper(name[nameLength - 6]) == 'N' && toupper(name[nameLength - 5]) == 'I' && toupper(name[nameLength - 4]) == 'I') &&
						 toupper(name[nameLength - 3]) == '.' && toupper(name[nameLength - 2]) == 'G' && toupper(name[nameLength - 1]) == 'Z'){
					++ validFiles2;		
					//Convert the string to char
					char * url;
					url = new char[dirname2.length() + 1];
					strcpy(url, dirname2.c_str()); 
					char filename[1024];
					strcpy(filename, url);
					strcat(filename, "/");
					strcat(filename, name);
					masks2NameList.push_back(filename); //cria lista para as mascaras validas
					delete[] url;
				}
			}
		}while (FindNextFile(hFind, &ffd) != 0);
		cout<<"Total files: "<<nFiles2<<";  MIP U-Net based-mask image files (.dcm, .nii and .nrrd): "<<validFiles2<<endl;
	}

	if(validFiles2 == 0){ //caso não existam maskFiles na diretoria 
		cout<<endl<<"!!There is no masks 2!!"<<endl;
		cout<<"Do you want ignore them and continue (y/n)? ";
		char option;
		cin>>option;
		if(option == 'n' || option == 'N') return 4;
	}
	else if(validFiles != validFiles2){ // se o nr de ficheiros de imagens válidos for diferentes do numero de mascaras válidas:
		cout<<endl<<"!!The number of masks is different from the number of images!!"<<endl;
		cout<<"Do you want ignore the masks and continue (y/n)? ";
		char option;
		cin>>option;
		if(option == 'n' || option == 'N') return 5;
	}

	std::sort(masks1NameList.begin(), masks1NameList.end());
	std::sort(masks2NameList.begin(), masks2NameList.end());

	//this is to help on the creation of images names
	stringstream stream;
	stream << validFiles;
	string str2;
	stream >> str2;
	const char* str3 = str2.c_str();
	int len = strlen(str3);

	for (int i = 0; i < validFiles; ++i) {

		printf("\n\n Mask based on 3D U-Net: %d Name: %s\n", i, masks1NameList[i].c_str());
		printf(" Mask based on MIP U-Net: %d Name: %s\n", i, masks2NameList[i].c_str());

		ImageType::Pointer inputMask1 = ImageType::New();
		ImageType::Pointer inputMask2 = ImageType::New();

		//Read segmentation mask 1
		char mask1Name[1024];
		strcpy(mask1Name, masks1NameList[i].c_str());
		typedef itk::ImageFileReader<ImageType>	ImageReaderType;
		ImageReaderType::Pointer reader1 = ImageReaderType::New();
		reader1->SetFileName(mask1Name);
		try {
			reader1->Update();
		}
		catch (itk::ExceptionObject& err) {
			std::cerr << "ExceptionObject caught !" << std::endl;
			std::cerr << err << std::endl;
			return 6;
		}
		inputMask1 = reader1->GetOutput(); 


		//Read segmentation mask 2
		char mask2Name[1024];
		strcpy(mask2Name, masks2NameList[i].c_str());
		ImageReaderType::Pointer reader2 = ImageReaderType::New();
		reader2->SetFileName(mask2Name);
		try {
			reader2->Update();
		}
		catch (itk::ExceptionObject& err) {
			std::cerr << "ExceptionObject caught !" << std::endl;
			std::cerr << err << std::endl;
			return 7;
		}
		inputMask2 = reader2->GetOutput(); 

		//Separate mask 1 into connected lesions
		//MASK 1
		typedef itk::Image< unsigned short, spaceDimension> MaskImageType;
		typedef itk::ConnectedComponentImageFilter <ImageType, MaskImageType> ConnectedComponentImageFilterType;
		ConnectedComponentImageFilterType::Pointer connected = ConnectedComponentImageFilterType::New();
		connected->SetInput(inputMask1);
		connected->FullyConnectedOn();
		connected->FullyConnectedOn();
		connected->Update();
		MaskImageType::Pointer connectedMask = connected->GetOutput();

		//Get number of lesions segmented in Mask 1
		ImageType::SizeType size1 = inputMask1->GetLargestPossibleRegion().GetSize();
		int maxVoxelMaskCon1 = 0;
		for (int x = 0; x < size1[0]; ++x) { 
			for (int y = 0; y < size1[1]; ++y) {
				for (int z = 0; z < size1[2]; ++z) {
					MaskImageType::IndexType index;
					index[0] = x; index[1] = y; index[2] = z;
					if (connectedMask->GetPixel(index) > maxVoxelMaskCon1) maxVoxelMaskCon1 = connectedMask->GetPixel(index);
				}
			}
		}

		//copy connectedMask, which has MaskImageType, to inputMask1 with ImageType
		for (int x = 0; x < size1[0]; ++x) {
			for (int y = 0; y < size1[1]; ++y) {
				for (int z = 0; z < size1[2]; ++z) {
					ImageType::IndexType index;
					index[0] = x; index[1] = y; index[2] = z;
					inputMask1->SetPixel(index, connectedMask->GetPixel(index));
				}
			}
		}



		//Separate mask 2 into connected lesions
		//MASK 2
		ConnectedComponentImageFilterType::Pointer connected2 = ConnectedComponentImageFilterType::New();
		connected2->SetInput(inputMask2);
		connected2->FullyConnectedOn();
		connected2->FullyConnectedOn();
		connected2->Update();
		MaskImageType::Pointer connectedMask2 = connected2->GetOutput();

		//Get number of lesions segmented in Mask 2
		int maxVoxelMaskCon2 = 0;
		for (int x = 0; x < size1[0]; ++x) {
			for (int y = 0; y < size1[1]; ++y) {
				for (int z = 0; z < size1[2]; ++z) {
					MaskImageType::IndexType index;
					index[0] = x; index[1] = y; index[2] = z;
					if (connectedMask2->GetPixel(index) > maxVoxelMaskCon2) maxVoxelMaskCon2 = connectedMask->GetPixel(index);
				}
			}
		}

		//copy connectedMask, which has MaskImageType, to inputMask1 with ImageType
		for (int x = 0; x < size1[0]; ++x) {
			for (int y = 0; y < size1[1]; ++y) {
				for (int z = 0; z < size1[2]; ++z) {
					ImageType::IndexType index;
					index[0] = x; index[1] = y; index[2] = z;
					inputMask2->SetPixel(index, connectedMask2->GetPixel(index));
				}
			}
		}


		//create new mask to save intersections
		typedef itk::ImageDuplicator<ImageType> DuplicatorType;
		DuplicatorType::Pointer duplicator = DuplicatorType::New();
		duplicator->SetInputImage(inputMask1);
		duplicator->Update();
		ImageType::Pointer mask = duplicator->GetOutput();
		//And get empty mask3D
		for (int x = 0; x < size1[0]; ++x) {
			for (int y = 0; y < size1[1]; ++y) {
				for (int z = 0; z < size1[2]; ++z) {
					ImageType::IndexType index;
					index[0] = x; index[1] = y; index[2] = z;
					mask->SetPixel(index, 0);
				}
			}
		}

		int aux = 0;
		for (int label = 1; label <= maxVoxelMaskCon1; ++label) {
		//intersection between input masks 
			for (int x = 0; x < size1[0]; ++x) {
				for (int y = 0; y < size1[1]; ++y) {
					for (int z = 0; z < size1[2]; ++z) {
						ImageType::IndexType index;
						index[0] = x; index[1] = y; index[2] = z;

						if (inputMask1->GetPixel(index) == label && inputMask2->GetPixel(index) >= 1 && inputMask1->GetPixel(index) != aux) { // with this last condition, no repetition is done

							//int i = inputMask2->GetPixel(index);
							for (int a = 0; a < size1[0]; ++a) {
								for (int b = 0; b < size1[1]; ++b) {
									for (int c = 0; c < size1[2]; ++c) {
										ImageType::IndexType index2;
										index2[0] = a; index2[1] = b; index2[2] = c;
										if (inputMask1->GetPixel(index2) == label) mask->SetPixel(index2, 1);
									}
								}
							}
							aux = label;
						}

					}
				}
			}

		}

		//To maintain labels that are in label 2 and not in lab 1
		int aux2 = 0;
		for (int label = 1; label <= maxVoxelMaskCon2; ++label) {
			//intersection between input masks 
			for (int x = 0; x < size1[0]; ++x) {
				for (int y = 0; y < size1[1]; ++y) {
					for (int z = 0; z < size1[2]; ++z) {
						ImageType::IndexType index;
						index[0] = x; index[1] = y; index[2] = z;

						if (inputMask2->GetPixel(index) == label && inputMask1->GetPixel(index) >= 1 && inputMask2->GetPixel(index) != aux2) { // with this last condition, no repetition is done

							//int i = inputMask2->GetPixel(index);
							for (int a = 0; a < size1[0]; ++a) {
								for (int b = 0; b < size1[1]; ++b) {
									for (int c = 0; c < size1[2]; ++c) {
										ImageType::IndexType index2;
										index2[0] = a; index2[1] = b; index2[2] = c;
										if (inputMask2->GetPixel(index2) == label) inputMask2->SetPixel(index2, 0);
									}
								}
							}
							aux2 = label;
						}

					}
				}
			}

		}
		//Join inputMask2 (clean now), with mask
		for (int x = 0; x < size1[0]; ++x) {
			for (int y = 0; y < size1[1]; ++y) {
				for (int z = 0; z < size1[2]; ++z) {
					ImageType::IndexType index;
					index[0] = x; index[1] = y; index[2] = z;
					if (inputMask2->GetPixel(index) >= 1) mask->SetPixel(index, 1);
				}
			}
		}

		

		typedef itk::ImageFileWriter<ImageType>	ImageFileWriterType;
		ImageFileWriterType::Pointer writer2 = ImageFileWriterType::New();
		mkdir("OutputMasks_intersection&union");
		char outNameBin2[1024];
		int lenBin1 = strlen(mask1Name);
		int pos1 = lenBin1 - 1;
		while (mask1Name[pos1] != '/' && pos1 > 0) --pos1;
		strcpy(outNameBin2, "OutputMasks_intersection&union/");
		int lenBin22 = strlen(outNameBin2);
		while (pos1 < lenBin1 - 7) {
			outNameBin2[lenBin22] = mask1Name[pos1];
			++pos1;
			++lenBin22;
		}
		outNameBin2[lenBin22] = '\0';

		strcat(outNameBin2, "_intersect.nrrd");

		writer2->SetFileName(outNameBin2);
		writer2->SetInput(mask);

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
