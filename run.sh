cmake -B build
cmake --build build
time ./build/tinyrenderer "${1:-obj/diablo3_pose/diablo3_pose.obj}"
