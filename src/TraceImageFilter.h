#include "itkUnaryFunctorImageFilter.h"

namespace itk {
    /**
    * This functor calculates the trace tr(A) which is defined as the sum of all elements
    * on the main diagonal of matrix A. tr(A) = a11+a22+...+ann
    */
    namespace Functor {
        template<class TInput, class TOutput>
        class Trace {
        public:
            Trace() : m_Dimension(0) {
            };

            ~Trace() {
            };

            inline TOutput operator()(const TInput &A) {
                double sum = 0.0; // calculate with double precision
                for (unsigned int index = 0; index < m_Dimension; index++) {
                    sum += A(index, index);
                }

                TOutput traceValue(sum);
                return traceValue;
            }

            void SetDimension(unsigned int n) {
                m_Dimension = n;
            }

        protected:
            unsigned int m_Dimension;
        };
    } // end namespace functor

    template<typename TInputImage, typename TOutputImage = TInputImage>
    class TraceImageFilter : public UnaryFunctorImageFilter<
            TInputImage,
            TOutputImage,
            Functor::Trace<typename TInputImage::PixelType, typename TOutputImage::PixelType> > {
    public:
        // itk requirements
        typedef TraceImageFilter Self;
        typedef UnaryFunctorImageFilter<
                TInputImage,
                TOutputImage,
                Functor::Trace<typename TInputImage::PixelType, typename TOutputImage::PixelType> > Superclass;
        typedef SmartPointer<Self> Pointer;
        typedef SmartPointer<const Self> ConstPointer;

        itkNewMacro(Self); // create the smart pointers and register with ITKs object factory
        itkTypeMacro(TraceImageFilter, UnaryFunctorImageFilter); // type information for runtime evaluation

        // methods
        void SetDimension(unsigned int n) {
            this->GetFunctor().SetDimension(n);
        }

        unsigned int GetDimension() {
            return this->GetFunctor().GetDimension();
        }

#ifdef ITK_USE_CONCEPT_CHECKING
        // input is numeric
        itkConceptMacro(InputHasNumericTraitsCheck,
                (Concept::HasNumericTraits<typename TInputImage::PixelType::ValueType>));

        // functor output (= double) can be converted to output pixeltype
        itkConceptMacro(DoubleConvertibleToOutputCheck,
                (Concept::Convertible<double, typename TOutputImage::PixelType>));

        // TODO: add concept check if TInputImage::PixelType is square matrix
#endif

    protected:
        TraceImageFilter() {
        };

        virtual ~TraceImageFilter() {
        };

    private:
        TraceImageFilter(const Self &); //purposely not implemented
        void operator=(const Self &);   //purposely not implemented
    };
}