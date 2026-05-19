sudo apt install -y extrepo &&
sudo apt remove -y clang-* lldb-* lld-* clang-format-* clang-tidy-* --autoremove &&
sudo extrepo enable realsense &&
sudo apt update &&
sudo apt install -y libllvm20 llvm-20-dev llvm-20-tools libclang-common-20-dev libclang-20-dev libclang-cpp20-dev clang-tools-20
