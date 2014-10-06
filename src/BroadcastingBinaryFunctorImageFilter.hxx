#include "itkImageSliceConstIteratorWithIndex.h"
#include "itkImageSliceIteratorWithIndex.h"

namespace itk {
    template<typename TInputImage1, typename TInputImage2, typename TOutputImage, typename TFunctor>
    BroadcastingBinaryFunctorImageFilter<TInputImage1, TInputImage2, TOutputImage, TFunctor>
    ::BroadcastingBinaryFunctorImageFilter() {
        this->SetNumberOfRequiredInputs(2);
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
        // we will use the multiple input system provided by ImageToImageFilter to ensure all pipeline
        // functionality works as expected. however, ImageToImageFilter expects all images to be of the
        // same size! we have to override certain functions to make it work, namely VerifyInputInformation()
        // and GenerateInputRequestedRegion().
        this->SetNthInput(1, const_cast< TInputImage2 * >( image2 ));
    }

    template<typename TInputImage1, typename TInputImage2, typename TOutputImage, typename TFunctor>
    void
    BroadcastingBinaryFunctorImageFilter<TInputImage1, TInputImage2, TOutputImage, TFunctor>
    ::ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread, ThreadIdType threadId) {
        const TInputImage1 *inputPtr1 = dynamic_cast< const TInputImage1 * >( ProcessObject::GetInput(0) );
        const TInputImage2 *inputPtr2 = dynamic_cast< const TInputImage2 * >( ProcessObject::GetInput(1) );
        TOutputImage *outputPtr = this->GetOutput();

        // abort if task is empty
        const SizeValueType size0 = outputRegionForThread.GetSize(0);
        if (size0 == 0) {
            return;
        }

        // processing
        const size_t numberOfLinesToProcess = outputRegionForThread.GetNumberOfPixels() / size0;

        ImageSliceConstIteratorWithIndex<TInputImage1> inputIt1(inputPtr1, outputRegionForThread);
        ImageRegionConstIteratorWithIndex<TInputImage2> inputIt2(inputPtr2, inputPtr2->GetLargestPossibleRegion());
        ImageRegionIterator<TOutputImage> outputIt(outputPtr, outputRegionForThread);

        // TODO: replace the static definition of the 'flat' plane with something that works in every case
        inputIt1.SetFirstDirection(0);
        inputIt1.SetSecondDirection(1);
        inputIt1.GoToBegin();

        outputIt.GoToBegin();
        typename TInputImage1::IndexType idxIt1 = inputIt1.GetIndex();
        idxIt1[2] = 0; // TODO: replace static dimension
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
                progress.CompletedPixel();
            }
            inputIt1.NextSlice();
            inputIt2.SetIndex(idxIt1);
        }
    }

    template<typename TInputImage1, typename TInputImage2, typename TOutputImage, typename TFunctor>
    void
    BroadcastingBinaryFunctorImageFilter<TInputImage1, TInputImage2, TOutputImage, TFunctor>
    ::VerifyInputInformation() {
        // itkImageToImageFilter.hxx implements a check for identical physical space which obviously
        // fails with this filter, so we overwrite it
    }

    template<typename TInputImage1, typename TInputImage2, typename TOutputImage, typename TFunctor>
    void
    BroadcastingBinaryFunctorImageFilter<TInputImage1, TInputImage2, TOutputImage, TFunctor>
    ::GenerateInputRequestedRegion() {
        Superclass::GenerateInputRequestedRegion();

        // the previously called itkImageToImageFilter.hxx implementation of this method sets the requested region
        // for all inputs to the output request region. again, this fails in this filer since our 2nd input differs
        // in size.
        TInputImage2 *inputPtr2 = dynamic_cast< TInputImage2 * >( ProcessObject::GetInput(1) );
        inputPtr2->SetRequestedRegion(inputPtr2->GetLargestPossibleRegion());
    }

    template<typename TInputImage1, typename TInputImage2, typename TOutputImage, typename TFunctor>
    void
    BroadcastingBinaryFunctorImageFilter<TInputImage1, TInputImage2, TOutputImage, TFunctor>
    ::GenerateOutputInformation() {
        const DataObject *input = ITK_NULLPTR;
        Input1ImagePointer inputPtr1 = dynamic_cast< const TInputImage1 * >( ProcessObject::GetInput(0) );

        for (unsigned int idx = 0; idx < this->GetNumberOfOutputs(); ++idx) {
            DataObject *output = this->GetOutput(idx);
            if (output) {
                output->CopyInformation(inputPtr1);
            }
        }
    }

}
