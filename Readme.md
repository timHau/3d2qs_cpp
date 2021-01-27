# 3d2qs_cpp

## getting started
first you will need the [eigen lib](http://eigen.tuxfamily.org/index.php?title=Main_Page), which we use for all linear algebra operations.

```shell
cd ./include
git clone https://gitlab.com/libeigen/eigen.git
mv ./eigen/Eigen .
rm -rf ./eigen
```

then place all the datasets into the data folder. For matterport we need
- region_segmentations 

For the SUNC data we need:
- house/<house id>/house.json
- object/
- room/<house id>/

## dependencies
- place eigen into include folder 
- add house / object folder into data folder
- add region_segmentations into data folder
- https://github.com/Bly7/OBJ-Loader
- https://www.boost.org/
- https://github.com/nlohmann/json
- https://gitlab.com/libeigen/eigen.git
- https://github.com/skystrife/cpptoml
- https://github.com/nmwsharp/happly
