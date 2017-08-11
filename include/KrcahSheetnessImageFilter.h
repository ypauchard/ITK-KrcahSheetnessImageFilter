#ifndef __KrcahSheetnessImageFilter_h_
#define __KrcahSheetnessImageFilter_h_

#include "itkBinaryFunctorImageFilter.h"
#include "KrcahSheetnessFunctor.h"

namespace itk {
    template<typename TInputImage, typename TConstant, typename TOutputImage>
    class KrcahSheetnessImageFilter :
            public BinaryFunctorImageFilter<TInputImage, Image<TConstant, TInputImage::ImageDimension>, TOutputImage,
                    Functor::KrcahSheetness<typename TInputImage::PixelType, TConstant, typename TOutputImage::PixelType> > {
    public:
        // itk requirements
        typedef KrcahSheetnessImageFilter Self;
        typedef BinaryFunctorImageFilter<TInputImage, Image<TConstant, TInputImage::ImageDimension>, TOutputImage,
                Functor::KrcahSheetness<typename TInputImage::PixelType, TConstant, typename TOutputImage::PixelType> > Superclass;
        typedef SmartPointer<Self> Pointer;
        typedef SmartPointer<const Self> ConstPointer;

        itkNewMacro(Self); // create the smart pointers and register with ITKs object factory
        itkTypeMacro(KrcahSheetnessImageFilter, BinaryFunctorImageFilter); // type information for runtime evaluation

        // member functions
        void SetAlpha(double value) {
            this->GetFunctor().SetAlpha(value);
        }

        void SetBeta(double value) {
            this->GetFunctor().SetBeta(value);
        }

        void SetGamma(double value) {
            this->GetFunctor().SetGamma(value);
        }

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