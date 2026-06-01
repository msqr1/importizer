set -e
mode=$1
os=$2
arch=$3
root=$(realpath "$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)/..")
buildDir="$root/build"
procCnt=$(nproc)

# Configure
cmake -B "$buildDir" -DCMAKE_BUILD_TYPE=$mode --preset importizer-$os \
  -DCPACK_PACKAGE_FILE_NAME=importizer-$os-$arch

# Build
cmake --build "$buildDir" --config $mode

# Test
ctest --test-dir "$buildDir/tests" -C $mode --verbose --progress --schedule-random \
  -j $procCnt

# Pack
cpack --config "$buildDir/CPackConfig.cmake" -C $mode \
  -DCPACK_THREADS=$procCnt
