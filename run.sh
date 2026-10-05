cmake -B build
cmake --build build || exit 1

# With arguments, render just those models
if [ $# -gt 0 ]; then
  time ./build/tinyrenderer "$@"
  exit
fi

# With no arguments, render every scenario, standing on the floor, to renders/<name>.tga
render() {
  name=$1
  shift
  echo "== $name"
  time ./build/tinyrenderer "$@" obj/floor.obj && cp renders/framebuffer.tga "renders/$name.tga"
}

render diablo obj/diablo3_pose/diablo3_pose.obj
render african_head obj/african_head/african_head.obj obj/african_head/african_head_eye_inner.obj
render boggie obj/boggie/body.obj obj/boggie/head.obj obj/boggie/eyes.obj
