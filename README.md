## Description
Implementation of a sheetness image filter as described by Krcah et. al.

## Usage
`./Testbench /path/to/input /path/to/output`

Example: `./Testbench ../data/pelvis.mhd ../data/out.mhd`

## Build Instruction
```
mkdir bin
cd bin
cmake ..
make
```

## References
1. Marcel Krcah, Gabor Szekely, Remi Blanc: [Fully automatic and fast segmentation of the femur bon from 3D-CT images with no shape prior](https://www.vision.ee.ethz.ch/publications/papers/proceedings/eth_biwi_00818.pdf)
2. M.Descoteaux, M.Audette, K.Chinzei, el al.: [Bone enhancement filtering: application to sinus bone segmentation and simulation of pituitary surgery](http://www.cim.mcgill.ca/~shape/publications/miccai05b.pdf)
