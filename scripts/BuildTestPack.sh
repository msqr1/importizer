set -e
mode=$1
os=$2
arch=$3
root=$(realpath "$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)/..")
buildDir="$root/build"
procCnt=$(cmake -P "$root/scripts/Nproc.cmake")
cmake -B "$buildDir" -DCMAKE_BUILD_TYPE=$mode --preset importizer-$os \
  -DCPACK_PACKAGE_FILE_NAME=importizer-$os-$arch
cmake --build "$buildDir" --config $mode
ctest --test-dir "$buildDir/tests" -C $mode --output-on-failure --progress \
  --schedule-random -j $procCnt
cpack --config "$buildDir/CPackConfig.cmake" -C $mode \
  -DCPACK_THREADS=$procCnt
