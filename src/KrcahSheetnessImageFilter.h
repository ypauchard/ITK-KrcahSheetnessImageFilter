#ifndef __KrcahSheetnessImageFilter_h_
#define __KrcahSheetnessImageFilter_h_

#include "BroadcastingBinaryFunctorImageFilter.h"
#include "KrcahSheetnessFunctor.h"

namespace itk {
    template<typename TInputImage1, typename TInputImage2, typename TOutputImage>
    class KrcahSheetnessImageFilter :
            public BroadcastingBinaryFunctorImageFilter<TInputImage1, TInputImage2, TOutputImage,
                    Functor::KrcahSheetness<typename TInputImage1::PixelType,
                            typename TInputImage2::PixelType, typename TOutputImage::PixelType> > {
    public:
        // itk requirements
        typedef KrcahSheetnessImageFilter Self;
        typedef BroadcastingBinaryFunctorImageFilter<TInputImage1, TInputImage2, TOutputImage,
                Functor::KrcahSheetness<typename TInputImage1::PixelType,
                        typename TInputImage2::PixelType, typename TOutputImage::PixelType> > Superclass;
        typedef SmartPointer<Self> Pointer;
        typedef SmartPointer<const Self> ConstPointer;

        itkNewMacro(Self); // create the smart pointers and register with ITKs object factory
        itkTypeMacro(KrcahSheetnessImageFilter, BroadcastingBinaryFunctorImageFilter); // type information for runtime evaluation

    protected:
        KrcahSheetnessImageFilter() {
        };

        virtual ~KrcahSheetnessImageFilter() {
        };

    private:
        KrcahSheetnessImageFilter(const Self &); //purposely not implemented
        void operator=(const Self &);   //purposely not implemented
    };
}

#endif //__KrcahSheetnessImageFilter_h_