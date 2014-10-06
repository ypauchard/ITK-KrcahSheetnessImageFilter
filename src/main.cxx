#include <ctime>

// ITK
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageSpatialObject.h"
#include "itkHessianRecursiveGaussianImageFilter.h"
#include "itkSymmetricEigenAnalysisImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkAbsImageFilter.h"
#include "itkBinaryFunctorImageFilter.h"
#include "itkMaximumImageFilter.h"
#include "itkMeanProjectionImageFilter.h"

#include "BroadcastingBinaryFunctorImageFilter.h"
#include "KrcahSheetnessFunctor.h"
#include "TraceImageFilter.h"

// pixel / image type
const unsigned int IMAGE_DIMENSION = 3;
typedef signed short InputPixelType;
typedef float InternalPixelType;
typedef InternalPixelType OutputPixelType;
typedef itk::Image<InputPixelType, IMAGE_DIMENSION> InputImageType;
typedef itk::Image<InternalPixelType, IMAGE_DIMENSION> InternalImageType;
typedef itk::Image<OutputPixelType, IMAGE_DIMENSION> OutputImageType;
typedef itk::ImageFileReader<InputImageType> FileReaderType;
typedef itk::ImageFileWriter<OutputImageType> FileWriterType;

// input processing
typedef itk::CurvatureAnisotropicDiffusionImageFilter<InputImageType, InternalImageType> DiffusionFilterType;

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
typedef itk::Functor::KrcahSheetness<EigenValueImageType::PixelType, MeanProjectionFilterType::OutputImageType, OutputImageType::PixelType> SheetnessFunctor;
typedef itk::BroadcastingBinaryFunctorImageFilter<EigenValueImageType, InternalImageType, OutputImageType, SheetnessFunctor> SheetnessBroadcastingFilterType;

// post processing
typedef itk::AbsImageFilter<InternalImageType, InternalImageType> AbsFilterType;
typedef itk::RescaleIntensityImageFilter<OutputImageType, OutputImageType> RescaleFilterType;
typedef itk::Functor::Maximum<OutputImageType::PixelType, OutputImageType::PixelType, OutputImageType::PixelType> MaximumFunctorType;
typedef itk::BinaryFunctorImageFilter<OutputImageType, OutputImageType, OutputImageType, MaximumFunctorType> MaximumFilterType;

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
    InternalImageType::Pointer resultSigma075Krcah = calculateKrcahSheetness(input, 0.75);
    std::cout << "Processing with sigma=1.0..." << std::endl;
    InternalImageType::Pointer resultSigma100Krcah = calculateKrcahSheetness(input, 1.0);

    // result(x,y) = max(resultSigma075Krcah(x,y), resultSigma100Krcah(x,y))
    std::cout << "combining results..." << std::endl;
    MaximumFilterType::Pointer m_MaximumFilterKrcah = MaximumFilterType::New();
    m_MaximumFilterKrcah->SetInput1(resultSigma075Krcah);
    m_MaximumFilterKrcah->SetInput2(resultSigma100Krcah);

    // write result
    std::cout << "writing to file..." << std::endl;
    FileWriterType::Pointer writer = FileWriterType::New();
    writer->SetFileName(outputPath);
    writer->SetInput(m_MaximumFilterKrcah->GetOutput());

    try {
        writer->Update();
    } catch (itk::ExceptionObject &error) {
        std::cerr << "Error while writing output: " << error << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

OutputImageType::Pointer calculateKrcahSheetness(InputImageType::Pointer input, float sigma) {
    std::cout << "CPU time (in s):" << std::endl;

    std::clock_t begin = std::clock();

    // anisotropic diffusion
    DiffusionFilterType::Pointer m_DiffusionFilter = DiffusionFilterType::New();
    m_DiffusionFilter->SetNumberOfIterations(10);
    m_DiffusionFilter->SetTimeStep(0.06);
    m_DiffusionFilter->SetConductanceParameter(2.0);
    m_DiffusionFilter->SetInput(input);
    m_DiffusionFilter->Update();

    std::clock_t end = std::clock();
    double elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "Anisotropic diffusion: " << elapsedSecs << std::endl;
    begin = std::clock();

    // hessian
    HessianFilterType::Pointer m_HessianFilter = HessianFilterType::New();
    m_HessianFilter->SetSigma(sigma);
    m_HessianFilter->SetInput(m_DiffusionFilter->GetOutput());
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

    // calculate average trace
    TraceFilterType::Pointer m_TraceFilter = TraceFilterType::New();
    m_TraceFilter->SetImageDimension(IMAGE_DIMENSION);
    m_TraceFilter->SetInput(m_HessianFilter->GetOutput());
    m_TraceFilter->Update();

    end = std::clock();
    elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "Trace: " << elapsedSecs << std::endl;
    begin = std::clock();

    MeanProjectionFilterType::Pointer m_MeanProjectionFilter = MeanProjectionFilterType::New();
    m_MeanProjectionFilter->SetProjectionDimension(2);
    m_MeanProjectionFilter->SetInput(m_TraceFilter->GetOutput());
    m_MeanProjectionFilter->Update();

    end = std::clock();
    elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "Mean projection: " << elapsedSecs << std::endl;
    begin = std::clock();

    // Sheetness
    SheetnessBroadcastingFilterType::Pointer m_SheetnessFilter = SheetnessBroadcastingFilterType::New();
    m_SheetnessFilter->SetInput1(m_EigenAnalysisFilter->GetOutput());
    m_SheetnessFilter->SetInput2(m_MeanProjectionFilter->GetOutput());
    m_SheetnessFilter->Update();

    end = std::clock();
    elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "Sheetness: " << elapsedSecs << std::endl;
    begin = std::clock();

    // abs
    AbsFilterType::Pointer m_AbsFilter = AbsFilterType::New();
    m_AbsFilter->SetInput(m_SheetnessFilter->GetOutput());
    m_AbsFilter->Update();

    end = std::clock();
    elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "Absolute: " << elapsedSecs << std::endl;
    begin = std::clock();

    // rescale
    RescaleFilterType::Pointer m_RescaleFilter = RescaleFilterType::New();
    m_RescaleFilter->SetOutputMinimum(0.0);
    m_RescaleFilter->SetOutputMaximum(1.0);
    m_RescaleFilter->SetInput(m_AbsFilter->GetOutput());
    m_RescaleFilter->Update();

    end = std::clock();
    elapsedSecs = double(end - begin) / CLOCKS_PER_SEC;
    std::cout << "Rescale: " << elapsedSecs << std::endl;
    std::cout << std::endl;

    return m_RescaleFilter->GetOutput();
}

FileReaderType::Pointer readImage(char *path) {
    FileReaderType::Pointer reader = FileReaderType::New();
    reader->SetFileName(path);
    return reader;
}