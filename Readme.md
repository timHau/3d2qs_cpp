# 3d2qs_cpp

## getting started
first you will need the [eigen lib](http://eigen.tuxfamily.org/index.php?title=Main_Page), which we use for all linear algebra operations.
We use `std::filesystem` so you should use at least C++17

```shell
cd ./include
git clone https://gitlab.com/libeigen/eigen.git
mv ./eigen ./eigen_bk
mv ./eigen_bk/Eigen .
rm -rf ./eigen_bk
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
