#include <iostream>
#include "gtest/gtest.h"

#include "itkSymmetricSecondRankTensor.h"
#include "TraceImageFilter.h"
#include "MaximumAbsoluteValueImageFilter.h"
#include "KrcahBackgroundFunctor.h"

TEST(TraceFunctor, double2x2) {
    typedef double InternalPixelType;
    typedef itk::SymmetricSecondRankTensor<InternalPixelType, 2> MatrixType;
    typedef itk::Functor::Trace<MatrixType, InternalPixelType> FunctorType;
    FunctorType trace;
    MatrixType a;

    a(0, 0) = 1.0;
    a(1, 1) = 2.0;
    trace.SetImageDimension(2);
    InternalPixelType traceValue = trace(a);
    ASSERT_DOUBLE_EQ(traceValue, 3.0);

    a(0, 1) = 5.0;
    a(1, 0) = 5.0;
    traceValue = trace(a);
    EXPECT_DOUBLE_EQ(traceValue, 3.0);

    a(0, 0) = -5.0;
    a(1, 1) = -2.0;
    traceValue = trace(a);
    EXPECT_DOUBLE_EQ(traceValue, -7.0);

    a(0, 0) = -5.0;
    a(1, 1) = 2.0;
    traceValue = trace(a);
    EXPECT_DOUBLE_EQ(traceValue, -3.0);

    a(0, 0) = 0;
    a(1, 1) = 0;
    traceValue = trace(a);
    EXPECT_DOUBLE_EQ(traceValue, 0.0);

    a(0, 0) = 1234.56789;
    a(1, 1) = 9876.54321;
    traceValue = trace(a);
    EXPECT_DOUBLE_EQ(traceValue, 11111.1111);

    a(0, 0) = std::numeric_limits<double>::min();
    a(1, 1) = std::numeric_limits<double>::max();
    traceValue = trace(a);
    EXPECT_DOUBLE_EQ(traceValue, std::numeric_limits<double>::min() + std::numeric_limits<double>::max());

    a(0, 0) = 1 * pow(10, -30);
    a(1, 1) = 1 * pow(10, -31);
    traceValue = trace(a);
    EXPECT_DOUBLE_EQ(traceValue, 1.1 * pow(10, -30));
}

TEST(TraceFunctor, double3x3) {
    typedef double InternalPixelType;
    typedef itk::SymmetricSecondRankTensor<InternalPixelType, 3> MatrixType;
    typedef itk::Functor::Trace<MatrixType, InternalPixelType> FunctorType;
    FunctorType trace;
    MatrixType a;

    a(0, 0) = 1.0;
    a(1, 1) = 2.0;
    a(2, 2) = 3.0;
    trace.SetImageDimension(3);
    InternalPixelType traceValue = trace(a);
    ASSERT_DOUBLE_EQ(traceValue, 6.0);
}

TEST(TraceFunctor, double99x99) {
    const unsigned int DIMENSION = 99;
    typedef double InternalPixelType;
    typedef itk::SymmetricSecondRankTensor<InternalPixelType, DIMENSION> MatrixType;
    typedef itk::Functor::Trace<MatrixType, InternalPixelType> FunctorType;
    FunctorType trace;
    MatrixType a;

    for (unsigned int index = 0; index < DIMENSION; index++) {
        a(index, index) = index;
    }

    trace.SetImageDimension(DIMENSION);
    InternalPixelType traceValue = trace(a);
    ASSERT_DOUBLE_EQ(traceValue, ((DIMENSION - 1) * ((DIMENSION - 1) + 1) / 2));
}

TEST(TraceFunctor, int3x3) {
    typedef int InternalPixelType;
    typedef itk::SymmetricSecondRankTensor<InternalPixelType, 3> MatrixType;
    typedef itk::Functor::Trace<MatrixType, InternalPixelType> FunctorType;
    FunctorType trace;
    MatrixType a;

    a(0, 0) = 1;
    a(1, 1) = 2;
    a(2, 2) = 3;
    trace.SetImageDimension(3);
    InternalPixelType traceValue = trace(a);
    ASSERT_EQ(traceValue, 6);
}

TEST(TraceFunctor, float3x3) {
    typedef float InternalPixelType;
    typedef itk::SymmetricSecondRankTensor<InternalPixelType, 3> MatrixType;
    typedef itk::Functor::Trace<MatrixType, InternalPixelType> FunctorType;
    FunctorType trace;
    MatrixType a;

    a(0, 0) = 1.1;
    a(1, 1) = 2.2;
    a(2, 2) = 3.3;
    trace.SetImageDimension(3);
    InternalPixelType traceValue = trace(a);
    ASSERT_FLOAT_EQ(traceValue, 6.6);
}

TEST(MaximumAbsoluteValueFunctor, BasicTests) {
    typedef itk::Functor::MaximumAbsoluteValue<float, float, float> FunctorType;
    FunctorType functor;

    // regular
    EXPECT_EQ(-5, functor(-5, -2));
    EXPECT_EQ(5, functor(5, -2));

    // A == B or A == -B
    EXPECT_EQ(-5, functor(-5, 5)) << "Got wrong return with A = -B.";
    EXPECT_EQ(5, functor(5, -5)) << "Got wrong return with A = -B.";
    EXPECT_EQ(5, functor(5, 5));

    // with zero
    EXPECT_EQ(5, functor(0, 5));
    EXPECT_EQ(5, functor(5, 0));
    EXPECT_EQ(0, functor(0, 0));
    EXPECT_EQ(-1, functor(-1, 0));

    // min / max values
    float minFloat = std::numeric_limits<float>::min();
    float maxFloat = std::numeric_limits<float>::max();
    EXPECT_EQ(minFloat, functor(minFloat, 0));
    EXPECT_EQ(-minFloat, functor(-minFloat, 0));
    EXPECT_EQ(maxFloat, functor(maxFloat, -maxFloat));
    EXPECT_EQ(-maxFloat, functor(-maxFloat, maxFloat));
    EXPECT_EQ(maxFloat, functor(maxFloat, maxFloat));
}

TEST(KrcahNotBackgroundFunctor, BasicTests) {
    typedef itk::Functor::KrcahBackground<float, float, float> FunctorType;
    FunctorType functor;

    float minFloat = std::numeric_limits<float>::min();
    float maxFloat = std::numeric_limits<float>::max();

    EXPECT_EQ(1, functor(400, 0.1));
    EXPECT_EQ(0, functor(400, 0));
    EXPECT_EQ(0, functor(400, -0.1));

    EXPECT_EQ(0, functor(-76, 0.4576));
    EXPECT_EQ(0, functor(-28, 0.545));

    EXPECT_EQ(0, functor(399, 0.1));
    EXPECT_EQ(0, functor(399, 0));
    EXPECT_EQ(0, functor(399, -0.1));

    EXPECT_EQ(0, functor(0, 0.1));
    EXPECT_EQ(0, functor(-400, 0.1));
    EXPECT_EQ(0, functor(-400, 0));
}
