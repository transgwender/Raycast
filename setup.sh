mkdir -p build
cd build || exit
cmake ..
if make; then
  ./raycast
else
  echo "Build failed. Not running raycast."
fi
