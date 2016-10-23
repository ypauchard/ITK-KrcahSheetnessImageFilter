/**
 *  Image GraphCut 3D Segmentation
 *
 *  Copyright (c) 2016, Zurich University of Applied Sciences, School of Engineering, T. Fitze, Y. Pauchard
 *
 *  Licensed under GNU General Public License 3.0 or later.
 *  Some rights reserved.
 */

 #include "ImageGraphCut3DFilter.h"


#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkBinaryBallStructuringElement.h"

//global typedefs
typedef itk::Image<unsigned char, 3> ImageType;

//function
void writeImage(ImageType::Pointer image, std::string path);

/** This example picks the largest component of the input mask and tries to split it using erosion and graphcut.
*/
int main(int argc, char *argv[]) {
    // Verify arguments
    if (argc != 4) {
        std::cerr << "Required: image.mhd output.mhd radius" << std::endl;
        std::cerr << "image.mhd:           3D binary image mask, background is 0" << std::endl;
        std::cerr << "output.mhd:          3D binary image mask of largest split object" << std::endl;
        std::cerr << "radius               Radius of sphere for 3D binary erosion" << std::endl;
        return EXIT_FAILURE;
    }

    // Parse arguments
    std::string imageFilename = argv[1];
    std::string outputFilename = argv[2];
    int radius = atoi(argv[3]);               // Erosion sphere radius

    // Print arguments
    std::cout << "imageFilename: " << imageFilename << std::endl
            << "outputFilename: " << outputFilename << std::endl
            << "radius: " << radius << std::endl;

    // Define all image types


    typedef itk::ConnectedComponentImageFilter <ImageType, ImageType > ConnectedComponentImageFilterType;
    typedef itk::RelabelComponentImageFilter <ImageType, ImageType> RelabelComponentImageFilterType;
    typedef itk::BinaryThresholdImageFilter<ImageType, ImageType> TresholdFilterType;

    // Read the image
    std::cout << "*** Reading image ***" << std::endl;
    typedef itk::ImageFileReader<ImageType> ReaderType;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(imageFilename);

    ImageType::Pointer input = reader->GetOutput();

    // Perform erosion
    std::cout << "*** Performing Erosion ***" << std::endl;
    typedef itk::BinaryBallStructuringElement<
            ImageType::PixelType, 3>                  StructuringElementType;
    StructuringElementType structuringElement;
    structuringElement.SetRadius(radius);
    structuringElement.CreateStructuringElement();

    typedef itk::BinaryErodeImageFilter <ImageType, ImageType, StructuringElementType>
            BinaryErodeImageFilterType;

    BinaryErodeImageFilterType::Pointer erodeFilter
            = BinaryErodeImageFilterType::New();
    erodeFilter->SetInput(input);
    erodeFilter->SetKernel(structuringElement);
    erodeFilter->SetErodeValue(1);
    erodeFilter->Update();

    writeImage(erodeFilter->GetOutput(),"debug_erode.nrrd");

    // If as a result we have more than one component
    ConnectedComponentImageFilterType::Pointer connectedComponentFilter = ConnectedComponentImageFilterType::New();
    connectedComponentFilter->SetInput(erodeFilter->GetOutput());
    connectedComponentFilter->Update();
    if(connectedComponentFilter->GetObjectCount() > 1) {
        std::cout << "*** After Erosion, "<<connectedComponentFilter->GetObjectCount()<<" objects found. Picking largest ***" << std::endl;
        // --> pick largest as foreground, rest as background
        RelabelComponentImageFilterType::Pointer relabelComponentImageFilter = RelabelComponentImageFilterType::New();
        relabelComponentImageFilter->SetInput(connectedComponentFilter->GetOutput());
        auto fgThresholdFilter = TresholdFilterType::New();
        fgThresholdFilter->SetInput(relabelComponentImageFilter->GetOutput());
        fgThresholdFilter->SetLowerThreshold(1);
        fgThresholdFilter->SetUpperThreshold(1);
        fgThresholdFilter->SetInsideValue(1);
        fgThresholdFilter->SetOutsideValue(0);
        fgThresholdFilter->Update();

        writeImage(fgThresholdFilter->GetOutput(),"debug_fg.nrrd");

        auto bgThresholdFilter = TresholdFilterType::New();
        bgThresholdFilter->SetInput(relabelComponentImageFilter->GetOutput());
        bgThresholdFilter->SetLowerThreshold(2);
        bgThresholdFilter->SetUpperThreshold(255);
        bgThresholdFilter->SetInsideValue(1);
        bgThresholdFilter->SetOutsideValue(0);
        bgThresholdFilter->Update();

        writeImage(bgThresholdFilter->GetOutput(),"debug_bg.nrrd");

        // --> Set up the graph cut
        typedef itk::ImageGraphCut3DFilter<ImageType, ImageType, ImageType, ImageType> GraphCutFilterType;
        GraphCutFilterType::Pointer graphCutFilter = GraphCutFilterType::New();
        graphCutFilter->SetInputImage(input);
        graphCutFilter->SetForegroundImage(fgThresholdFilter->GetOutput());
        graphCutFilter->SetBackgroundImage(bgThresholdFilter->GetOutput());

        // --> Set graph cut parameters
        graphCutFilter->SetVerboseOutput(true);
        graphCutFilter->SetBoundaryDirectionTypeToBrightDark();
        graphCutFilter->SetLambda(1.0);
        graphCutFilter->SetSigma(0.001); //something small
        graphCutFilter->SetTerminalWeight(std::numeric_limits<float>::max());

        // --> Define the color values of the output
        graphCutFilter->SetForegroundPixelValue(1);
        graphCutFilter->SetBackgroundPixelValue(0);

        // --> Start the computation
        std::cout << "*** Performing Graph Cut ***" << std::endl;
        graphCutFilter->Update();

        // get largest component - just in case there are small disjoint parts creeping in...
        std::cout << "*** Writing Result ***" << std::endl;
        connectedComponentFilter->SetInput(graphCutFilter->GetOutput());
        connectedComponentFilter->Update();
        if(connectedComponentFilter->GetObjectCount() > 1) {
            std::cout << "*** After graphcut " << connectedComponentFilter->GetObjectCount() <<
            " objects found, getting largest ***" << std::endl;
        }
        relabelComponentImageFilter->SetInput(connectedComponentFilter->GetOutput());
        auto thresholdFilter = TresholdFilterType::New();
        thresholdFilter->SetInput(relabelComponentImageFilter->GetOutput());
        thresholdFilter->SetLowerThreshold(1);
        thresholdFilter->SetUpperThreshold(1);
        thresholdFilter->SetInsideValue(1);
        thresholdFilter->SetOutsideValue(0);
        thresholdFilter->Update();

        writeImage(thresholdFilter->GetOutput(),outputFilename);


    } else {
        // else: Nothing needs to be done.
        std::cout << "*** After Erosion, only one object found - no output produced.***" << std::endl;
    }



}


void writeImage(ImageType::Pointer image, std::string path){
    typedef itk::ImageFileWriter<ImageType> WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName(path);
    writer->SetInput(image);
    try {
        std::cout << "Writing output image " << path << std::endl;
        writer->Update();
    }
    catch (itk::ExceptionObject &err) {
        std::cerr << "ERROR: Exception caught while writing output image" << std::endl;
        std::cerr << err << std::endl;
    }

}