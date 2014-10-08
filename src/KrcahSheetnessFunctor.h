#ifndef __KrcahSheetnessFunctor_h_
#define __KrcahSheetnessFunctor_h_

#include "itkUnaryFunctorImageFilter.h"
#include "vnl/vnl_math.h"
#include <algorithm>

namespace itk {
    namespace Functor {
        template<class TInputPixel, class TTracePixel, class TOutputPixel>
        class KrcahSheetness {
        public:
            KrcahSheetness() {
                // suggested values by Krcah el. al.
                m_Alpha = 0.5;
                m_Beta = 0.5;
                m_Gamma = 0.25;
            }

            inline TOutputPixel operator()(const TInputPixel &A, const TTracePixel T) {
                double sheetness = 0.0;
                double a1 = static_cast<double>( A[0] );
                double a2 = static_cast<double>( A[1] );
                double a3 = static_cast<double>( A[2] );
                double l1 = vnl_math_abs(a1);
                double l2 = vnl_math_abs(a2);
                double l3 = vnl_math_abs(a3);

                // Sort the values by their absolute value.
                // At the end of the sorting we should have
                // l1 <= l2 <= l3
                if (l1 > l2) {
                    std::swap(l1, l2);
                    std::swap(a1, a2);
                }
                if (l1 > l3) {
                    std::swap(l1, l3);
                    std::swap(a1, a3);
                }
                if (l2 > l3) {
                    std::swap(l2, l3);
                    std::swap(a2, a3);
                }

                // Avoid divisions by zero (or close to zero)
                if (static_cast<double>( l3 ) < vnl_math::eps || static_cast<double>( l2 ) < vnl_math::eps) {
                    return static_cast<TOutputPixel>( sheetness );
                }

                //const double T = l1 + l2 + l3; // http://en.wikipedia.org/wiki/Trace_%28linear_algebra%29#Eigenvalue_relationships
                const double Rsheet = l2 / l3;
                const double Rnoise = (l1 + l2 + l3) / T;
                const double Rtube = l1 / (l2 * l3);

                sheetness = (-vnl_math_sgn(a3));
                sheetness *= vcl_exp(-(Rsheet * Rsheet) / (m_Alpha * m_Alpha));
                sheetness *= vcl_exp(-(Rtube * Rtube) / (m_Beta * m_Beta));
                sheetness *= (1.0 - vcl_exp(-(Rnoise * Rnoise) / (m_Gamma * m_Gamma)));

                return static_cast<TOutputPixel>( sheetness );
            }

            void SetAlpha(double value) {
                this->m_Alpha = value;
            }

            void SetBeta(double value) {
                this->m_Beta = value;
            }

            void SetGamma(double value) {
                this->m_Gamma = value;
            }

        private:
            double m_Alpha;
            double m_Beta;
            double m_Gamma;
        };
    }
}

#endif // __KrcahSheetnessFunctor_h_