#include "itkImageSliceConstIteratorWithIndex.h"
#include "itkImageSliceIteratorWithIndex.h"

namespace itk {
    template<typename TInputImage1, typename TInputImage2, typename TOutputImage, typename TFunctor>
    BroadcastingBinaryFunctorImageFilter<TInputImage1, TInputImage2, TOutputImage, TFunctor>
    ::BroadcastingBinaryFunctorImageFilter() {
        this->SetNumberOfRequiredInputs(1);
        this->InPlaceOff();
    }

    template<typename TInputImage1, typename TInputImage2, typename TOutputImage, typename TFunctor>
    void
    BroadcastingBinaryFunctorImageFilter<TInputImage1, TInputImage2, TOutputImage, TFunctor>
    ::SetInput1(const TInputImage1 *image1) {
        // Process object is not const-correct so the const casting is required.
        this->SetNthInput(0, const_cast< TInputImage1 * >( image1 ));
    }

    template<typename TInputImage1, typename TInputImage2, typename TOutputImage, typename TFunctor>
    void
    BroadcastingBinaryFunctorImageFilter<TInputImage1, TInputImage2, TOutputImage, TFunctor>
    ::SetInput2(const TInputImage2 *image2) {
        // while ImageToImageFilter supports multiple inputs, it enforces the policy that all inputs occupy the same
        // physical space. since this is not the case here, we create a member variable on handle it ourselves
        m_Input2 = const_cast< TInputImage2 * >( image2 );
    }

    template<typename TInputImage1, typename TInputImage2, typename TOutputImage, typename TFunctor>
    void
    BroadcastingBinaryFunctorImageFilter<TInputImage1, TInputImage2, TOutputImage, TFunctor>
    ::ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread, ThreadIdType threadId) {
        const TInputImage1 *inputPtr1 = this->GetInput(0);
        const typename TInputImage2::Pointer inputPtr2 = m_Input2;
        TOutputImage *outputPtr = this->GetOutput(0);

        // abort if task is empty
        const SizeValueType size0 = outputRegionForThread.GetSize(0);
        if (size0 == 0) {
            return;
        }

        // processing
        const size_t numberOfLinesToProcess = outputRegionForThread.GetNumberOfPixels() / size0;

        ImageSliceConstIteratorWithIndex<TInputImage1> inputIt1(inputPtr1, outputRegionForThread);
        ImageRegionConstIteratorWithIndex<TInputImage2> inputIt2(inputPtr2, inputPtr2->GetLargestPossibleRegion());
        ImageSliceIteratorWithIndex<TOutputImage> outputIt(outputPtr, outputRegionForThread);

        inputIt1.SetFirstDirection(0);
        inputIt1.SetSecondDirection(1);
        inputIt1.GoToBegin();
        typename TInputImage1::IndexType idxIt1 = inputIt1.GetIndex();
        idxIt1[2] = 1;
        inputIt2.SetIndex(idxIt1);

        ProgressReporter progress(this, threadId, numberOfLinesToProcess);

        while (!inputIt1.IsAtEnd() && !inputIt2.IsAtEnd()) {
            while (!inputIt1.IsAtEndOfSlice()) {
                while (!inputIt1.IsAtEndOfLine()) {
                    outputIt.Set(m_Functor(inputIt1.Get(), inputIt2.Get()));
                    ++inputIt1;
                    ++outputIt;
                }
                inputIt1.NextLine();
                outputIt.NextLine();
                progress.CompletedPixel();
            }
            inputIt1.NextSlice();
            outputIt.NextSlice();
            inputIt2.SetIndex(idxIt1);
        }
    }

}
