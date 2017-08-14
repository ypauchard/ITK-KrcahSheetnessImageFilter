#ifndef AutomaticSheetnessParameterEstimationImageFilter_h
#define AutomaticSheetnessParameterEstimationImageFilter_h

#include "FrobeniusNormImageFilter.h"
#include "itkLabelStatisticsImageFilter.h"

namespace itk
{

template <class TInputImage, class TLabelImage>
class ITK_EXPORT AutomaticSheetnessParameterEstimationImageFilter : public ImageToImageFilter<TInputImage, TInputImage> {
public:
  /** Standard class typedefs. */
  typedef AutomaticSheetnessParameterEstimationImageFilter    Self;
  typedef SmartPointer<Self>                Pointer;
  typedef SmartPointer<const Self>          ConstPointer;
  typedef typename TLabelImage::PixelType TLabelPixelType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Runtime information support. */
  itkTypeMacro(AutomaticSheetnessParameterEstimationImageFilter, ImageToImageFilter);

  itkSetMacro(Label,TLabelPixelType);
  itkGetMacro(Label,TLabelPixelType);

  itkSetMacro(Scale,double);
  itkGetMacro(Scale,double);

  itkGetMacro(Alpha,double);
  itkGetMacro(Beta,double);
  itkGetMacro(C,double);

  void SetLabelInput(const TLabelImage *input) {
    // Process object is not const-correct so the const casting is required.
    this->SetNthInput( 1, const_cast< TLabelImage * >( input ) );
  }

  const TLabelImage * GetLabelInput() const{
    return itkDynamicCastInDebugMode< TLabelImage * >( const_cast< DataObject * >( this->ProcessObject::GetInput(1) ) );
  }  

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  typedef typename TInputImage::PixelType InputPixelType;
  itkConceptMacro(BracketOperatorsCheck,
    (Concept::BracketOperator< InputPixelType, unsigned int, double >));
  itkConceptMacro(DoubleConvertibleToOutputCheck,
    (Concept::Convertible<double, typename TInputImage::PixelType>));
  /** End concept checking */
#endif

protected:
  AutomaticSheetnessParameterEstimationImageFilter();
  virtual ~AutomaticSheetnessParameterEstimationImageFilter();
  void GenerateData() ITK_OVERRIDE;
private:
  AutomaticSheetnessParameterEstimationImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  // Parameters
  double m_Alpha;
  double m_Beta;
  double m_C;
  double m_Scale;
  TLabelPixelType m_Label;

  // Filter types
  itkStaticConstMacro(NDimension, unsigned int, TInputImage::ImageDimension);
  typedef double FrobeniusImagePixelType;
  typedef Image<FrobeniusImagePixelType, NDimension> TFrobeniusOutputImage;

  typedef FrobeniusNormImageFilter<TInputImage, TFrobeniusOutputImage> FrobeniusNormImageFilterType;
  typedef LabelStatisticsImageFilter<TFrobeniusOutputImage, TLabelImage> LabelStatisticsImageFilterType;

}; // class AutomaticSheetnessParameterEstimationImageFilter

} // namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "AutomaticSheetnessParameterEstimationImageFilter.hxx"
#endif

#endif /* AutomaticSheetnessParameterEstimationImageFilter_h */