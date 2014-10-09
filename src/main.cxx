#include <ctime>


// ITK
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"

#include "KrcahSheetnessFeatureGenerator.h"

// pixel / image type
const unsigned int IMAGE_DIMENSION = 3;
typedef short InputPixelType;
typedef float OutputPixelType;
typedef itk::Image<InputPixelType, IMAGE_DIMENSION> InputImageType;
typedef itk::Image<OutputPixelType, IMAGE_DIMENSION> OutputImageType;
typedef itk::ImageFileReader<InputImageType> FileReaderType;
typedef itk::ImageFileWriter<OutputImageType> FileWriterType;

// sheetness
typedef itk::KrcahSheetnessFeatureGenerator<InputImageType, OutputImageType> KrcahSheetnessFeatureGenerator;


// functions
FileReaderType::Pointer readImage(char *pathInput);
OutputImageType::Pointer getSheetnessImage(InputImageType::Pointer input);
OutputImageType::Pointer extractFemur(OutputImageType::Pointer input);

// expected CLI call:
// ./Testbench /path/to/input /path/to/output
int main(int argc, char *argv[]) {
    // read input
    FileReaderType::Pointer inputReader = readImage(argv[1]);
    InputImageType::Pointer inputImage = inputReader->GetOutput();

    // generate sheetness
    std::cout << "generating sheetness..." << std::endl;
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
    KrcahSheetnessFeatureGenerator::Pointer generator = KrcahSheetnessFeatureGenerator::New();
    generator->SetInput(input);
    generator->Update();
    return generator->GetOutput();
}

FileReaderType::Pointer readImage(char *path) {
    FileReaderType::Pointer reader = FileReaderType::New();
    reader->SetFileName(path);
    return reader;
}