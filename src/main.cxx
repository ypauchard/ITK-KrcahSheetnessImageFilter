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

#include "DescoteauxSheetnessImageFilter.h"
#include "KrcahSheetnessImageFilter.h"
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

// filters
typedef itk::CurvatureAnisotropicDiffusionImageFilter<InputImageType, InternalImageType> DiffusionFilterType;
typedef itk::HessianRecursiveGaussianImageFilter<InternalImageType> HessianFilterType;
typedef HessianFilterType::OutputImageType HessianImageType;
typedef HessianImageType::PixelType HessianPixelType;
typedef itk::TraceImageFilter<HessianImageType, InternalImageType> TraceFilterType;
typedef itk::FixedArray<double, HessianPixelType::Dimension> EigenValueArrayType;
typedef itk::Image<EigenValueArrayType, IMAGE_DIMENSION> EigenValueImageType;
typedef itk::SymmetricEigenAnalysisImageFilter<HessianImageType, EigenValueImageType> EigenAnalysisFilterType;
typedef itk::KrcahSheetnessImageFilter<EigenValueImageType, OutputImageType> SheetnessFilter;
typedef itk::RescaleIntensityImageFilter<OutputImageType, OutputImageType> RescaleFilterType;
typedef itk::AbsImageFilter<InternalImageType, InternalImageType> AbsFilterType;
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
    // anisotropic diffusion
    DiffusionFilterType::Pointer m_DiffusionFilter = DiffusionFilterType::New();
    m_DiffusionFilter->SetNumberOfIterations(10);
    m_DiffusionFilter->SetTimeStep(0.06);
    m_DiffusionFilter->SetConductanceParameter(2.0);

    // hessian
    HessianFilterType::Pointer m_HessianFilter = HessianFilterType::New();
    m_HessianFilter->SetSigma(sigma);

    // eigen analysis
    EigenAnalysisFilterType::Pointer m_EigenAnalysisFilter = EigenAnalysisFilterType::New();
    m_EigenAnalysisFilter->SetDimension(IMAGE_DIMENSION);

    // calculate trace
    TraceFilterType::Pointer m_TraceFilter = TraceFilterType::New();
    m_TraceFilter->SetDimension(IMAGE_DIMENSION);

    // sheetness
    SheetnessFilter::Pointer m_SheetnessFilter = SheetnessFilter::New();
    m_SheetnessFilter->SetSheetnessNormalization(0.5); // = alpha
    m_SheetnessFilter->SetBloobinessNormalization(0.5); // = beta
    m_SheetnessFilter->SetNoiseNormalization(0.25); // = gamma
    m_SheetnessFilter->SetDetectBrightSheets(true);

    // abs
    AbsFilterType::Pointer m_AbsFilter = AbsFilterType::New();

    // rescale
    RescaleFilterType::Pointer m_RescaleFilter = RescaleFilterType::New();
    m_RescaleFilter->SetOutputMinimum(0.0);
    m_RescaleFilter->SetOutputMaximum(1.0);

    // connect pipeline
    m_DiffusionFilter->SetInput(input);
    m_HessianFilter->SetInput(m_DiffusionFilter->GetOutput());
    m_EigenAnalysisFilter->SetInput(m_HessianFilter->GetOutput());
    m_SheetnessFilter->SetInput(m_EigenAnalysisFilter->GetOutput());
    m_AbsFilter->SetInput(m_SheetnessFilter->GetOutput());
    m_RescaleFilter->SetInput(m_AbsFilter->GetOutput());

    m_RescaleFilter->Update();
    return m_RescaleFilter->GetOutput();
}

FileReaderType::Pointer readImage(char *path) {
    FileReaderType::Pointer reader = FileReaderType::New();
    reader->SetFileName(path);
    return reader;
}