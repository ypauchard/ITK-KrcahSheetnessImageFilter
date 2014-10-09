#ifndef __KrcahBackgroundfunctor_h_
#define __KrcahBackgroundfunctor_h_

namespace itk {
    namespace Functor {
        template<class TThresholdPixel, class TSheetnessPixel, class TOutputPixel>
        class KrcahBackground {
        public:
            KrcahBackground() {
                m_lowerThreshold = 400;
                m_lowerSheetness = 0;
            }

            inline TOutputPixel operator()(const TThresholdPixel &T, const TSheetnessPixel S) {
                return static_cast<TOutputPixel>( T >= m_lowerThreshold && S > 0 ? 0 : 1 );
            }

        private:
            unsigned int m_lowerThreshold;
            unsigned int m_lowerSheetness;
        };
    }
}

#endif // __KrcahBackgroundfunctor_h_