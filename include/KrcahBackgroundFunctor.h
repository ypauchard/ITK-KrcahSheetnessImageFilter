#ifndef __KrcahBackgroundfunctor_h_
#define __KrcahBackgroundfunctor_h_

namespace itk {
    namespace Functor {
        template<class TThresholdPixel, class TSheetnessPixel, class TOutputPixel>
        class KrcahBackground {
        public:
            KrcahBackground() {
            }

            inline TOutputPixel operator()(const TThresholdPixel T, const TSheetnessPixel S) {

                // 1 = pixel is NOT background, 0 = pixel is background
                if (T >= 400 && S > 0) {
                    return static_cast<TOutputPixel>(1);
                }
                return static_cast<TOutputPixel>(0);
            }
        };
    }
}

#endif // __KrcahBackgroundfunctor_h_