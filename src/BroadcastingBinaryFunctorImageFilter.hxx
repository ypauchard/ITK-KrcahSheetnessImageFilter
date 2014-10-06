#include "itkImageSliceConstIteratorWithIndex.h"

namespace itk {
    template<typename TInputImage1, typename TInputImage2, typename TOutputImage, typename TFunctor>
    BroadcastingBinaryFunctorImageFilter<TInputImage1, TInputImage2, TOutputImage, TFunctor>
    ::BroadcastingBinaryFunctorImageFilter() {
        this->SetNumberOfRequiredInputs(2);
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
        // Process object is not const-correct so the const casting is required.
        this->SetNthInput(1, const_cast< TInputImage2 * >( image2 ));
    }

    template<typename TInputImage1, typename TInputImage2, typename TOutputImage, typename TFunctor>
    void
    BroadcastingBinaryFunctorImageFilter<TInputImage1, TInputImage2, TOutputImage, TFunctor>
    ::ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread, ThreadIdType threadId) {
        // We use dynamic_cast since inputs are stored as DataObjects.  The ImageToImageFilter::GetInput(int)
        // always returns a pointer to a TInputImage1 so it cannot be used for the second input.
        const TInputImage1 *inputPtr1 = dynamic_cast< const TInputImage1 * >( ProcessObject::GetInput(0) );
        const TInputImage2 *inputPtr2 = dynamic_cast< const TInputImage2 * >( ProcessObject::GetInput(1) );
        TOutputImage *outputPtr = this->GetOutput(0);

        // abort if task is empty
        const SizeValueType size0 = outputRegionForThread.GetSize(0);
        if (size0 == 0) {
            return;
        }

        // processing
        const size_t numberOfLinesToProcess = outputRegionForThread.GetNumberOfPixels() / size0;

        ImageSliceConstIteratorWithIndex<TInputImage1> inputIt1(inputPtr1, outputRegionForThread);
        ImageRegionIterator<TInputImage2> inputIt2(inputPtr2, outputRegionForThread);
        ImageSliceConstIteratorWithIndex<TOutputImage> outputIt(outputPtr, outputRegionForThread);

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
            ++inputIt2;
        }
    }

}
