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
#include "MaximumAbsoluteValueImageFilter.h"
#include <vector>

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
typedef itk::MaximumAbsoluteValueImageFilter<SheetnessImageType, SheetnessImageType, SheetnessImageType> MaximumAbsoluteValueFilterType;

class ArgumentDatabase {
public:
    typedef std::vector<double> TSigmas;
    TSigmas sigmas;
    InputPixelType threshold;
    double automaticSheetnessScale;
    double radius;
    double intensityScaling;
    std::string inputFileName;
    std::string outputFileName;

    ArgumentDatabase(int argc, char *argv[]) :
         threshold(-750.0f)
        ,automaticSheetnessScale(0.05f)
        ,radius(2.5f)
        ,intensityScaling(100.0f)
        ,m_HasFailed(false)
    {
        // Verify arguments
        if (argc < 5) {
            std::cerr << "Required: inputImage.mhd outputSheetness.mhd N sigma1 ... sigmaN [intensityScaling] [threshold]" << std::endl;
            std::cerr << "inputImage.mhd:        3D image in Hounsfield Units -1024 to 3071" << std::endl;
            std::cerr << "outputSheetness.mhd:   3D image sheetness results." << std::endl;
            std::cerr << "N:                     Number of sigma values" << std::endl;
            std::cerr << "sigma:                 Sigma value (double)" << std::endl;
            std::cerr << "intensityScaling:      Value to scale output by (originally [0,1])" << std::endl;
            std::cerr << "threshold:             Threshold for generating a mask" << std::endl;
            m_HasFailed = true;
            return; // Bad practice. TODO: Errors should be exception based.
        }

        // Parse arguments
        inputFileName = argv[1];
        outputFileName = argv[2];

        unsigned int nSigma = atoi(argv[3]);
        for (unsigned int i = 0; i < nSigma; i++){
            sigmas.push_back(atof(argv[4+i]));
        }
        if (argc >= 5+nSigma) {
            automaticSheetnessScale = atof(argv[4+nSigma]);
        }
        if (argc >= 6+nSigma) {
            threshold = atof(argv[5+nSigma]);
        }

        if (sigmas.size() < 1) {
            std::cerr << "Please supply at least one sigma" << std::endl;
            m_HasFailed = true;
            return;  // Bad practice. TODO: Errors should be exception based.
        }

        if (automaticSheetnessScale <= 0 || automaticSheetnessScale > 1) {
            std::cerr << "automaticSheetnessScale must be a positive number in (0,1]" << std::endl;
            m_HasFailed = true;
            return;  // Bad practice. TODO: Errors should be exception based.
        }
    } // ArgumentDatabase(int argc, char *argv[])

    bool HasFailed(){ return m_HasFailed; }
private:
    bool m_HasFailed;
};

// Operator to print parameters
std::ostream &operator<<(std::ostream &os, ArgumentDatabase const &db) { 
    os << "Parameters:" << std::endl;
    os << "  Padding Threshold       " << db.threshold << std::endl;
    os << "  SE Radius               " << db.radius << std::endl;
    os << "  Sigmas                 ";// one spacing missing intentionally
    for (ArgumentDatabase::TSigmas::const_iterator i = db.sigmas.begin(); i != db.sigmas.end(); ++i)
        os << " " << *i;
    os << std::endl;
    os << "  automaticSheetnessScale " << db.automaticSheetnessScale << std::endl;
    os << "  intensityScaling        " << db.intensityScaling << std::endl;
    return os;
}

