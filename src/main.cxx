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
#include "itkAbsImageFilter.h"
#include "itkMaximumImageFilter.h"
#include "itkMeanProjectionImageFilter.h"

#include "BroadcastingBinaryFunctorImageFilter.h"
#include "KrcahSheetnessFunctor.h"
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
typedef itk::TraceImageFilter<HessianImageType, InternalImageType> TraceFilterType;
typedef itk::MeanProjectionImageFilter<InternalImageType, InternalImageType> MeanProjectionFilterType;
typedef itk::FixedArray<double, HessianPixelType::Dimension> EigenValueArrayType;
typedef itk::Image<EigenValueArrayType, IMAGE_DIMENSION> EigenValueImageType;
typedef itk::SymmetricEigenAnalysisImageFilter<HessianImageType, EigenValueImageType> EigenAnalysisFilterType;

// sheetness
typedef itk::Functor::KrcahSheetness<EigenValueImageType::PixelType, MeanProjectionFilterType::OutputImageType::PixelType, InternalImageType::PixelType> SheetnessFunctor;
typedef itk::BroadcastingBinaryFunctorImageFilter<EigenValueImageType, InternalImageType, OutputImageType, SheetnessFunctor> SheetnessBroadcastingFilterType;

// post processing
typedef itk::Functor::Maximum<OutputPixelType, OutputPixelType, OutputPixelType> MaximumFunctorType;
typedef itk::BinaryFunctorImageFilter<OutputImageType, OutputImageType, OutputImageType, MaximumFunctorType> MaximumFilterType;
typedef itk::RescaleIntensityImageFilter<OutputImageType, OutputImageType> RescaleFilterType;
typedef itk::AbsImageFilter<InternalImageType, InternalImageType> AbsFilterType;

// functions
int process(char *in, char *out);

FileReaderType::Pointer readImage(char *pathInput);

OutputImageType::Pointer calculateKrcahSheetness(InputImageType::Pointer input, float sigma);

// expected CLI call:
// ./Testbench /path/to/input /path/to/output
int main(int argc, char *argv[]) {
    int ret = process(argv[1], argv[2]);
    return ret;
}

int process(char *inputPath, char *outputPath) {
    // read input
    FileReaderType::Pointer inputReader = readImage(inputPath);
    InputImageType::Pointer input = inputReader->GetOutput();

    // get the sheetness for both sigma with femur optimized sheetness implementation
    std::cout << "Processing with sigma=0.75..." << std::endl;
    OutputImageType::Pointer resultSigma075Krcah = calculateKrcahSheetness(input, 0.75);
    std::cout << "Processing with sigma=1.0..." << std::endl;
    OutputImageType::Pointer resultSigma100Krcah = calculateKrcahSheetness(input, 1.0);

    // abs of both results
    std::cout << "combining results..." << std::endl;
    AbsFilterType::Pointer m_AbsFilter075 = AbsFilterType::New();
    AbsFilterType::Pointer m_AbsFilter100 = AbsFilterType::New();
    m_AbsFilter075->SetInput(resultSigma075Krcah);
    m_AbsFilter100->SetInput(resultSigma100Krcah);

    // result(x,y) = max(abs(esultSigma075Krcah(x,y)), abs(resultSigma100Krcah(x,y)))
    MaximumFilterType::Pointer m_MaximumFilter = MaximumFilterType::New();
    m_MaximumFilter->SetInput1(m_AbsFilter075->GetOutput());
    m_MaximumFilter->SetInput2(m_AbsFilter100->GetOutput());

    // rescale the resulting image
    RescaleFilterType::Pointer m_RescaleFilter = RescaleFilterType::New();
    m_RescaleFilter->SetOutputMinimum(0.0);
    m_RescaleFilter->SetOutputMaximum(1.0);
    m_RescaleFilter->SetInput(m_MaximumFilter->GetOutput());

    // write result
    std::cout << "writing to file..." << std::endl;
    FileWriterType::Pointer writer = FileWriterType::New();
    writer->SetFileName(outputPath);
    writer->SetInput(m_MaximumFilter->GetOutput());
    writer->Update();

    return EXIT_SUCCESS;
}

OutputImageType::Pointer calculateKrcahSheetness(InputImageType::Pointer input, float sigma) {
    std::cout << "CPU time (in s):" << std::endl;

    /******
    * Input preprocessing
    ******/
    std::clock_t begin = std::clock();

    // I*G (anisotropic diffusion)
//    AnisotropicDiffusionFilterType::Pointer m_DiffusionFilter = AnisotropicDiffusionFilterType::New();
//    m_DiffusionFilter->SetNumberOfIterations(20);
//    m_DiffusionFilter->SetTimeStep(0.06);
//    m_DiffusionFilter->SetConductanceParameter(50);
//    m_DiffusionFilter->SetInput(input);
//    m_DiffusionFilter->Update();

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
    MeanProjectionFilterType::Pointer m_MeanProjectionFilter = MeanProjectionFilterType::New();
    m_MeanProjectionFilter->SetProjectionDimension(2);
    m_MeanProjectionFilter->SetInput(m_TraceFilter->GetOutput());
    m_MeanProjectionFilter->Update();

    end = std::clock();
    elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "Mean projection: " << elapsedSecs << std::endl;
    begin = std::clock();

    /******
    * Sheetness
    ******/
    SheetnessBroadcastingFilterType::Pointer m_SheetnessFilter = SheetnessBroadcastingFilterType::New();
    m_SheetnessFilter->SetInput1(m_EigenAnalysisFilter->GetOutput());
    m_SheetnessFilter->SetInput2(m_MeanProjectionFilter->GetOutput());
    m_SheetnessFilter->Update();

    end = std::clock();
    elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "Sheetness: " << elapsedSecs << std::endl;
    std::cout << std::endl;

    return m_SheetnessFilter->GetOutput();
}

FileReaderType::Pointer readImage(char *path) {
    FileReaderType::Pointer reader = FileReaderType::New();
    reader->SetFileName(path);
    return reader;
}