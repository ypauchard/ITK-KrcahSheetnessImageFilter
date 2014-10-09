#ifndef __KrcahSheetnessFeatureGenerator_hxx_
#define __KrcahSheetnessFeatureGenerator_hxx_

//#include "KrcahSheetnessFeatureGenerator.h"

namespace itk {
    template<typename TInput, typename TOutput>
    KrcahSheetnessFeatureGenerator<TInput, TOutput>
    ::KrcahSheetnessFeatureGenerator() {
        this->SetNumberOfRequiredInputs(1);
    }

    template<typename TInput, typename TOutput>
    KrcahSheetnessFeatureGenerator<TInput, TOutput>
    ::~KrcahSheetnessFeatureGenerator() {
    }

    template<typename TInput, typename TOutput>
    void KrcahSheetnessFeatureGenerator<TInput, TOutput>
    ::GenerateData() {
        // get input
        typename InputImageType::ConstPointer input(this->GetInput());

        // calc sheetness with the 2 sigmas
        typename OutputImageType::Pointer resultSigma075 = generateSheetnessWithSigma(input, 0.75);
        typename OutputImageType::Pointer resultSigma100 = generateSheetnessWithSigma(input, 1.0);

        // combine results
        typename MaximumAbsoluteValueFilterType::Pointer maximumAbsoluteValueFilter = MaximumAbsoluteValueFilterType::New();
        maximumAbsoluteValueFilter->SetInput1(resultSigma075);
        maximumAbsoluteValueFilter->SetInput2(resultSigma100);
        maximumAbsoluteValueFilter->Update();

        // copy output from last filter
        this->GetOutput()->Graft(maximumAbsoluteValueFilter->GetOutput());
    }

    template<typename TInput, typename TOutput>
    typename TOutput::Pointer KrcahSheetnessFeatureGenerator<TInput, TOutput>
    ::generateSheetnessWithSigma(typename TInput::ConstPointer input, float sigma) {
        /******
        * Input preprocessing
        ******/
        typename InputCastFilterType::Pointer castFilter = InputCastFilterType::New();
        castFilter->SetInput(input);
        castFilter->Update();

        // I*G (discrete gauss)
        typename GaussianFilterType::Pointer m_DiffusionFilter = GaussianFilterType::New();
        m_DiffusionFilter->SetVariance(1);
        m_DiffusionFilter->SetInput(castFilter->GetOutput());
        m_DiffusionFilter->Update();

        // I - (I*G)
        typename SubstractFilterType::Pointer m_SubstractFilter = SubstractFilterType::New();
        m_SubstractFilter->SetInput1(castFilter->GetOutput());
        m_SubstractFilter->SetInput2(m_DiffusionFilter->GetOutput()); // =s
        m_SubstractFilter->Update();

        // k(I-(I*G))
        typename MultiplyFilterType::Pointer m_MultiplyFilter = MultiplyFilterType::New();
        m_MultiplyFilter->SetInput(m_SubstractFilter->GetOutput());
        m_MultiplyFilter->SetConstant(10); // =k
        m_MultiplyFilter->Update();

        // I+k*(I-(I*G))
        typename AddFilterType::Pointer m_AddFilter = AddFilterType::New();
        m_AddFilter->SetInput1(castFilter->GetOutput());
        m_AddFilter->SetInput2(m_MultiplyFilter->GetOutput());
        m_AddFilter->Update();

        /******
        * sheetness prerequisites
        ******/
        // hessian
        typename HessianFilterType::Pointer m_HessianFilter = HessianFilterType::New();
        m_HessianFilter->SetSigma(sigma);
        m_HessianFilter->SetInput(m_AddFilter->GetOutput());
        m_HessianFilter->Update();

        // eigen analysis
        typename EigenAnalysisFilterType::Pointer m_EigenAnalysisFilter = EigenAnalysisFilterType::New();
        m_EigenAnalysisFilter->SetDimension(NDimension);
        m_EigenAnalysisFilter->SetInput(m_HessianFilter->GetOutput());
        m_EigenAnalysisFilter->Update();

        // calculate trace
        typename TraceFilterType::Pointer m_TraceFilter = TraceFilterType::New();
        m_TraceFilter->SetImageDimension(NDimension);
        m_TraceFilter->SetInput(m_HessianFilter->GetOutput());
        m_TraceFilter->Update();

        // calculate average
        typename StatisticsFilterType::Pointer m_StatisticsFilter = StatisticsFilterType::New();
        m_StatisticsFilter->SetInput(m_TraceFilter->GetOutput());
        m_StatisticsFilter->Update();

        /******
        * Sheetness
        ******/
        typename SheetnessFilterType::Pointer m_SheetnessFilter = SheetnessFilterType::New();
        m_SheetnessFilter->SetInput(m_EigenAnalysisFilter->GetOutput());
        m_SheetnessFilter->SetConstant(m_StatisticsFilter->GetMean());
        m_SheetnessFilter->Update();
        return m_SheetnessFilter->GetOutput();
    }
}

#endif // __KrcahSheetnessFeatureGenerator_hxx_