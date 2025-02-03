# PETsegmentation-MIPandDL
This repository contains the code and models to obtain fully automatic segmentation of lesions avid for [18F]FDG and [68Ga]Ga-PSMA in PET/CT using MIPs and DL.

This is a step-by-step guide to achieve standard-DL-based segmentation, 3D-MIP-DL-based segmentation and further results with a combined approach (standard-DL+MIP-DL-based). 

## Standard-DL-based segmentation
To obtain the standard-DL-based segmentation, that was based on nnU-Net, follow the next steps. 
Please, be aware to use the model specific for your task, or [18F]FDG PET/CT or [68Ga]Ga-PSMA lesions segmentation. 

### 1. Install the required libraries : 
You will first need and assure that the [nnU-Net required libraries](https://github.com/MIC-DKFZ/nnUNet) and configure required paths.

### 2. Download weights : 
- Weights from the [18F]FDG model can be downloaded at the following [link](https://drive.google.com/drive/u/0/folders/1T02D1G6igx9eaFcgxzG0y0QBdp7Qg2gF).
- Weights from the [68Ga]Ga-PSMA model can be downloaded at the following [link]

### 3. Inference :
For [18F]FDG PET/CT scans, only PET (one-channel) should be provided, since since the inclusion of low-dose CT as a second channel does not improve significantly the segmentation results (Constantino et al, In: Annual Congress of the European Association of Nuclear Medicine, 2023).
In the PATH_FOLDER patient should be numbered as "patientID_0000" with PET the nifti format and in SUV units.

For [68Ga]Ga-PSMA scans, two-channel has to be provided (PET plus low-dose CT). 
In the PATH_FOLDER every patients should be numbered as patientID_0000 for CT and patientID_0001 for PET in the nifti format where CT should be in hounsfield unit and PET in SUV.

Then, from the terminal type:
nnUNet_predict -i PATH_FOLDER -o ./PATH_OUTPUT -t 1


## 3D-MIP-based DL segmentation 
To obtain the 3D-MIP-based DL segmentation, follow the next steps for 3D-MIP generation, 3D-MIP segmentation, and post-processing reconstruction to the PET space.
One again, be aware to use the model specific for your task, or [18F]FDG PET/CT or [68Ga]Ga-PSMA lesions segmentation. 

### 1. 3D-MIP generation : 
A c++ code was used for this step. The respective code is in the file "    ". 

### 2. 3D-MIP segmentation : 
3D-MIP DL segmentation also uses nnU-Net framework. Follow similar steps as explained above in "Standard-DL-based segmentation" section. 
Instead, load the weights related with FDG 3D-MIP model (download:     ) or PSMA 3D-MIP model (download:      ). 

### 3. Post-processing reconstruction  :
The post-processing reconstruction code that uses the 3D-MIP segmentation mask and reconstructs it to the 3D PET space is in the file "        ".


## Standard-DL + MIP-DL-based segmentation (combined approach)
To incorporate the advantages of MIP images to easily identify regions of higher metabolic uptake, a pipeline combining results from standard-DL-based and MIP-DL-based segmentations was built. The code is in the file " ". 



