/**
* TODO:
* - Input2 is 3d with the relevant axis dimension = 1. change to 2d?
* - dimension along which to broadcast is set statically
*/

#ifndef __BroadcastingBinaryFunctorImageFilter_h_
#define __BroadcastingBinaryFunctorImageFilter_h_

#include "itkInPlaceImageFilter.h"

namespace itk {
    template<typename TInputImage1, typename TInputImage2, typename TOutputImage, typename TFunctor>
    class BroadcastingBinaryFunctorImageFilter : public InPlaceImageFilter<TInputImage1, TOutputImage> {
    public:
        /** Standard class typedefs. */
        typedef BroadcastingBinaryFunctorImageFilter Self;
        typedef InPlaceImageFilter<TInputImage1, TOutputImage> Superclass;
        typedef SmartPointer<Self> Pointer;
        typedef SmartPointer<const Self> ConstPointer;

        /** Method for creation through the object factory. */
        itkNewMacro(Self);

        /** Run-time type information (and related methods). */
        itkTypeMacro(ImageFilterMultipleInputs, ImageToImageFilter);

        /** Some convenient typedefs. */
        typedef TFunctor FunctorType;
        typedef TInputImage1 Input1ImageType;
        typedef typename Input1ImageType::ConstPointer Input1ImagePointer;
        typedef typename Input1ImageType::RegionType Input1ImageRegionType;
        typedef typename Input1ImageType::PixelType Input1ImagePixelType;
        typedef SimpleDataObjectDecorator<Input1ImagePixelType>
                DecoratedInput1ImagePixelType;

        typedef TInputImage2 Input2ImageType;
        typedef typename Input2ImageType::ConstPointer Input2ImagePointer;
        typedef typename Input2ImageType::RegionType Input2ImageRegionType;
        typedef typename Input2ImageType::PixelType Input2ImagePixelType;
        typedef SimpleDataObjectDecorator<Input2ImagePixelType>
                DecoratedInput2ImagePixelType;

        typedef TOutputImage OutputImageType;
        typedef typename OutputImageType::Pointer OutputImagePointer;
        typedef typename OutputImageType::RegionType OutputImageRegionType;
        typedef typename OutputImageType::PixelType OutputImagePixelType;

        /** Methods */
        virtual void SetInput1(const TInputImage1 *image1);

        virtual void SetInput2(const TInputImage2 *image1);

        // getters / setters
        FunctorType &GetFunctor() {
            return m_Functor;
        }

        const FunctorType &GetFunctor() const {
            return m_Functor;
        }

        void SetFunctor(const FunctorType &functor) {
            if (m_Functor != functor) {
                m_Functor = functor;
                this->Modified();
            }
        }

#ifdef ITK_USE_CONCEPT_CHECKING
// TODO:
#endif

    protected:
        BroadcastingBinaryFunctorImageFilter();

        ~BroadcastingBinaryFunctorImageFilter() {
        }

        // work distribution is handled by the superclass
        void ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread,
                ThreadIdType threadId);

        void VerifyInputInformation();

        void GenerateInputRequestedRegion();

    private:
        BroadcastingBinaryFunctorImageFilter(const Self &); //purposely not implemented
        void operator=(const Self &);                       //purposely not implemented

        FunctorType m_Functor;
    };
} //namespace ITK


#ifndef ITK_MANUAL_INSTANTIATION

#include "BroadcastingBinaryFunctorImageFilter.hxx"

#endif


#endif // __BroadcastingBinaryFunctorImageFilter_h_