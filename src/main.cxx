// ITK
#include "itkImageFileReader.h"
#include "itkImageSpatialObject.h"
#include "itkHessianRecursiveGaussianImageFilter.h"
#include "itkSymmetricEigenAnalysisImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkAbsImageFilter.h"
#include "itkBinaryFunctorImageFilter.h"
#include "itkMaximumImageFilter.h"

#include "DescoteauxSheetnessImageFilter.h"
#include "DescoteauxSheetnessImageFilterFemur.h"

// vtk
#include "QuickView.h"

// pixel / image type
const unsigned int IMAGE_DIMENSION = 2;
typedef signed short InputPixelType;
typedef float InternalPixelType;
typedef InternalPixelType OutputPixelType;
typedef itk::Image<InputPixelType, IMAGE_DIMENSION> InputImageType;
typedef itk::Image<InternalPixelType, IMAGE_DIMENSION> InternalImageType;
typedef itk::Image<OutputPixelType, IMAGE_DIMENSION> OutputImageType;
typedef itk::ImageFileReader<InputImageType> FileReaderType;

// filters
typedef itk::CurvatureAnisotropicDiffusionImageFilter<InputImageType, InternalImageType> DiffusionFilterType;
typedef itk::HessianRecursiveGaussianImageFilter<InternalImageType> HessianFilterType;
typedef HessianFilterType::OutputImageType HessianImageType;
typedef HessianImageType::PixelType HessianPixelType;
typedef itk::FixedArray<double, HessianPixelType::Dimension> EigenValueArrayType;
typedef itk::Image<EigenValueArrayType, IMAGE_DIMENSION> EigenValueImageType;
typedef itk::SymmetricEigenAnalysisImageFilter<HessianImageType, EigenValueImageType> EigenAnalysisFilterType;
typedef itk::DescoteauxSheetnessImageFilter<EigenValueImageType, OutputImageType> SheetnessFilterType;
typedef itk::DescoteauxSheetnessImageFilterFemur<EigenValueImageType, OutputImageType> FemurSheetnessFilterType;
typedef itk::RescaleIntensityImageFilter<OutputImageType, OutputImageType> RescaleFilterType;
typedef itk::Functor::Abs<InternalImageType::PixelType, InternalImageType::PixelType> AbsFunctorType;
typedef itk::UnaryFunctorImageFilter<InternalImageType, InternalImageType, AbsFunctorType> AbsFilterType;
typedef itk::Functor::Maximum<OutputImageType::PixelType, OutputImageType::PixelType, OutputImageType::PixelType> MaximumFunctorType;
typedef itk::BinaryFunctorImageFilter<OutputImageType, OutputImageType, OutputImageType, MaximumFunctorType> MaximumFilterType;

// functions
void process(char *);
FileReaderType::Pointer readImage(char *path);
OutputImageType::Pointer calculateSheetness(InputImageType::Pointer input, float sigma);

OutputImageType::Pointer calculateFemurSheetness(InputImageType::Pointer input, float sigma);

// data
QuickView viewer;

// expected CLI call:
// ./Testbench /path/to/image.jpg
int main(int argc, char *argv[]) {
    process(argv[1]);

    return EXIT_SUCCESS;
}

void process(char *imagePath) {
    // read input
    FileReaderType::Pointer inputReader = readImage(imagePath);
    InputImageType::Pointer input = inputReader->GetOutput();

    // get the sheetness for both sigma with default sheetness implementation
    InternalImageType::Pointer resultSigma075Default = calculateSheetness(input, 0.75);
    InternalImageType::Pointer resultSigma100Default = calculateSheetness(input, 1.0);

    // result(x,y) = max(resultSigma075Default(x,y), resultSigma100Default(x,y))
    MaximumFilterType::Pointer m_MaximumFilterDefault = MaximumFilterType::New();
    m_MaximumFilterDefault->SetInput1(resultSigma075Default);
    m_MaximumFilterDefault->SetInput2(resultSigma100Default);
    m_MaximumFilterDefault->Update();

    // get the sheetness for both sigma with femur optimized sheetness implementation
    InternalImageType::Pointer resultSigma075Femur = calculateFemurSheetness(input, 0.75);
    InternalImageType::Pointer resultSigma100Femur = calculateFemurSheetness(input, 1.0);

    // result(x,y) = max(resultSigma075Femur(x,y), resultSigma100Femur(x,y))
    MaximumFilterType::Pointer m_MaximumFilterFemur = MaximumFilterType::New();
    m_MaximumFilterFemur->SetInput1(resultSigma075Femur);
    m_MaximumFilterFemur->SetInput2(resultSigma100Femur);
    m_MaximumFilterFemur->Update();

    // output
    viewer.AddImage(input.GetPointer(), true, "input");
    viewer.AddImage(m_MaximumFilterDefault->GetOutput(), true, "default sheetness");
    viewer.AddImage(m_MaximumFilterFemur->GetOutput(), true, "femur optimzed sheetness");
    viewer.Visualize();
}

OutputImageType::Pointer calculateSheetness(InputImageType::Pointer input, float sigma) {
    // anisotropic diffusion
    DiffusionFilterType::Pointer m_DiffusionFilter = DiffusionFilterType::New();
    m_DiffusionFilter->SetNumberOfIterations(10);
    m_DiffusionFilter->SetTimeStep(0.120);
    m_DiffusionFilter->SetConductanceParameter(2.0);

    // hessian
    HessianFilterType::Pointer m_HessianFilter = HessianFilterType::New();
    m_HessianFilter->SetSigma(sigma);

    // eigen analysis
    EigenAnalysisFilterType::Pointer m_EigenAnalysisFilter = EigenAnalysisFilterType::New();
    m_EigenAnalysisFilter->SetDimension(IMAGE_DIMENSION);

    // sheetness
    SheetnessFilterType::Pointer m_SheetnessFilter = SheetnessFilterType::New();
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

OutputImageType::Pointer calculateFemurSheetness(InputImageType::Pointer input, float sigma) {
    // anisotropic diffusion
    DiffusionFilterType::Pointer m_DiffusionFilter = DiffusionFilterType::New();
    m_DiffusionFilter->SetNumberOfIterations(10);
    m_DiffusionFilter->SetTimeStep(0.120);
    m_DiffusionFilter->SetConductanceParameter(2.0);

    // hessian
    HessianFilterType::Pointer m_HessianFilter = HessianFilterType::New();
    m_HessianFilter->SetSigma(sigma);

    // eigen analysis
    EigenAnalysisFilterType::Pointer m_EigenAnalysisFilter = EigenAnalysisFilterType::New();
    m_EigenAnalysisFilter->SetDimension(IMAGE_DIMENSION);

    // sheetness
    FemurSheetnessFilterType::Pointer m_SheetnessFilter = FemurSheetnessFilterType::New();
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
    reader->Update();
    return reader;
}