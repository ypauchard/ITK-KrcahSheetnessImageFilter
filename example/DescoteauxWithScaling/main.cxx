#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkDescoteauxSheetnessImageFilter.h"

#include "itkDescoteauxSheetnessImageFilter.h"
#include "itkHessianRecursiveGaussianImageFilter.h"
#include "itkSymmetricEigenAnalysisImageFilter.h"
#include "itkThresholdImageFilter.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkMaskImageFilter.h"
#include "itkBinaryErodeImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkLabelStatisticsImageFilter.h"
#include "itkShiftScaleImageFilter.h"

#include "AutomaticSheetnessParameterEstimationImageFilter.h"

// Templating
const unsigned int IMAGE_DIMENSION = 3;
typedef short InputPixelType;
typedef unsigned char MaskPixelType;
typedef float SheetnessPixelType;
typedef itk::Image<InputPixelType, IMAGE_DIMENSION> InputImageType;
typedef itk::Image<MaskPixelType, IMAGE_DIMENSION> MaskImageType;
typedef itk::Image<SheetnessPixelType, IMAGE_DIMENSION> SheetnessImageType;

typedef itk::ImageFileReader<InputImageType> FileReaderType;
typedef itk::ImageFileWriter<SheetnessImageType> SheetnessWriterType;
typedef itk::ImageFileWriter<MaskImageType> MaskWriterType;

typedef itk::HessianRecursiveGaussianImageFilter< InputImageType >  HessianFilterType;
typedef HessianFilterType::OutputImageType                          HessianImageType;
typedef HessianImageType::PixelType                                 HessianPixelType;

typedef  itk::FixedArray< double, HessianPixelType::Dimension >     EigenValueArrayType;
typedef  itk::Image< EigenValueArrayType, IMAGE_DIMENSION >               EigenValueImageType;

typedef  itk::SymmetricEigenAnalysisImageFilter<HessianImageType, EigenValueImageType>     EigenAnalysisFilterType;
typedef itk::DescoteauxSheetnessImageFilter< EigenValueImageType, SheetnessImageType >   DescoteauxSheetnessFilterType;

typedef itk::BinaryThresholdImageFilter<InputImageType, MaskImageType> BinaryThresholdImageFilterType;
typedef itk::ThresholdImageFilter<InputImageType> ThresholdImageFilterType;
typedef itk::MaskImageFilter<InputImageType, MaskImageType, InputImageType> MaskImageFilterType;
typedef itk::BinaryBallStructuringElement<MaskImageType::PixelType, IMAGE_DIMENSION> StructuringElementType;
typedef itk::BinaryErodeImageFilter<MaskImageType, MaskImageType, StructuringElementType> BinaryErodeImageFilterType;
typedef itk::LabelStatisticsImageFilter<InputImageType, MaskImageType> LabelStatisticsImageFilterType;
typedef itk::ShiftScaleImageFilter<SheetnessImageType, SheetnessImageType> ShiftScaleImageFilterType;

typedef itk::AutomaticSheetnessParameterEstimationImageFilter<EigenValueImageType, MaskImageType> AutomaticSheetnessParameterEstimationImageFilterType;


