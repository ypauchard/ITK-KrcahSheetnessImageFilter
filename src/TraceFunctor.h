/**
* This functor calculates the trace tr(A) which is defined as the sum of all elements
* on the main diagonal of matrix A. tr(A) = a11+a22+...+ann
*/

#include "itkUnaryFunctorImageFilter.h"

namespace itk {
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
    }
}