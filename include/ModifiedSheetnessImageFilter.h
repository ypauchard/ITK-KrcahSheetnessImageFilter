#ifndef ModifiedSheetnessImageFilter_h
#define ModifiedSheetnessImageFilter_h

#include "itkUnaryFunctorImageFilter.h"
#include "vnl/vnl_math.h"

namespace itk {

namespace Functor {  
  
template< class TInput, class TOutput>
class ModifiedSheetness {
public:
  ModifiedSheetness() {
    m_Alpha = 0.5;              // Suggested value from Vesselness paper
    m_C     = 1.0;              // Should be tuned from data
    m_DetectBrightSheets = -1;  // Detect bright sheets is default
  }
  ~ModifiedSheetness() {}
  bool operator!=( const ModifiedSheetness & ) const {
    return false;
  }

  bool operator==( const ModifiedSheetness & other ) const {
    return !(*this != other);
  }

  inline TOutput operator()( const TInput & A ) {
    double sheetness = 0.0;

    // Get eigenvalues
    double a1 = static_cast<double>( A[0] );
    double a2 = static_cast<double>( A[1] );
    double a3 = static_cast<double>( A[2] );

    double l1 = vnl_math_abs( a1 );
    double l2 = vnl_math_abs( a2 );
    double l3 = vnl_math_abs( a3 );

    // Sort the values by their absolute value.
    //  l1 <= l2 <= l3
    if( l2 > l3 ) {
      double tmpl = l3;
      l3 = l2;
      l2 = tmpl;
      double tmpa = a3;
      a3 = a2;
      a2 = tmpa;
    }

    if( l1 > l2 ) {
      double tmp = l1;
      l1 = l2;
      l2 = tmp;
      double tmpa = a1;
      a1 = a2;
      a2 = tmpa;
    }   

    if( l2 > l3 ){
      double tmp = l3;
      l3 = l2;
      l2 = tmp;
      double tmpa = a3;
      a3 = a2;
      a2 = tmpa;
    }

    // Avoid divisions by zero (or close to zero)
    if( static_cast<double>( l3 ) < vnl_math::eps ) {
      // If l3 approx. 0, Rs -> inf, sheetness -> 0
      return static_cast<TOutput>( sheetness );
    }
    
    const double Rt = l1 / vcl_sqrt(l2*l2 + l3*l3);
    const double Rn = vcl_sqrt( l3*l3 + l2*l2 + l1*l1 );

    // Calculate sheetness
    sheetness  =         m_DetectBrightSheets * (a3 / l3);
    sheetness *=         vcl_exp( - ( Rt * Rt ) / ( 2.0 * m_Alpha  * m_Alpha  ) ); 
    sheetness *= ( 1.0 - vcl_exp( - ( Rn * Rn ) / ( 2.0 * m_C     * m_C     ) ) ); 

    return static_cast<TOutput>( sheetness );
  }

  void SetAlpha( double value ) {
    this->m_Alpha = value;
  }

  void SetC( double value ) {
    this->m_C = value;
  }

  void DetectBrightSheetsOn() {
    this->m_DetectBrightSheets = -1;
  }

  void DetectDarkSheetsOn() {
    this->m_DetectBrightSheets = 1;
  }

private:
  double    m_Alpha;
  double    m_C;
  double    m_DetectBrightSheets;
}; // class ModifiedSheetness
} // namespace Functor

template <class TInputImage, class TOutputImage>
class ITK_EXPORT ModifiedSheetnessImageFilter :
    public
UnaryFunctorImageFilter<TInputImage,TOutputImage, 
Functor::ModifiedSheetness< typename TInputImage::PixelType, 
                                       typename TOutputImage::PixelType>   >
{
public:
  /** Standard class typedefs. */
  typedef ModifiedSheetnessImageFilter    Self;
  typedef UnaryFunctorImageFilter<
    TInputImage,TOutputImage, 
    Functor::ModifiedSheetness< 
      typename TInputImage::PixelType, 
      typename TOutputImage::PixelType> >   Superclass;
  typedef SmartPointer<Self>                Pointer;
  typedef SmartPointer<const Self>          ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Runtime information support. */
  itkTypeMacro(ModifiedSheetnessImageFilter, 
               UnaryFunctorImageFilter);

  /** Set the normalization term for sheetness */
  void SetNormalization( double value ) {
    this->GetFunctor().SetAlpha( value );
  }

  /** Set the normalization term for noise. */
  void SetNoiseNormalization( double value ) {
    this->GetFunctor().SetC( value );
  }
  void DetectBrightSheetsOn() {
    this->GetFunctor().DetectBrightSheetsOn();
  }
  void DetectDarkSheetsOn() {
    this->GetFunctor().DetectDarkSheetsOn();
  }

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  typedef typename TInputImage::PixelType InputPixelType;
  itkConceptMacro(BracketOperatorsCheck,
    (Concept::BracketOperator< InputPixelType, unsigned int, double >));
  itkConceptMacro(DoubleConvertibleToOutputCheck,
    (Concept::Convertible<double, typename TOutputImage::PixelType>));
  /** End concept checking */
#endif

protected:
  ModifiedSheetnessImageFilter() {}
  virtual ~ModifiedSheetnessImageFilter() {}

private:
  ModifiedSheetnessImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
}; // class ModifiedSheetnessImageFilter


} // end namespace itk

#endif /* ModifiedSheetnessImageFilter_h */
