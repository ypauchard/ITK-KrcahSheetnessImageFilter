/*=========================================================================
Program: Insight Segmentation & Registration Toolkit
Module: itkDescoteauxSheetnessImageFilter.h
Language: C++
Date: $Date$
Version: $Revision$
Copyright (c) Insight Software Consortium. All rights reserved.
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the above copyright notices for more information.
=========================================================================*/
#ifndef __itkDescoteauxSheetnessImageFilterFemur_h
#define __itkDescoteauxSheetnessImageFilterFemur_h

#include "itkUnaryFunctorImageFilter.h"
#include "vnl/vnl_math.h"
#include <algorithm>

namespace itk {
/** \class DescoteauxSheetnessImageFilterFemur
*
* \brief Computes a measure of Sheetness from the Hessian Eigenvalues optimzed for usage in femur / pelvis segmentation.
*
* Based on the "Sheetness" measure proposed by Decouteaux et. al.
*
* M.Descoteaux, M.Audette, K.Chinzei, el al.:
* "Bone enhancement filtering: Application to sinus bone segmentation
* and simulation of pituitary surgery."
* In: MICCAI. (2005) 9–16
*
* and the work by Krcah et. al.
*
* M. Krcah, G. Szekely, R. Blanc:
* "Fully automatic and fast segmentation of the femur bone from 3d-CT images with no shape prior"
*/
    namespace Function {
        template<class TInput, class TOutput>
        class FemurSheetness {
        public:
            FemurSheetness() {
                // suggested values by Krcah el. al.
                m_Alpha = 0.5;
                m_Beta = 0.5;
                m_Gamma = 0.25;
                m_DetectBrightSheets = true;
            }

            ~FemurSheetness() {
            }

            bool operator!=(const FemurSheetness &) const {
                return false;
            }

            bool operator==(const FemurSheetness &other) const {
                return !(*this != other);
            }

            inline TOutput operator()(const TInput &A) {
                double sheetness = 0.0;
                double a1 = static_cast<double>( A[0] );
                double a2 = static_cast<double>( A[1] );
                double a3 = static_cast<double>( A[2] );
                double l1 = vnl_math_abs(a1);
                double l2 = vnl_math_abs(a2);
                double l3 = vnl_math_abs(a3);
//
// Sort the values by their absolute value.
// At the end of the sorting we should have
//
// l1 <= l2 <= l3
//
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

                if (this->m_DetectBrightSheets) {
                    if (a3 > 0.0) {
                        return static_cast<TOutput>( sheetness );
                    }
                }
                else {
                    if (a3 < 0.0) {
                        return static_cast<TOutput>( sheetness );
                    }
                }
//
// Avoid divisions by zero (or close to zero)
//
                if (static_cast<double>( l3 ) < vnl_math::eps || static_cast<double>( l2 ) < vnl_math::eps) {
                    return static_cast<TOutput>( sheetness );
                }

                const double T = l1 + l2 + l3; // http://en.wikipedia.org/wiki/Trace_%28linear_algebra%29#Eigenvalue_relationships
                const double Rsheet = l2 / l3;
                const double Rnoise = (l1 + l2 + l3) / T;
                const double Rtube = l1 / (l2 * l3);

                sheetness = (-vnl_math_sgn(a3));
                sheetness *= vcl_exp(-(Rsheet * Rsheet) / (m_Alpha * m_Alpha));
                sheetness *= vcl_exp(-(Rtube * Rtube) / (m_Beta * m_Beta));
                sheetness *= (1.0 - vcl_exp(-(Rnoise * Rnoise) / (m_Gamma * m_Gamma)));

                return static_cast<TOutput>( sheetness );
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

            void SetDetectBrightSheets(bool value) {
                this->m_DetectBrightSheets = value;
            }

            void SetDetectDarkSheets(bool value) {
                this->m_DetectBrightSheets = !value;
            }

        private:
            double m_Alpha;
            double m_Beta;
            double m_Gamma;
            bool m_DetectBrightSheets;
        };
    }
    template<class TInputImage, class TOutputImage>
    class ITK_EXPORT DescoteauxSheetnessImageFilterFemur :
            public UnaryFunctorImageFilter<TInputImage, TOutputImage,
                    Function::FemurSheetness<typename TInputImage::PixelType,
                            typename TOutputImage::PixelType> > {
    public:
/** Standard class typedefs. */
        typedef DescoteauxSheetnessImageFilterFemur Self;
        typedef UnaryFunctorImageFilter<
                TInputImage, TOutputImage,
                Function::FemurSheetness<
                        typename TInputImage::PixelType,
                        typename TOutputImage::PixelType> > Superclass;
        typedef SmartPointer<Self> Pointer;
        typedef SmartPointer<const Self> ConstPointer;
/** Method for creation through the object factory. */
        itkNewMacro(Self);
/** Runtime information support. */
        itkTypeMacro(DescoteauxSheetnessImageFilterFemur,
                UnaryFunctorImageFilter);

/** Set the normalization term for sheetness */
        void SetSheetnessNormalization(double value) {
            this->GetFunctor().SetAlpha(value);
        }

/** Set the normalization term for bloobiness. */
        void SetBloobinessNormalization(double value) {
            this->GetFunctor().SetBeta(value);
        }

/** Set the normalization term for noise. */
        void SetNoiseNormalization(double value) {
            this->GetFunctor().SetGamma(value);
        }

        void SetDetectBrightSheets(bool value) {
            this->GetFunctor().SetDetectBrightSheets(value);
        }

        void SetDetectDarkSheets(bool value) {
            this->GetFunctor().SetDetectDarkSheets(value);
        }

#ifdef ITK_USE_CONCEPT_CHECKING
/** Begin concept checking */
        typedef typename TInputImage::PixelType InputPixelType;
        itkConceptMacro(BracketOperatorsCheck,
                (Concept::BracketOperator<InputPixelType, unsigned int, double>));
        itkConceptMacro(DoubleConvertibleToOutputCheck,
                (Concept::Convertible<double, typename TOutputImage::PixelType>));
/** End concept checking */
#endif
    protected:
        DescoteauxSheetnessImageFilterFemur() {
        }

        virtual ~DescoteauxSheetnessImageFilterFemur() {
        }

    private:
        DescoteauxSheetnessImageFilterFemur(const Self &); //purposely not implemented
        void operator=(const Self &); //purposely not implemented
    };
} // end namespace itk
#endif