#ifndef _AutomaticSheetnessParameterEstimationImageFilter_hxx_
#define _AutomaticSheetnessParameterEstimationImageFilter_hxx_

namespace itk {
    template <class TInputImage, class TLabelImage>
    AutomaticSheetnessParameterEstimationImageFilter<TInputImage, TLabelImage>
    ::AutomaticSheetnessParameterEstimationImageFilter()
        : m_Alpha(0.5), m_Beta(0.5), m_C(0.5),
        m_Label(1.0f)
    {
        this->SetNumberOfRequiredInputs(1);
    }

    template <class TInputImage, class TLabelImage>
    AutomaticSheetnessParameterEstimationImageFilter<TInputImage, TLabelImage>
    ::~AutomaticSheetnessParameterEstimationImageFilter() {
    }

    template <class TInputImage, class TLabelImage>
    void AutomaticSheetnessParameterEstimationImageFilter<TInputImage, TLabelImage>
    ::GenerateData() {
        // Compute Frobenius norm
        typename FrobeniusNormImageFilterType::Pointer frobeniusFilter = FrobeniusNormImageFilterType::New();
        frobeniusFilter->SetInput(this->GetInput());

        // Compute max
        typename LabelStatisticsImageFilterType::Pointer statisticsFilter = LabelStatisticsImageFilterType::New();
        statisticsFilter->SetInput(frobeniusFilter->GetOutput());
        statisticsFilter->SetLabelInput(this->GetLabelInput());
        statisticsFilter->Update();

        // Set C
        // m_C = static_cast<double>(0.5 * statisticsFilter->GetMaximum(this->GetLabel()));
        m_C = static_cast<double>(0.1 * statisticsFilter->GetMaximum(this->GetLabel()));

        // Set output
        this->GetOutput()->Graft(this->GetInput());
    }
}

#endif /* _AutomaticSheetnessParameterEstimationImageFilter_hxx_ */