typename SheetnessImageType::Pointer calculateSheetnessAtScale(
     typename InputImageType::Pointer inputFilePointer
    , typename MaskImageType::Pointer maskFilePointer
    , double sigma
    , double automaticSheetnessScale
    , int label = 1
) {
    // Hessian + EigenAnalysis
    std::cout << "Computing Hessian and performing Eigen-analysis" << std::endl;
    typename HessianFilterType::Pointer hessian = HessianFilterType::New();
    hessian->SetInput(inputFilePointer);
    hessian->SetSigma(sigma);

    EigenAnalysisFilterType::Pointer eigen = EigenAnalysisFilterType::New();
    eigen->SetDimension(IMAGE_DIMENSION);
    eigen->SetInput(hessian->GetOutput());
    eigen->Update();

    // Get scaling parameters
    std::cout << "Automatically determining scaling parameters" << std::endl;
    typename AutomaticSheetnessParameterEstimationImageFilterType::Pointer scalerFilter = AutomaticSheetnessParameterEstimationImageFilterType::New();
    scalerFilter->SetInput(eigen->GetOutput());
    scalerFilter->SetLabelInput(maskFilePointer);
    scalerFilter->SetLabel(label);
    scalerFilter->SetScale(automaticSheetnessScale);
    scalerFilter->Update();

    std::cout << "Determined Sheetness parameters: " << std::endl;
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

    return sheetnessFilter->GetOutput();
}


int main(int argc, char *argv[]) {
    // Parse arguments
    ArgumentDatabase db(argc, argv);
    if (db.HasFailed()){
        return EXIT_FAILURE;
    } else {
        std::cout << db;
    }

    // read input
    std::cout << "Reading input " << db.inputFileName << std::endl;
    typename FileReaderType::Pointer reader = FileReaderType::New();
    reader->SetFileName(db.inputFileName);
    reader->Update();

    std::cout << "Preprocessing..." << std::endl;
    // Threshold
    typename BinaryThresholdImageFilterType::Pointer binaryFilter = BinaryThresholdImageFilterType::New();
    binaryFilter->SetInput(reader->GetOutput());
    binaryFilter->SetLowerThreshold(db.threshold);
    binaryFilter->SetOutsideValue(0);
    binaryFilter->SetInsideValue(1);

    // Erode
    StructuringElementType structuringElement;
    structuringElement.SetRadius(db.radius);
    structuringElement.CreateStructuringElement();

    typename BinaryErodeImageFilterType::Pointer erosionFilter = BinaryErodeImageFilterType::New();
    erosionFilter->SetInput(binaryFilter->GetOutput());
    erosionFilter->SetKernel(structuringElement);
    erosionFilter->SetErodeValue(1);
    erosionFilter->Update();

    // Loop over sigmas, take maximum value
    std::cout << "Current sigma: " << db.sigmas.at(0) << std::endl;
    typename SheetnessImageType::Pointer sheetnessFilePointer = calculateSheetnessAtScale(
         reader->GetOutput()
        ,erosionFilter->GetOutput()
        ,db.sigmas.at(0)
        ,db.automaticSheetnessScale
    );

    if (db.sigmas.size() > 1) { // need for std::next()
        for(ArgumentDatabase::TSigmas::const_iterator it = std::next(db.sigmas.begin()); it != db.sigmas.end(); ++it) {
            std::cout << "Current sigma: " << *it << std::endl;

            // Calculte the remaining (n-1) sheetnesses
            typename SheetnessImageType::Pointer tempSheetnessFilePointer = calculateSheetnessAtScale(
                 reader->GetOutput()
                ,erosionFilter->GetOutput()
                ,*it
                ,db.automaticSheetnessScale
            );

            // Take abs max
            typename MaximumAbsoluteValueFilterType::Pointer maximumAbsoluteValueFilter = MaximumAbsoluteValueFilterType::New();
            maximumAbsoluteValueFilter->SetInput1(sheetnessFilePointer);
            maximumAbsoluteValueFilter->SetInput2(tempSheetnessFilePointer);
            maximumAbsoluteValueFilter->Update();

            // Save max and move on
            sheetnessFilePointer = maximumAbsoluteValueFilter->GetOutput();
        }
    }

    // Scale the image
    typename ShiftScaleImageFilterType::Pointer scaler = ShiftScaleImageFilterType::New();
    scaler->SetInput(sheetnessFilePointer);
    scaler->SetScale(db.intensityScaling);

    // write output
    std::cout << std::endl;
    std::cout << "writing sheetness to file " << db.outputFileName << std::endl;
    typename SheetnessWriterType::Pointer writer = SheetnessWriterType::New();
    writer->SetFileName(db.outputFileName);
    writer->SetInput(scaler->GetOutput());
    writer->Update();

    return EXIT_SUCCESS;
}

