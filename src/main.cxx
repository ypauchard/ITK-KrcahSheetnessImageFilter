#include <ctime>


// ITK
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"
#include "itkImageSpatialObject.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkAddImageFilter.h"
#include "itkHessianRecursiveGaussianImageFilter.h"
#include "itkSymmetricEigenAnalysisImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkScalarConnectedComponentImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
#include "itkStatisticsImageFilter.h"

#include "MaximumAbsoluteValueImageFilter.h"
#include "KrcahSheetnessImageFilter.h"
#include "TraceImageFilter.h"
#include "KrcahSheetnessFeatureGenerator.h"

// pixel / image type
const unsigned int IMAGE_DIMENSION = 3;
typedef short InputPixelType;
typedef float OutputPixelType;
typedef itk::Image<InputPixelType, IMAGE_DIMENSION> InputImageType;
typedef itk::Image<OutputPixelType, IMAGE_DIMENSION> OutputImageType;
typedef itk::ImageFileReader<InputImageType> FileReaderType;
typedef itk::ImageFileWriter<OutputImageType> FileWriterType;

typedef itk::KrcahSheetnessFeatureGenerator<InputImageType, OutputImageType> KrcahSheetnessFeatureGenerator;

// femur connected components
typedef itk::Image<unsigned long, IMAGE_DIMENSION> LabelImageType;
typedef itk::ScalarConnectedComponentImageFilter<OutputImageType, LabelImageType> ConnectedComponentImageFilterType;
typedef itk::RelabelComponentImageFilter<LabelImageType, LabelImageType> RelabelFilterType;
typedef itk::BinaryThresholdImageFilter<LabelImageType, LabelImageType> BinaryThresholdFilterType;
typedef itk::RescaleIntensityImageFilter<LabelImageType, OutputImageType> BinaryRescaleFilterType;


// functions
FileReaderType::Pointer readImage(char *pathInput);
OutputImageType::Pointer getSheetnessImage(InputImageType::Pointer input);
OutputImageType::Pointer extractFemur(OutputImageType::Pointer input);

// expected CLI call:
// ./Testbench /path/to/input /path/to/output
int main(int argc, char *argv[]) {
    // read input
    FileReaderType::Pointer inputReader = readImage(argv[1]);
    InputImageType::Pointer inputImage = inputReader->GetOutput();

    // generate sheetness
    std::cout << "generating sheetness..." << std::endl;
    OutputImageType::Pointer sheetnessImage = getSheetnessImage(inputImage);

    // write output
    std::cout << "writing sheetness to file..." << std::endl;
    FileWriterType::Pointer writer = FileWriterType::New();
    writer->SetFileName(argv[2]);
    writer->SetInput(sheetnessImage);
    writer->Update();

    return EXIT_SUCCESS;
}

OutputImageType::Pointer getSheetnessImage(InputImageType::Pointer input) {
    KrcahSheetnessFeatureGenerator::Pointer generator = KrcahSheetnessFeatureGenerator::New();
    generator->SetInput(input);
    generator->Update();
    return generator->GetOutput();
}

OutputImageType::Pointer extractFemur(OutputImageType::Pointer input) {

    std::cout << "CPU time (in s):" << std::endl;
    std::clock_t begin = std::clock();
    std::clock_t beginTotal = std::clock();

    // connected components
    ConnectedComponentImageFilterType::Pointer m_ConnectedCompontentFilter = ConnectedComponentImageFilterType::New();
    m_ConnectedCompontentFilter->SetDistanceThreshold(0.10);
    m_ConnectedCompontentFilter->SetInput(input);
    m_ConnectedCompontentFilter->Update();

    std::clock_t end = std::clock();
    double elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "connected components: " << elapsedSecs << std::endl;
    begin = std::clock();

    // filter by size
    RelabelFilterType::Pointer m_RelabelFilter = RelabelFilterType::New();
    m_RelabelFilter->SetMinimumObjectSize(1500);
    m_RelabelFilter->SetInput(m_ConnectedCompontentFilter->GetOutput());
    m_RelabelFilter->Update();

    end = std::clock();
    elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "relabel: " << elapsedSecs << std::endl;
    begin = std::clock();

    // threshold
    BinaryThresholdFilterType::Pointer m_ThresholdFilter = BinaryThresholdFilterType::New();
    m_ThresholdFilter->SetLowerThreshold(4);
    m_ThresholdFilter->SetInput(m_RelabelFilter->GetOutput());
    m_ThresholdFilter->Update();

    end = std::clock();
    elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "treshold: " << elapsedSecs << std::endl;
    begin = std::clock();

    // rescale to range 0.0 - 1.0
    BinaryRescaleFilterType::Pointer m_RescaleFilter = BinaryRescaleFilterType::New();
    m_RescaleFilter->SetOutputMinimum(0.0);
    m_RescaleFilter->SetOutputMaximum(1.0);
    m_RescaleFilter->SetInput(m_ThresholdFilter->GetOutput());
    m_RescaleFilter->Update();

    end = std::clock();
    elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "rescale: " << elapsedSecs << std::endl;
    begin = std::clock();

    std::clock_t endTotal = std::clock();
    double elapsedSecsTotal = double(endTotal - beginTotal) / CLOCKS_PER_SEC;
    std::cout << "------------" << std::endl;
    std::cout << "Total: " << elapsedSecsTotal << std::endl;
    std::cout << std::endl;

    return m_RescaleFilter->GetOutput();
}

FileReaderType::Pointer readImage(char *path) {
    FileReaderType::Pointer reader = FileReaderType::New();
    reader->SetFileName(path);
    return reader;
}