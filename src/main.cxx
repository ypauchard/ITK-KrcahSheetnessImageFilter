#include <ctime>


// ITK
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageSpatialObject.h"
#include "itkDiscreteGaussianImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkAddImageFilter.h"
#include "itkHessianRecursiveGaussianImageFilter.h"
#include "itkSymmetricEigenAnalysisImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkScalarConnectedComponentImageFilter.h"
#include "itkRelabelComponentImageFilter.h"
#include "itkStatisticsImageFilter.h"

#include "MaximumAbsoluteValueImageFilter.h"
#include "KrcahSheetnessImageFilter.h"
#include "TraceImageFilter.h"

// pixel / image type
const unsigned int IMAGE_DIMENSION = 3;
typedef float InputPixelType;
typedef float InternalPixelType;
typedef float OutputPixelType;
typedef itk::Image<InputPixelType, IMAGE_DIMENSION> InputImageType;
typedef itk::Image<InternalPixelType, IMAGE_DIMENSION> InternalImageType;
typedef itk::Image<OutputPixelType, IMAGE_DIMENSION> OutputImageType;
typedef itk::ImageFileReader<InputImageType> FileReaderType;
typedef itk::ImageFileWriter<OutputImageType> FileWriterType;

// input processing
typedef itk::CurvatureAnisotropicDiffusionImageFilter<InputImageType, InternalImageType> AnisotropicDiffusionFilterType;
typedef itk::DiscreteGaussianImageFilter<InputImageType, InternalImageType> GaussianFilterType;
typedef itk::SubtractImageFilter<InputImageType, InternalImageType, InternalImageType> SubstractFilterType;
typedef itk::MultiplyImageFilter<InternalImageType, InternalImageType, InternalImageType> MultiplyFilterType;
typedef itk::AddImageFilter<InputImageType, InternalImageType, InternalImageType> AddFilterType;

// sheetness prerequisites
typedef itk::HessianRecursiveGaussianImageFilter<InternalImageType> HessianFilterType;
typedef HessianFilterType::OutputImageType HessianImageType;
typedef HessianImageType::PixelType HessianPixelType;
typedef itk::FixedArray<double, HessianPixelType::Dimension> EigenValueArrayType;
typedef itk::Image<EigenValueArrayType, IMAGE_DIMENSION> EigenValueImageType;
typedef itk::SymmetricEigenAnalysisImageFilter<HessianImageType, EigenValueImageType> EigenAnalysisFilterType;
typedef itk::TraceImageFilter<HessianImageType, InternalImageType> TraceFilterType;
typedef itk::StatisticsImageFilter<InternalImageType> StatisticsFilterType;

// sheetness
typedef itk::KrcahSheetnessImageFilter<EigenValueImageType, double, OutputImageType> SheetnessFilterType;

// post processing
typedef itk::MaximumAbsoluteValueImageFilter<OutputImageType, OutputImageType, OutputImageType> MaximumAbsoluteValueFilterType;

// femur connected components
typedef itk::Image<unsigned long, IMAGE_DIMENSION> LabelImageType;
typedef itk::ScalarConnectedComponentImageFilter<OutputImageType, LabelImageType> ConnectedComponentImageFilterType;
typedef itk::RelabelComponentImageFilter<LabelImageType, LabelImageType> RelabelFilterType;
typedef itk::BinaryThresholdImageFilter<LabelImageType, LabelImageType> BinaryThresholdFilterType;
typedef itk::RescaleIntensityImageFilter<LabelImageType, OutputImageType> BinaryRescaleFilterType;


// functions
FileReaderType::Pointer readImage(char *pathInput);

OutputImageType::Pointer getSheetnessImage(InputImageType::Pointer input);
OutputImageType::Pointer calculateKrcahSheetness(InputImageType::Pointer input, float sigma);
OutputImageType::Pointer extractFemur(OutputImageType::Pointer input);

// expected CLI call:
// ./Testbench /path/to/input /path/to/output
int main(int argc, char *argv[]) {
    // read input
    FileReaderType::Pointer inputReader = readImage(argv[1]);
    InputImageType::Pointer inputImage = inputReader->GetOutput();

    // process
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
    // get the sheetness for both sigma with femur optimized sheetness implementation
    std::cout << "Processing with sigma=0.75..." << std::endl;
    OutputImageType::Pointer resultSigma075Krcah = calculateKrcahSheetness(input, 0.75);
    std::cout << "Processing with sigma=1.0..." << std::endl;
    OutputImageType::Pointer resultSigma100Krcah = calculateKrcahSheetness(input, 1.0);

    // combine the results
    std::cout << "combining results..." << std::endl;
    MaximumAbsoluteValueFilterType::Pointer maximumAbsoluteValueFilter = MaximumAbsoluteValueFilterType::New();
    maximumAbsoluteValueFilter->SetInput1(resultSigma075Krcah);
    maximumAbsoluteValueFilter->SetInput2(resultSigma100Krcah);

    maximumAbsoluteValueFilter->Update();
    return maximumAbsoluteValueFilter->GetOutput();
}

