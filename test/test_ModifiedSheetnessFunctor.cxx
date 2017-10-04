#include "gtest/gtest.h"

#include "ModifiedSheetnessImageFilter.h"

namespace {
  template <typename T>
  class ModifiedSheetnessFunctorTest : public ::testing::Test {
  protected:
    typedef T InternalPixelType;
    static const unsigned int DIMENSION = 3;
    typedef  itk::FixedArray< InternalPixelType, DIMENSION >     EigenValueArrayType;
    typedef itk::Functor::ModifiedSheetness<EigenValueArrayType, InternalPixelType> FunctorType;

    FunctorType mModifiedSheetness;
    EigenValueArrayType mEigenValueArray;

    ModifiedSheetnessFunctorTest() {
      mModifiedSheetness = FunctorType();
      mEigenValueArray = EigenValueArrayType();
    }
  
    virtual ~ModifiedSheetnessFunctorTest() {
      // Nothing to do
    }
  };

    // Define the templates we would like to test
    typedef ::testing::Types<char, int, char, float> TestingLabelTypes;
    TYPED_TEST_CASE(ModifiedSheetnessFunctorTest, TestingLabelTypes);

    TYPED_TEST(ModifiedSheetnessFunctorTest, ZeroInput) {
      this->mEigenValueArray[0] = 0;
      this->mEigenValueArray[1] = 0;
      this->mEigenValueArray[2] = 0;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), 0);
    }

    TYPED_TEST(ModifiedSheetnessFunctorTest, ZeroOneZeroInput) {
      this->mEigenValueArray[0] = 1;
      this->mEigenValueArray[1] = 0;
      this->mEigenValueArray[2] = 0;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)-0.3934693402873666);
    
      this->mEigenValueArray[0] = 0;
      this->mEigenValueArray[1] = 0;
      this->mEigenValueArray[2] = 1;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)-0.3934693402873666);
    
      this->mEigenValueArray[0] = 0;
      this->mEigenValueArray[1] = 1;
      this->mEigenValueArray[2] = 0;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)-0.3934693402873666);
    }

    TYPED_TEST(ModifiedSheetnessFunctorTest, ZeroOneOneInput) {
      this->mEigenValueArray[0] = 1;
      this->mEigenValueArray[1] = 1;
      this->mEigenValueArray[2] = 0;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)-0.6321205588285578);
    }

    TYPED_TEST(ModifiedSheetnessFunctorTest, SortsOnAbsoluteValueWithBrightSheets) {
      this->mEigenValueArray[0] = 2;
      this->mEigenValueArray[1] = 1;
      this->mEigenValueArray[2] = 1;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)-0.760871035093);

      this->mEigenValueArray[0] = -2;
      this->mEigenValueArray[1] = 1;
      this->mEigenValueArray[2] = 1;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)0.760871035093);
    }

    TYPED_TEST(ModifiedSheetnessFunctorTest, SortsOnAbsoluteValueWithDarkSheets) {
      this->mModifiedSheetness.DetectDarkSheetsOn();

      this->mEigenValueArray[0] = 2;
      this->mEigenValueArray[1] = 1;
      this->mEigenValueArray[2] = 1;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)0.760871035093);

      this->mEigenValueArray[0] = -2;
      this->mEigenValueArray[1] = 1;
      this->mEigenValueArray[2] = 1;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)-0.760871035093);
    }

    TYPED_TEST(ModifiedSheetnessFunctorTest, BrightnessWorks) {
      this->mEigenValueArray[0] = 1;
      this->mEigenValueArray[1] = 2;
      this->mEigenValueArray[2] = 3;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)-0.922274573238);

      this->mEigenValueArray[0] = 1;
      this->mEigenValueArray[1] = 2;
      this->mEigenValueArray[2] = -3;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)0.922274573238);

      this->mModifiedSheetness.DetectDarkSheetsOn();
      this->mEigenValueArray[0] = 1;
      this->mEigenValueArray[1] = 2;
      this->mEigenValueArray[2] = 3;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)0.922274573238);

      this->mEigenValueArray[0] = 1;
      this->mEigenValueArray[1] = 2;
      this->mEigenValueArray[2] = -3;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)-0.922274573238);
    
      this->mModifiedSheetness.DetectBrightSheetsOn();
      this->mEigenValueArray[0] = 1;
      this->mEigenValueArray[1] = 2;
      this->mEigenValueArray[2] = 3;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)-0.922274573238);

      this->mEigenValueArray[0] = 1;
      this->mEigenValueArray[1] = 2;
      this->mEigenValueArray[2] = -3;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)0.922274573238);
    }

    TYPED_TEST(ModifiedSheetnessFunctorTest, SetAndGetAlpha) {
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness.GetAlpha(), 0.5);
      this->mModifiedSheetness.SetAlpha(0);
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness.GetAlpha(), 0);
      this->mModifiedSheetness.SetAlpha(100);
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness.GetAlpha(), 100);
    }

    TYPED_TEST(ModifiedSheetnessFunctorTest, ChangingAlphaWorks) {
      this->mEigenValueArray[0] = 1;
      this->mEigenValueArray[1] = 2;
      this->mEigenValueArray[2] = 3;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)-0.922274573238);

      this->mEigenValueArray[0] = 1;
      this->mEigenValueArray[1] = 2;
      this->mEigenValueArray[2] = -3;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)0.922274573238);

      this->mModifiedSheetness.SetAlpha(1.0);
      this->mEigenValueArray[0] = 1;
      this->mEigenValueArray[1] = 2;
      this->mEigenValueArray[2] = 3;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)-0.979304847814);

      this->mEigenValueArray[0] = 1;
      this->mEigenValueArray[1] = 2;
      this->mEigenValueArray[2] = -3;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)0.979304847814);
    
      this->mModifiedSheetness.SetAlpha(2.5);
      this->mEigenValueArray[0] = 1;
      this->mEigenValueArray[1] = 2;
      this->mEigenValueArray[2] = 3;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)-0.995896145936);

      this->mEigenValueArray[0] = 1;
      this->mEigenValueArray[1] = 2;
      this->mEigenValueArray[2] = -3;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)0.995896145936);
    }

    TYPED_TEST(ModifiedSheetnessFunctorTest, SetAndGetC) {
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness.GetC(), 1.0);
      this->mModifiedSheetness.SetC(0);
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness.GetC(), 0);
      this->mModifiedSheetness.SetC(100);
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness.GetC(), 100);
    }

    TYPED_TEST(ModifiedSheetnessFunctorTest, ChangingCWorks) {
      this->mEigenValueArray[0] = 1;
      this->mEigenValueArray[1] = 2;
      this->mEigenValueArray[2] = 3;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)-0.922274573238);

      this->mEigenValueArray[0] = 1;
      this->mEigenValueArray[1] = 2;
      this->mEigenValueArray[2] = -3;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)0.922274573238);

      this->mModifiedSheetness.SetC(0.5);
      this->mEigenValueArray[0] = 1;
      this->mEigenValueArray[1] = 2;
      this->mEigenValueArray[2] = 3;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)-0.923116346386);

      this->mEigenValueArray[0] = 1;
      this->mEigenValueArray[1] = 2;
      this->mEigenValueArray[2] = -3;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)0.923116346386);
    
      this->mModifiedSheetness.SetC(2.5);
      this->mEigenValueArray[0] = 1;
      this->mEigenValueArray[1] = 2;
      this->mEigenValueArray[2] = 3;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)-0.621922134474);

      this->mEigenValueArray[0] = 1;
      this->mEigenValueArray[1] = 2;
      this->mEigenValueArray[2] = -3;
      EXPECT_DOUBLE_EQ(this->mModifiedSheetness(this->mEigenValueArray), (TypeParam)0.621922134474);
    }
}
