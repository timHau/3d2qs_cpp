# 3d2qs_cpp

## getting started
first you will need the [eigen lib](http://eigen.tuxfamily.org/index.php?title=Main_Page), which we use for all linear algebra operations.

```shell
cd ./include
git clone https://gitlab.com/libeigen/eigen.git
mv ./eigen/Eigen .
rm -rf ./eigen
```

## data folder
```
data/
├── 3d_objects_debug/
│  ├── matterport3d/
│  └── sunc/
│ 
├── matterport3d/
│  ├── .gitignore
│  ├── config/
│  └── region_segmentations/
│ 
└── sunc
   ├── .gitignore
   ├── config/
   ├── house/
   └── object/
```

## dependencies
- place eigen into include folder 
- add house / object folder into data folder
- add region_segmentations into data folder
- https://github.com/nlohmann/json
- https://gitlab.com/libeigen/eigen.git
- https://github.com/skystrife/cpptoml
- https://github.com/nmwsharp/happly
