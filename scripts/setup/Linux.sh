set -e
wget https://apt.llvm.org/llvm.sh
chmod u+x llvm.sh
v="22"
sudo ./llvm.sh $v
sudo apt install -y \
  llvm-${v}-dev \
  libclang-common-${v}-dev \
  libclang-${v}-dev \
  libclang-cpp${v}-dev \
  clang-tools-${v}

echo m="$(gh api repos/msqr1/importizer/contents/CMakePresets.json \
  -H "Accept: application/vnd.github.raw" \
  -H "X-GitHub-Api-Version: 2026-03-10" | jq -c)"