OutputImageType::Pointer calculateKrcahSheetness(InputImageType::Pointer input, float sigma) {

    /******
    * Input preprocessing
    ******/
    std::cout << "CPU time (in s):" << std::endl;
    std::clock_t begin = std::clock();
    std::clock_t beginTotal = std::clock();

    // I*G (discrete gauss)
    GaussianFilterType::Pointer m_DiffusionFilter = GaussianFilterType::New();
    m_DiffusionFilter->SetVariance(1);
    m_DiffusionFilter->SetInput(input);
    m_DiffusionFilter->Update();

    std::clock_t end = std::clock();
    double elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "diffusion: " << elapsedSecs << std::endl;
    begin = std::clock();

    // I - (I*G)
    SubstractFilterType::Pointer m_SubstractFilter = SubstractFilterType::New();
    m_SubstractFilter->SetInput1(input);
    m_SubstractFilter->SetInput2(m_DiffusionFilter->GetOutput()); // =s
    m_SubstractFilter->Update();

    end = std::clock();
    elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "substract: " << elapsedSecs << std::endl;
    begin = std::clock();

    // k(I-(I*G))
    MultiplyFilterType::Pointer m_MultiplyFilter = MultiplyFilterType::New();
    m_MultiplyFilter->SetInput(m_SubstractFilter->GetOutput());
    m_MultiplyFilter->SetConstant(10); // =k
    m_MultiplyFilter->Update();

    end = std::clock();
    elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "multiply: " << elapsedSecs << std::endl;
    begin = std::clock();

    // I+k*(I-(I*G))
    AddFilterType::Pointer m_AddFilter = AddFilterType::New();
    m_AddFilter->SetInput1(input);
    m_AddFilter->SetInput2(m_MultiplyFilter->GetOutput());
    m_AddFilter->Update();

    end = std::clock();
    elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "add: " << elapsedSecs << std::endl;
    begin = std::clock();

    /******
    * sheetness prerequisites
    ******/
    // hessian
    HessianFilterType::Pointer m_HessianFilter = HessianFilterType::New();
    m_HessianFilter->SetSigma(sigma);
    m_HessianFilter->SetInput(m_AddFilter->GetOutput());
    m_HessianFilter->Update();

    end = std::clock();
    elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "Hessian: " << elapsedSecs << std::endl;
    begin = std::clock();

    // eigen analysis
    EigenAnalysisFilterType::Pointer m_EigenAnalysisFilter = EigenAnalysisFilterType::New();
    m_EigenAnalysisFilter->SetDimension(IMAGE_DIMENSION);
    m_EigenAnalysisFilter->SetInput(m_HessianFilter->GetOutput());
    m_EigenAnalysisFilter->Update();

    end = std::clock();
    elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "Eigen analysis: " << elapsedSecs << std::endl;
    begin = std::clock();

    // calculate trace
    TraceFilterType::Pointer m_TraceFilter = TraceFilterType::New();
    m_TraceFilter->SetImageDimension(IMAGE_DIMENSION);
    m_TraceFilter->SetInput(m_HessianFilter->GetOutput());
    m_TraceFilter->Update();

    end = std::clock();
    elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "Trace: " << elapsedSecs << std::endl;
    begin = std::clock();

    // calculate average
    StatisticsFilterType::Pointer m_StatisticsFilter = StatisticsFilterType::New();
    m_StatisticsFilter->SetInput(m_TraceFilter->GetOutput());
    m_StatisticsFilter->Update();

    end = std::clock();
    elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "statistics: " << elapsedSecs << std::endl;
    begin = std::clock();

    /******
    * Sheetness
    ******/
    SheetnessFilterType::Pointer m_SheetnessFilter = SheetnessFilterType::New();
    m_SheetnessFilter->SetInput(m_EigenAnalysisFilter->GetOutput());
    m_SheetnessFilter->SetConstant(m_StatisticsFilter->GetMean());
    m_SheetnessFilter->Update();

    end = std::clock();
    elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "Sheetness: " << elapsedSecs << std::endl;

    std::clock_t endTotal = std::clock();
    double elapsedSecsTotal = double(endTotal - beginTotal) / CLOCKS_PER_SEC;
    std::cout << "------------" << std::endl;
    std::cout << "Total: " << elapsedSecsTotal << std::endl;
    std::cout << std::endl;

    return m_SheetnessFilter->GetOutput();
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