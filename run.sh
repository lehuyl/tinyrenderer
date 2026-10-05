cmake -B build
cmake --build build
time ./build/tinyrenderer "${@:-obj/diablo3_pose/diablo3_pose.obj}"
