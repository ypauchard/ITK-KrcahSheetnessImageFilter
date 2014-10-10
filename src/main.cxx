// ITK
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"

// sheetness
#include "KrcahSheetnessFeatureGenerator.h"

// exclude regions
#include "itkBinaryThresholdImageFilter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkLabelShapeKeepNObjectsImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkBinaryFunctorImageFilter.h"
#include "KrcahBackgroundFunctor.h"


// pixel / image type
const unsigned int IMAGE_DIMENSION = 3;
typedef short InputPixelType;
typedef float SheetnessPixelType;
typedef unsigned char MaskPixelType;
typedef itk::Image<InputPixelType, IMAGE_DIMENSION> InputImageType;
typedef itk::Image<SheetnessPixelType, IMAGE_DIMENSION> SheetnessImageType;
typedef itk::Image<MaskPixelType, IMAGE_DIMENSION> MaskImageType;
typedef itk::ImageFileReader<InputImageType> FileReaderType;
typedef itk::ImageFileWriter<SheetnessImageType> FileWriterType;
typedef itk::ImageFileWriter<MaskImageType> MaskWriterType;

// sheetness
typedef itk::KrcahSheetnessFeatureGenerator<InputImageType, SheetnessImageType> KrcahSheetnessFeatureGenerator;

// not bone region
typedef unsigned long LabelPixelType;
typedef itk::Image<LabelPixelType, IMAGE_DIMENSION> LabelImageType;
typedef itk::BinaryThresholdImageFilter<InputImageType, LabelImageType> BinaryThresholdFilterType;
typedef itk::ConnectedComponentImageFilter<LabelImageType, LabelImageType> ConnectedComponentFilterType;
typedef itk::LabelShapeKeepNObjectsImageFilter<LabelImageType> KeepNObjectsFilterType;
typedef itk::CastImageFilter<LabelImageType, MaskImageType> LabelToMaskCastFilter;

// not background region
typedef itk::Functor::KrcahBackground<InputPixelType, SheetnessPixelType, SheetnessPixelType> FunctorType;
typedef itk::BinaryFunctorImageFilter<InputImageType, SheetnessImageType, MaskImageType, FunctorType> BinaryFunctorFilterType;

// functions
FileReaderType::Pointer readImage(char *pathInput);

SheetnessImageType::Pointer getSheetnessImage(InputImageType::Pointer input);

MaskImageType::Pointer getExclusionRegionNotBone(InputImageType::Pointer input);

MaskImageType::Pointer getExclusionRegionNotBkg(InputImageType::Pointer input, SheetnessImageType::Pointer sheetness);

// expected CLI call:
// ./KrcahSheetness /path/to/input /path/to/outputSheetness /path/to/outputExclusionNotBone /path/to/outputExclusionNotBackground
int main(int argc, char *argv[]) {
    // Verify arguments
    if (argc != 5) {
        std::cerr << "Required: inputImage.mhd outputSheetness.mhd outputBackground.mhd outputForeground.mhd" << std::endl;
        std::cerr << "inputImage.mhd:        3D image in Hounsfield Units -1024 to 3071" << std::endl;
        std::cerr << "outputSheetness.mhd:   3D image sheetness results." << std::endl;
        std::cerr << "                       Pixeltype float, ranging from -1 to 1." << std::endl;
        std::cerr << "outputBackground.mhd:  3D image background mask." << std::endl;
        std::cerr << "                       Pixeltype unsigned char, pixel value 1 indicating background." << std::endl;
        std::cerr << "outputForeground.mhd:  3D image foreground mask" << std::endl;
        std::cerr << "                       Pixeltype unsigned char, pixel value 1 indicating foreground." << std::endl;
        return EXIT_FAILURE;
    }

    // read input
    FileReaderType::Pointer inputReader = readImage(argv[1]);
    InputImageType::Pointer inputImage = inputReader->GetOutput();

    // generate sheetness
    std::cout << "generating sheetness..." << std::endl;
    SheetnessImageType::Pointer sheetnessImage = getSheetnessImage(inputImage);

    // first exclude region 'not bone'
    std::cout << "generating exlusion regions" << std::endl;
    MaskImageType::Pointer exclusionNotBone = getExclusionRegionNotBone(inputImage);

    FileReaderType::Pointer inputReader2 = readImage(argv[1]);
    InputImageType::Pointer inputImage2 = inputReader2->GetOutput();

//    itk::MultiThreader::SetGlobalDefaultNumberOfThreads(1);
    MaskImageType::Pointer exclusionNotBkg = getExclusionRegionNotBkg(inputImage2, sheetnessImage);

    // write output
    std::cout << "writing sheetness to file..." << std::endl;
    FileWriterType::Pointer writer = FileWriterType::New();
    writer->SetFileName(argv[2]);
    writer->SetInput(sheetnessImage);
    writer->Update();

    std::cout << "writing exclusion regions" << std::endl;
    MaskWriterType::Pointer writerExcludeNotBone = MaskWriterType::New();
    writerExcludeNotBone->SetFileName(argv[3]);
    writerExcludeNotBone->SetInput(exclusionNotBone);
    writerExcludeNotBone->Update();

    MaskWriterType::Pointer writerExcludeNotBkg = MaskWriterType::New();
    writerExcludeNotBkg->SetFileName(argv[4]);
    writerExcludeNotBkg->SetInput(exclusionNotBkg);
    writerExcludeNotBkg->Update();

    return EXIT_SUCCESS;
}

MaskImageType::Pointer getExclusionRegionNotBkg(InputImageType::Pointer input, SheetnessImageType::Pointer sheetness) {
    BinaryFunctorFilterType::Pointer notBackgroundRegionFilter = BinaryFunctorFilterType::New();
    notBackgroundRegionFilter->SetInput1(input);
    notBackgroundRegionFilter->SetInput2(sheetness);

    notBackgroundRegionFilter->Update();
    return notBackgroundRegionFilter->GetOutput();
}

MaskImageType::Pointer getExclusionRegionNotBone(InputImageType::Pointer input) {
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
    LabelToMaskCastFilter::Pointer castFilter = LabelToMaskCastFilter::New();
    castFilter->SetInput(keepNObjectsFilter->GetOutput());

    castFilter->Update();
    return castFilter->GetOutput();
}

SheetnessImageType::Pointer getSheetnessImage(InputImageType::Pointer input) {
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