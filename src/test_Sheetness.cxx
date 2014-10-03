#include <iostream>
#include "gtest/gtest.h"

#include "itkSymmetricSecondRankTensor.h"
#include "TraceFunctor.h"

TEST(TraceFunctor, double2x2) {
    typedef double InternalPixelType;
    typedef itk::SymmetricSecondRankTensor<InternalPixelType, 2> MatrixType;
    typedef itk::Functor::Trace<MatrixType, InternalPixelType> FunctorType;
    FunctorType trace;
    MatrixType a;

    a(0, 0) = 1.0;
    a(1, 1) = 2.0;
    trace.SetDimension(2);
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
    trace.SetDimension(3);
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

    trace.SetDimension(DIMENSION);
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
    trace.SetDimension(3);
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
    trace.SetDimension(3);
    InternalPixelType traceValue = trace(a);
    ASSERT_FLOAT_EQ(traceValue, 6.6);
}