int main(int argc, char *argv[]) {
    // Constants
    double sigma = 1;
    InputPixelType threshold = -750; // -3024 is the padding value
    double radius = 1;

    // Verify arguments
    if (argc != 4) {
        std::cerr << "Required: inputImage.mhd outputSheetness.mhd sigma" << std::endl;
        std::cerr << "inputImage.mhd:        3D image in Hounsfield Units -1024 to 3071" << std::endl;
        std::cerr << "outputSheetness.mhd:   3D image sheetness results." << std::endl;
        std::cerr << "sigma:                 Sigma value (double)" << std::endl;
        // std::cerr << "mask.mhd:              3D mask image." << std::endl;
        return EXIT_FAILURE;
    }
    sigma = atof(argv[3]);

    // read input
    std::cout << "Reading input " << argv[1] << std::endl;
    typename FileReaderType::Pointer reader = FileReaderType::New();
    reader->SetFileName(argv[1]);
    reader->Update();

    std::cout << "Preprocessing input" << std::endl;
    std::cout << "  Padding Threshold " << threshold << std::endl;
    std::cout << "  SE Radius         " << radius << std::endl;

    // Threshold
    typename BinaryThresholdImageFilterType::Pointer binaryFilter = BinaryThresholdImageFilterType::New();
    binaryFilter->SetInput(reader->GetOutput());
    binaryFilter->SetLowerThreshold(threshold);
    binaryFilter->SetOutsideValue(0);
    binaryFilter->SetInsideValue(1);

    // Erode
    StructuringElementType structuringElement;
    structuringElement.SetRadius(radius);
    structuringElement.CreateStructuringElement();

    typename BinaryErodeImageFilterType::Pointer erosionFilter = BinaryErodeImageFilterType::New();
    erosionFilter->SetInput(binaryFilter->GetOutput());
    erosionFilter->SetKernel(structuringElement);
    erosionFilter->SetErodeValue(1);
    erosionFilter->Update();

    // // Mask
    // typename MaskImageFilterType::Pointer maskingFilter = MaskImageFilterType::New();
    // maskingFilter->SetInput(reader->GetOutput());
    // maskingFilter->SetMaskImage(erosionFilter->GetOutput());
    // // maskingFilter->SetMaskingValue(1);
    // maskingFilter->SetOutsideValue(-100);
    // maskingFilter->Update();

    // std::cout << "Writing mask to " << argv[4] << std::endl;
    // typename MaskWriterType::Pointer maskWriter = MaskWriterType::New();
    // maskWriter->SetFileName(argv[4]);
    // maskWriter->SetInput(erosionFilter->GetOutput());
    // maskWriter->Update();

    // typename ThresholdImageFilterType::Pointer thresFilter = ThresholdImageFilterType::New();
    // thresFilter->SetInput(reader->GetOutput());
    // thresFilter->SetOutsideValue(threshold);
    // thresFilter->ThresholdBelow(threshold);
    // thresFilter->Update();

    // Hessian + EigenAnalysis
    std::cout << "Computing Hessian and performing Eigen-analysis" << std::endl;
    typename HessianFilterType::Pointer hessian = HessianFilterType::New();
    hessian->SetInput(reader->GetOutput());
    hessian->SetSigma(sigma);

    EigenAnalysisFilterType::Pointer eigen = EigenAnalysisFilterType::New();
    eigen->SetDimension(IMAGE_DIMENSION);
    eigen->SetInput(hessian->GetOutput());
    eigen->Update();

    // Get scaling parameters
    std::cout << "Automatically determining scaling parameters" << std::endl;
    typename AutomaticSheetnessParameterEstimationImageFilterType::Pointer scalerFilter = AutomaticSheetnessParameterEstimationImageFilterType::New();
    scalerFilter->SetInput(eigen->GetOutput());
    scalerFilter->SetLabelInput(erosionFilter->GetOutput());
    scalerFilter->SetLabel(1);
    scalerFilter->SetScale(0.05f);
    scalerFilter->Update();

    std::cout << "Determined parameters: " << std::endl;
    std::cout << "  Alpha " << scalerFilter->GetAlpha() << std::endl;
    std::cout << "  Beta  " << scalerFilter->GetBeta() << std::endl;
    std::cout << "  C     " << scalerFilter->GetC() << std::endl;

    // compute sheetness
    std::cout << "Computing sheetness..." << std::endl;
    typename DescoteauxSheetnessFilterType::Pointer sheetnessFilter = DescoteauxSheetnessFilterType::New();
    sheetnessFilter->SetInput(scalerFilter->GetOutput());
    sheetnessFilter->SetDetectBrightSheets(true);
    sheetnessFilter->SetSheetnessNormalization(scalerFilter->GetAlpha());
    sheetnessFilter->SetBloobinessNormalization(scalerFilter->GetBeta());
    sheetnessFilter->SetNoiseNormalization(scalerFilter->GetC());
    sheetnessFilter->Update();

    // Scale the image
    typename ShiftScaleImageFilterType::Pointer scaler = ShiftScaleImageFilterType::New();
    scaler->SetInput(sheetnessFilter->GetOutput());
    scaler->SetScale(100);

    // write output
    std::cout << "writing sheetness to file " << argv[2] << std::endl;
    typename SheetnessWriterType::Pointer writer = SheetnessWriterType::New();
    writer->SetFileName(argv[2]);
    writer->SetInput(scaler->GetOutput());
    writer->Update();

    return EXIT_SUCCESS;
}