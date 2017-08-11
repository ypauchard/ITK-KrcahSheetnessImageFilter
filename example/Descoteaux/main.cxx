#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkDescoteauxSheetnessFeatureGenerator.h"

#include "itkDiscreteGaussianImageFilter.h"
#include "itkSubtractImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkAddImageFilter.h"
#include "itkCastImageFilter.h"

// Templating
const unsigned int IMAGE_DIMENSION = 3;
typedef short InputPixelType;
typedef float SheetnessPixelType;
typedef itk::Image<InputPixelType, IMAGE_DIMENSION> InputImageType;
typedef itk::Image<SheetnessPixelType, IMAGE_DIMENSION> SheetnessImageType;

typedef itk::ImageFileReader<InputImageType> FileReaderType;
typedef itk::ImageFileWriter<InputImageType> UnsharpWriterType;
typedef itk::ImageFileWriter<SheetnessImageType> SheetnessWriterType;

typedef itk::DescoteauxSheetnessFeatureGenerator<IMAGE_DIMENSION> DescoteauxSheetnessFeatureGeneratorType;
typedef DescoteauxSheetnessFeatureGeneratorType::SpatialObjectType    SpatialObjectType;
typedef itk::ImageSpatialObject< IMAGE_DIMENSION, InputPixelType  > InputImageSpatialObjectType;
typedef itk::ImageSpatialObject< IMAGE_DIMENSION, SheetnessPixelType > OutputImageSpatialObjectType;

typedef itk::CastImageFilter<InputImageType, InputImageType> CastFilterType;
typedef itk::DiscreteGaussianImageFilter<InputImageType, InputImageType> GaussianFilterType;
typedef itk::SubtractImageFilter<InputImageType, InputImageType, InputImageType> SubstractFilterType;
typedef itk::MultiplyImageFilter<InputImageType, InputImageType, InputImageType> MultiplyFilterType;
typedef itk::AddImageFilter<InputImageType, InputImageType, InputImageType> AddFilterType;

int main(int argc, char *argv[]) {
    // Constants
    double sigma = 1;
    double alpha = 0.5;
    double beta = 0.5;
    double c = 0.5;
    double k = 10;
    double unsharpSigma = 1;

    // Verify arguments
    if (argc != 4) {
        std::cerr << "Required: inputImage.mhd outputSheetness.mhd outputUnsharp.mhd" << std::endl;
        std::cerr << "inputImage.mhd:        3D image in Hounsfield Units -1024 to 3071" << std::endl;
        std::cerr << "outputSheetness.mhd:   3D image sheetness results." << std::endl;
        std::cerr << "outputUnsharp.mhd:     3D image unsharp image." << std::endl;
        return EXIT_FAILURE;
    }

    // read input
    std::cout << "Reading input " << argv[1] << std::endl;
    typename FileReaderType::Pointer reader = FileReaderType::New();
    reader->SetFileName(argv[1]);
    reader->Update();

    std::cout << "Generating unsharp of input" << std::endl;
    // I*G (discrete gauss)
    typename GaussianFilterType::Pointer gaussFilter = GaussianFilterType::New();
    gaussFilter->SetVariance(unsharpSigma);
    gaussFilter->SetInput(reader->GetOutput());

    // I - (I*G)
    typename SubstractFilterType::Pointer subtractFilter = SubstractFilterType::New();
    subtractFilter->SetInput1(reader->GetOutput());
    subtractFilter->SetInput2(gaussFilter->GetOutput());

    // k(I-(I*G))
    typename MultiplyFilterType::Pointer multiplyFilter = MultiplyFilterType::New();
    multiplyFilter->SetInput(subtractFilter->GetOutput());
    multiplyFilter->SetConstant(k);

    // I+k*(I-(I*G))
    typename AddFilterType::Pointer addFilter = AddFilterType::New();
    addFilter->SetInput1(reader->GetOutput());
    addFilter->SetInput2(multiplyFilter->GetOutput());
    addFilter->Update();

    // write output
    std::cout << "writing unsharp to file " << argv[3] << std::endl;
    typename UnsharpWriterType::Pointer unsharpWriter = UnsharpWriterType::New();
    unsharpWriter->SetFileName(argv[3]);
    unsharpWriter->SetInput(addFilter->GetOutput());
    unsharpWriter->Update();

    // compute sheetness
    std::cout << "Computing sheetness..." << std::endl;
    typename InputImageSpatialObjectType::Pointer inputObject = InputImageSpatialObjectType::New();
    inputObject->SetImage(addFilter->GetOutput());

    typename DescoteauxSheetnessFeatureGeneratorType::Pointer sheetnessFilterGenerator = DescoteauxSheetnessFeatureGeneratorType::New();
    sheetnessFilterGenerator->SetInput(inputObject);
    sheetnessFilterGenerator->SetSigma(sigma);
    sheetnessFilterGenerator->SetSheetnessNormalization(alpha);
    sheetnessFilterGenerator->SetBloobinessNormalization(beta);
    sheetnessFilterGenerator->SetNoiseNormalization(c);
    sheetnessFilterGenerator->DetectBrightSheetsOn();
    sheetnessFilterGenerator->Update();

    typename SpatialObjectType::ConstPointer sheetnessFeature = sheetnessFilterGenerator->GetFeature();
    typename OutputImageSpatialObjectType::ConstPointer outputObject = 
    dynamic_cast< const OutputImageSpatialObjectType * >( sheetnessFeature.GetPointer() );
    typename SheetnessImageType::ConstPointer sheetnessImage = outputObject->GetImage();

    // write output
    std::cout << "writing sheetness to file " << argv[2] << std::endl;
    typename SheetnessWriterType::Pointer writer = SheetnessWriterType::New();
    writer->SetFileName(argv[2]);
    writer->SetInput(sheetnessImage);
    writer->Update();

    return EXIT_SUCCESS;
}