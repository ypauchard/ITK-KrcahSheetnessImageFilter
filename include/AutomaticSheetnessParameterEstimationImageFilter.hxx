#ifndef _AutomaticSheetnessParameterEstimationImageFilter_hxx_
#define _AutomaticSheetnessParameterEstimationImageFilter_hxx_

namespace itk {
    template <class TInputImage, class TLabelImage>
    AutomaticSheetnessParameterEstimationImageFilter<TInputImage, TLabelImage>
    ::AutomaticSheetnessParameterEstimationImageFilter()
        : m_Alpha(0.5f), m_Beta(0.5f), m_C(0.5f),
        m_Label(1.0f), m_Scale(0.1f)
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
        if (this->GetLabelInput() == ITK_NULLPTR) { // Not verified yet...
            typename StatisticsImageFilterType::Pointer statisticsFilter = StatisticsImageFilterType::New();
            statisticsFilter->SetInput(frobeniusFilter->GetOutput());
            statisticsFilter->Update();
            
            // Set C
            m_C = static_cast<double>(this->GetScale() * statisticsFilter->GetMaximum());
        } else {
            typename LabelStatisticsImageFilterType::Pointer statisticsFilter = LabelStatisticsImageFilterType::New();
            statisticsFilter->SetInput(frobeniusFilter->GetOutput());
            statisticsFilter->SetLabelInput(this->GetLabelInput());
            statisticsFilter->Update();

            // Set C
            m_C = static_cast<double>(this->GetScale() * statisticsFilter->GetMaximum(this->GetLabel()));
        }

        // Set output
        this->GetOutput()->Graft(this->GetInput());
    }
}

#endif /* _AutomaticSheetnessParameterEstimationImageFilter_hxx_ */
