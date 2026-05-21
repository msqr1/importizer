set -e
sudo apt remove -y clang-* lldb-* lld-* clang-format-* clang-tidy-* --autoremove
wget https://apt.llvm.org/llvm.sh
chmod u+x llvm.sh
ver="22"
sudo ./llvm.sh $ver
sudo apt install -y \
  llvm-${ver}-dev \
  libclang-common-${ver}-dev \
  libclang-${ver}-dev \
  libclang-cpp${ver}-dev \
  clang-tools-${ver}
