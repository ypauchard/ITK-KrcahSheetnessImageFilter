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
typedef typename HessianFilterType::OutputImageType HessianImageType;
typedef typename HessianImageType::PixelType HessianPixelType;
typedef itk::FixedArray<double, HessianPixelType::Dimension> EigenValueArrayType;
typedef itk::Image<EigenValueArrayType, IMAGE_DIMENSION> EigenValueImageType;
typedef itk::SymmetricEigenAnalysisImageFilter<HessianImageType, EigenValueImageType> EigenAnalysisFilterType;
typedef itk::DescoteauxSheetnessImageFilter<EigenValueImageType, OutputImageType> SheetnessFilterType;
typedef itk::RescaleIntensityImageFilter<OutputImageType, OutputImageType> RescaleFilterType;
typedef itk::Functor::Abs<InternalImageType::PixelType, InternalImageType::PixelType> AbsFunctorType;
typedef itk::UnaryFunctorImageFilter<InternalImageType, InternalImageType, AbsFunctorType> AbsFilterType;
typedef itk::Functor::Maximum<OutputImageType::PixelType, OutputImageType::PixelType, OutputImageType::PixelType> MaximumFunctorType;
typedef itk::BinaryFunctorImageFilter<OutputImageType, OutputImageType, OutputImageType, MaximumFunctorType> MaximumFilterType;

// functions
void process(char *);
FileReaderType::Pointer readImage(char *path);
OutputImageType::Pointer calculateSheetness(InputImageType::Pointer input, float sigma);

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

    // get the sheetness for both sigma
    InternalImageType::Pointer resultSigma075 = calculateSheetness(input, 0.75);
    InternalImageType::Pointer resultSigma100 = calculateSheetness(input, 1.0);

    // result(x,y) = max(resultSigma075(x,y), resultSigma100(x,y))
    MaximumFilterType::Pointer m_MaximumFilter = MaximumFilterType::New();
    m_MaximumFilter->SetInput1(resultSigma075);
    m_MaximumFilter->SetInput2(resultSigma100);
    m_MaximumFilter->Update();

    // output
    viewer.AddImage(input.GetPointer(), true, "input");
    viewer.AddImage(resultSigma075.GetPointer(), true, "abs(r1), sigma=0.75");
    viewer.AddImage(resultSigma100.GetPointer(), true, "abs(r2), sigma=1.0");
    viewer.AddImage(m_MaximumFilter->GetOutput(), true, "r(x,y) = max(r1(x,y)), r2(x,y)");
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
    //m_SheetnessFilter->SetDetectBrightSheets(true);

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