#include "gtest/gtest.h"

#include "ModifiedSheetnessImageFilter.h"

TEST(ModifiedSheetnessFunctor, int) {
  typedef int InternalPixelType;
  const unsigned int DIMENSION = 3;
  typedef  itk::FixedArray< InternalPixelType, DIMENSION >     EigenValueArrayType;
  typedef itk::Functor::ModifiedSheetness<EigenValueArrayType, InternalPixelType> FunctorType;
  FunctorType ModifiedSheetness;
  EigenValueArrayType eigenValueArray;

  eigenValueArray[0] = 0;
  eigenValueArray[1] = 0;
  eigenValueArray[2] = 0;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), 0);

  eigenValueArray[0] = 1;
  eigenValueArray[1] = 0;
  eigenValueArray[2] = 0;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.3934693402873666);

  eigenValueArray[0] = 0;
  eigenValueArray[1] = 0;
  eigenValueArray[2] = 1;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.3934693402873666);

  eigenValueArray[0] = 0;
  eigenValueArray[1] = 1;
  eigenValueArray[2] = 0;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.3934693402873666);

  eigenValueArray[0] = 1;
  eigenValueArray[1] = 1;
  eigenValueArray[2] = 0;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.6321205588285578);

  eigenValueArray[0] = 2;
  eigenValueArray[1] = 1;
  eigenValueArray[2] = 1;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.6369467760753132);

  eigenValueArray[0] = -2;
  eigenValueArray[1] = 1;
  eigenValueArray[2] = 1;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)0.6369467760753132);

  ModifiedSheetness.DetectDarkSheetsOn();
  eigenValueArray[0] = 2;
  eigenValueArray[1] = 1;
  eigenValueArray[2] = 1;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)0.6369467760753132);

  eigenValueArray[0] = -2;
  eigenValueArray[1] = 1;
  eigenValueArray[2] = 1;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.6369467760753132);
}

TEST(ModifiedSheetnessFunctor, double) {
  typedef double InternalPixelType;
  const unsigned int DIMENSION = 3;
  typedef  itk::FixedArray< InternalPixelType, DIMENSION >     EigenValueArrayType;
  typedef itk::Functor::ModifiedSheetness<EigenValueArrayType, InternalPixelType> FunctorType;
  FunctorType ModifiedSheetness;
  EigenValueArrayType eigenValueArray;

  eigenValueArray[0] = 0;
  eigenValueArray[1] = 0;
  eigenValueArray[2] = 0;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), 0);

  eigenValueArray[0] = 1;
  eigenValueArray[1] = 0;
  eigenValueArray[2] = 0;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.3934693402873666);

  eigenValueArray[0] = 0;
  eigenValueArray[1] = 0;
  eigenValueArray[2] = 1;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.3934693402873666);

  eigenValueArray[0] = 0;
  eigenValueArray[1] = 1;
  eigenValueArray[2] = 0;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.3934693402873666);

  eigenValueArray[0] = 1;
  eigenValueArray[1] = 1;
  eigenValueArray[2] = 0;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.6321205588285578);

  eigenValueArray[0] = 1.1;
  eigenValueArray[1] = 1;
  eigenValueArray[2] = 1;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.32328095982825455);

  eigenValueArray[0] = -1.1;
  eigenValueArray[1] = 1;
  eigenValueArray[2] = 1;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)0.32328095982825455);

  ModifiedSheetness.DetectDarkSheetsOn();
  eigenValueArray[0] = 1.1;
  eigenValueArray[1] = 1;
  eigenValueArray[2] = 1;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)0.32328095982825455);

  eigenValueArray[0] = -1.1;
  eigenValueArray[1] = 1;
  eigenValueArray[2] = 1;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.32328095982825455);
}

