#include <ctime>


// ITK
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"

// not bone region
#include "itkBinaryThresholdImageFilter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkLabelShapeKeepNObjectsImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
#include "itkCastImageFilter.h"

#include "KrcahSheetnessFeatureGenerator.h"

// pixel / image type
const unsigned int IMAGE_DIMENSION = 3;
typedef short InputPixelType;
typedef unsigned long LabelPixelType;
typedef float OutputPixelType;
typedef itk::Image<InputPixelType, IMAGE_DIMENSION> InputImageType;
typedef itk::Image<LabelPixelType, IMAGE_DIMENSION> LabelImageType;
typedef itk::Image<OutputPixelType, IMAGE_DIMENSION> OutputImageType;
typedef itk::ImageFileReader<InputImageType> FileReaderType;
typedef itk::ImageFileWriter<OutputImageType> FileWriterType;

// sheetness
typedef itk::KrcahSheetnessFeatureGenerator<InputImageType, OutputImageType> KrcahSheetnessFeatureGenerator;

// not bone region
typedef itk::BinaryThresholdImageFilter<InputImageType, LabelImageType> BinaryThresholdFilterType;
typedef itk::ConnectedComponentImageFilter<LabelImageType, LabelImageType> ConnectedComponentFilterType;
typedef itk::LabelShapeKeepNObjectsImageFilter<LabelImageType> KeepNObjectsFilterType;
typedef itk::CastImageFilter<LabelImageType, OutputImageType> CastFilterType;


// functions
FileReaderType::Pointer readImage(char *pathInput);
OutputImageType::Pointer getSheetnessImage(InputImageType::Pointer input);

OutputImageType::Pointer getExclusionRegionNotBone(InputImageType::Pointer input);
OutputImageType::Pointer extractFemur(OutputImageType::Pointer input);

// expected CLI call:
// ./Testbench /path/to/input /path/to/outputSheetness /path/to/outputExclusionNotBone
int main(int argc, char *argv[]) {
    // read input
    FileReaderType::Pointer inputReader = readImage(argv[1]);
    InputImageType::Pointer inputImage = inputReader->GetOutput();

    // generate sheetness
    std::cout << "generating sheetness..." << std::endl;
    OutputImageType::Pointer sheetnessImage = getSheetnessImage(inputImage);

    // threshold <50HU
    std::cout << "generating exlusion region 'not bone'..." << std::endl;
    OutputImageType::Pointer exclusionNotBone = getExclusionRegionNotBone(inputImage);

    // write output
    std::cout << "writing sheetness to file..." << std::endl;
    FileWriterType::Pointer writer = FileWriterType::New();
    writer->SetFileName(argv[2]);
    writer->SetInput(sheetnessImage);
    writer->Update();

    std::cout << "writing exclusion not bone to file..." << std::endl;
    FileWriterType::Pointer writerExcludeNotBone = FileWriterType::New();
    writerExcludeNotBone->SetFileName(argv[3]);
    writerExcludeNotBone->SetInput(exclusionNotBone);
    writerExcludeNotBone->Update();

    return EXIT_SUCCESS;
}

OutputImageType::Pointer getExclusionRegionNotBone(InputImageType::Pointer input) {
    // threshold
    BinaryThresholdFilterType::Pointer thresholdFilter = BinaryThresholdFilterType::New();
    thresholdFilter->SetUpperThreshold(-50);
    thresholdFilter->SetInsideValue(255); // ConnectedComponentImageFilter assumes 0 is background
    thresholdFilter->SetOutsideValue(0);
    thresholdFilter->SetInput(input);

    // find connected components
    ConnectedComponentFilterType::Pointer connectedComponentFilter = ConnectedComponentFilterType::New();
    connectedComponentFilter->SetInput(thresholdFilter->GetOutput());

    // extract largest connected component
    KeepNObjectsFilterType::Pointer keepNObjectsFilter = KeepNObjectsFilterType::New();
    keepNObjectsFilter->SetBackgroundValue(0);
    keepNObjectsFilter->SetNumberOfObjects(1);
    keepNObjectsFilter->SetAttribute(KeepNObjectsFilterType::LabelObjectType::NUMBER_OF_PIXELS);
    keepNObjectsFilter->SetInput(connectedComponentFilter->GetOutput());

    // cast to output. pixel value for region = 1, 'background' = 0
    CastFilterType::Pointer castFilter = CastFilterType::New();
    castFilter->SetInput(keepNObjectsFilter->GetOutput());

    castFilter->Update();
    return castFilter->GetOutput();
}

OutputImageType::Pointer getSheetnessImage(InputImageType::Pointer input) {
    KrcahSheetnessFeatureGenerator::Pointer generator = KrcahSheetnessFeatureGenerator::New();
    generator->SetInput(input);
    generator->Update();
    return generator->GetOutput();
}

FileReaderType::Pointer readImage(char *path) {
    FileReaderType::Pointer reader = FileReaderType::New();
    reader->SetFileName(path);
    return reader;
}