sudo apt remove -y clang-* lldb-* lld-* clang-format-* clang-tidy-* --autoremove &&
wget https://apt.llvm.org/llvm.sh &&
chmod u+x llvm.sh &&
sudo ./llvm.sh 22
sudo apt install -y libllvm22 llvm-22-dev llvm-22-tools libclang-common-22-dev libclang-22-dev libclang-cpp22-dev clang-tools-22
