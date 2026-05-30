mode=$1
os=$2
arch=$3
scriptDir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"
buildDir="$scriptDir/../build"

# Configure
cmake -B "$buildDir" -DCMAKE_BUILD_TYPE=$mode --preset importizer-$os \
  -DCPACK_PACKAGE_FILE_NAME=importizer-$os-$arch

# Build
cmake --build "$buildDir" --config $mode -j $(cmake -P "$scriptDir/Nproc.cmake")

# Test
ctest --test-dir "$buildDir/tests" -C $mode --output-on-failure --progress \
  --schedule-random -j $(cmake -P "$scriptDir/Nproc.cmake")

# Pack
cpack --config "$buildDir/CPackConfig.cmake" -C $mode \
  -DCPACK_THREADS=$(cmake -P "$scriptDir/Nproc.cmake")
