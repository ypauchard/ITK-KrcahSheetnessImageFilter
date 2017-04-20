## Description
Implementation of graphcut segmentation based on a sheetness image filter as described by Krcah et. al.

If you are interested in the original implementation it is now available [here](https://github.com/mkrcah/bone-segmentation)

## Usage
**Create the sheetness image, and foreground and background masks**

`./KrcahSheetness  /path/to/input /path/to/outputSheetness /path/to/background /path/to/foreground`

Example: 
```
cd src/GraphCut3D/data/test/left_femur
../../../../../bin/KrcahSheetness input.nrrd sheetness.nrrd sheetness_background.nrrd sheetness_foreground.nrrd
```

**Perform Graphcut**

`./KrcahGraphcut /path/to/outputSheetness /path/to/foreground /path/to/background /path/to/outputGraphcut sigma lambda `

Example: 
```
cd src/GraphCut3D/data/test/left_femur
../../../../../bin/KrcahGraphcut sheetness.nrrd sheetness_foreground.nrrd sheetness_background.nrrd sheetness_graphcut.nrrd 0.2 5.0
```

**Post-processing**

`./KrcahSplit /path/to/outputGraphcut /path/to/outputSplitObject erosion_radius intermediate_image_prefix(optional) `

Example: 
```
cd src/GraphCut3D/data/test/left_femur
../../../../../bin/KrcahSplit sheetness_graphcut.nrrd sheetness_graphcut_split.nrrd 3 sheetness
```

## Build Instruction
```
mkdir bin
cd bin
cmake ..
make
```

## Dependency
Tested with InsightToolkit-4.8.2 (ITK) and CMake 3.2.3 on MacOSx ElCapitan

## References
1. Marcel Krcah, Gabor Szekely, Remi Blanc: [Fully automatic and fast segmentation of the femur bon from 3D-CT images with no shape prior](https://www.vision.ee.ethz.ch/publications/papers/proceedings/eth_biwi_00818.pdf)
2. M.Descoteaux, M.Audette, K.Chinzei, el al.: [Bone enhancement filtering: application to sinus bone segmentation and simulation of pituitary surgery](http://www.cim.mcgill.ca/~shape/publications/miccai05b.pdf)

## License
- KrcahSheetness: BSD 3-Clause License (see LICENSE.txt)
- GraphCut3D: GPLv3 because of the used library [maxflow](https://pub.ist.ac.at/~vnk/software.html) (see GraphCut3D/LICENSE.txt)