TEST(ModifiedSheetnessFunctor, Brightness) {
  typedef double InternalPixelType;
  const unsigned int DIMENSION = 3;
  typedef  itk::FixedArray< InternalPixelType, DIMENSION >     EigenValueArrayType;
  typedef itk::Functor::ModifiedSheetness<EigenValueArrayType, InternalPixelType> FunctorType;
  FunctorType ModifiedSheetness;
  EigenValueArrayType eigenValueArray;

  eigenValueArray[0] = 1;
  eigenValueArray[1] = 2;
  eigenValueArray[2] = 3;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.8566220679893631);

  eigenValueArray[0] = 1;
  eigenValueArray[1] = 2;
  eigenValueArray[2] = -3;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)0.8566220679893631);

  ModifiedSheetness.DetectDarkSheetsOn();
  eigenValueArray[0] = 1;
  eigenValueArray[1] = 2;
  eigenValueArray[2] = 3;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)0.8566220679893631);

  eigenValueArray[0] = 1;
  eigenValueArray[1] = 2;
  eigenValueArray[2] = -3;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.8566220679893631);
 
  ModifiedSheetness.DetectBrightSheetsOn();
  eigenValueArray[0] = 1;
  eigenValueArray[1] = 2;
  eigenValueArray[2] = 3;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.8566220679893631);

  eigenValueArray[0] = 1;
  eigenValueArray[1] = 2;
  eigenValueArray[2] = -3;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)0.8566220679893631);
}

TEST(ModifiedSheetnessFunctor, Alpha) {
  typedef double InternalPixelType;
  const unsigned int DIMENSION = 3;
  typedef  itk::FixedArray< InternalPixelType, DIMENSION >     EigenValueArrayType;
  typedef itk::Functor::ModifiedSheetness<EigenValueArrayType, InternalPixelType> FunctorType;
  FunctorType ModifiedSheetness;
  EigenValueArrayType eigenValueArray;

  eigenValueArray[0] = 1;
  eigenValueArray[1] = 2;
  eigenValueArray[2] = 3;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.8566220679893631);

  eigenValueArray[0] = 1;
  eigenValueArray[1] = 2;
  eigenValueArray[2] = -3;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)0.8566220679893631);

  ModifiedSheetness.SetAlpha(1.0);
  eigenValueArray[0] = 1;
  eigenValueArray[1] = 2;
  eigenValueArray[2] = 3;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.961391238876612);

  eigenValueArray[0] = 1;
  eigenValueArray[1] = 2;
  eigenValueArray[2] = -3;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)0.961391238876612);
 
  ModifiedSheetness.SetAlpha(2.5);
  eigenValueArray[0] = 1;
  eigenValueArray[1] = 2;
  eigenValueArray[2] = 3;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.9929587623609664);

  eigenValueArray[0] = 1;
  eigenValueArray[1] = 2;
  eigenValueArray[2] = -3;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)0.9929587623609664);
}

TEST(ModifiedSheetnessFunctor, C) {
  typedef double InternalPixelType;
  const unsigned int DIMENSION = 3;
  typedef  itk::FixedArray< InternalPixelType, DIMENSION >     EigenValueArrayType;
  typedef itk::Functor::ModifiedSheetness<EigenValueArrayType, InternalPixelType> FunctorType;
  FunctorType ModifiedSheetness;
  EigenValueArrayType eigenValueArray;

  eigenValueArray[0] = 1;
  eigenValueArray[1] = 2;
  eigenValueArray[2] = 3;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.8566220679893631);

  eigenValueArray[0] = 1;
  eigenValueArray[1] = 2;
  eigenValueArray[2] = -3;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)0.8566220679893631);

  ModifiedSheetness.SetC(0.5);
  eigenValueArray[0] = 1;
  eigenValueArray[1] = 2;
  eigenValueArray[2] = 3;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.8574039191598484);

  eigenValueArray[0] = 1;
  eigenValueArray[1] = 2;
  eigenValueArray[2] = -3;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)0.8574039191598484);
 
  ModifiedSheetness.SetC(2.5);
  eigenValueArray[0] = 1;
  eigenValueArray[1] = 2;
  eigenValueArray[2] = 3;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)-0.5776503445077833);

  eigenValueArray[0] = 1;
  eigenValueArray[1] = 2;
  eigenValueArray[2] = -3;
  ASSERT_EQ(ModifiedSheetness(eigenValueArray), (InternalPixelType)0.5776503445077833);
}