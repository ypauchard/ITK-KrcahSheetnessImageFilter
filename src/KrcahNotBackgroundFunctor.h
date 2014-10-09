#ifndef __KrcahNotBackgroundfunctor_h_
#define __KrcahNotBackgroundfunctor_h_

namespace itk {
    namespace Functor {
        template<class TThresholdPixel, class TSheetnessPixel, class TOutputPixel>
        class KrcahNotBackground {
        public:
            KrcahNotBackground() {
                m_lowerThreshold = 400;
                m_lowerSheetness = 0;
            }

            inline TOutputPixel operator()(const TThresholdPixel &T, const TSheetnessPixel S) {
                return static_cast<TOutputPixel>( T >= m_lowerThreshold && S > 0 ? 1 : 0 );
            }

        private:
            unsigned int m_lowerThreshold;
            unsigned int m_lowerSheetness;
        };
    }
}

#endif // __KrcahNotBackgroundfunctor_h_