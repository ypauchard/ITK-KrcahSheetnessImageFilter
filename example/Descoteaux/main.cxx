#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkDescoteauxSheetnessImageFilter.h"

#include "itkDescoteauxSheetnessImageFilter.h"
#include "itkHessianRecursiveGaussianImageFilter.h"
#include "itkSymmetricEigenAnalysisImageFilter.h"

// Templating
const unsigned int IMAGE_DIMENSION = 3;
typedef short InputPixelType;
typedef float SheetnessPixelType;
typedef itk::Image<InputPixelType, IMAGE_DIMENSION> InputImageType;
typedef itk::Image<SheetnessPixelType, IMAGE_DIMENSION> SheetnessImageType;

typedef itk::ImageFileReader<InputImageType> FileReaderType;
typedef itk::ImageFileWriter<SheetnessImageType> SheetnessWriterType;

typedef itk::HessianRecursiveGaussianImageFilter< InputImageType >  HessianFilterType;
typedef HessianFilterType::OutputImageType                          HessianImageType;
typedef HessianImageType::PixelType                                 HessianPixelType;

typedef  itk::FixedArray< double, HessianPixelType::Dimension >     EigenValueArrayType;
typedef  itk::Image< EigenValueArrayType, IMAGE_DIMENSION >               EigenValueImageType;

typedef  itk::SymmetricEigenAnalysisImageFilter<HessianImageType, EigenValueImageType>     EigenAnalysisFilterType;
typedef itk::DescoteauxSheetnessImageFilter< EigenValueImageType, SheetnessImageType >   DescoteauxSheetnessFilterType;


int main(int argc, char *argv[]) {
    // Constants
    double sigma = 1;
    double alpha = 0.5;
    double beta = 0.5;
    double c = 0.5;

    // Verify arguments
    if (argc != 5) {
        std::cerr << "Required: inputImage.mhd outputSheetness.mhd sigmas normalization_constant" << std::endl;
        std::cerr << "inputImage.mhd:        3D image in Hounsfield Units -1024 to 3071" << std::endl;
        std::cerr << "outputSheetness.mhd:   3D image sheetness results." << std::endl;
        std::cerr << "sigmas:                Sigma value (double)" << std::endl;
        std::cerr << "normalization_constan: 3D image sheetness results." << std::endl;
        return EXIT_FAILURE;
    }
    sigma = atof(argv[3]);
    c = atof(argv[4]);

    // read input
    std::cout << "Reading input " << argv[1] << std::endl;
    typename FileReaderType::Pointer reader = FileReaderType::New();
    reader->SetFileName(argv[1]);
    reader->Update();

    // Hessian + EigenAnalysis
    std::cout << "Computing Hessian and performing Eigen-analysis" << std::endl;
    HessianFilterType::Pointer hessian = HessianFilterType::New();
    hessian->SetInput(reader->GetOutput());
    hessian->SetSigma(sigma);

    EigenAnalysisFilterType::Pointer eigen = EigenAnalysisFilterType::New();
    eigen->SetDimension(IMAGE_DIMENSION);
    eigen->SetInput(hessian->GetOutput());
    eigen->Update();

    // compute sheetness
    std::cout << "Computing sheetness..." << std::endl;
    DescoteauxSheetnessFilterType::Pointer sheetnessFilter = DescoteauxSheetnessFilterType::New();
    sheetnessFilter->SetInput(eigen->GetOutput());
    sheetnessFilter->SetDetectBrightSheets(true);
    sheetnessFilter->SetSheetnessNormalization(alpha);
    sheetnessFilter->SetBloobinessNormalization(beta);
    sheetnessFilter->SetNoiseNormalization(c);
    sheetnessFilter->Update();

    // write output
    std::cout << "writing sheetness to file " << argv[2] << std::endl;
    typename SheetnessWriterType::Pointer writer = SheetnessWriterType::New();
    writer->SetFileName(argv[2]);
    writer->SetInput(sheetnessFilter->GetOutput());
    writer->Update();

    return EXIT_SUCCESS;
}