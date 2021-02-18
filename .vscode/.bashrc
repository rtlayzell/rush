alias test='pushd . > /dev/null && cd ~/src/rush-lang/rush/build && ctest --output-on-failure ; popd > /dev/null'
alias build='pushd . > /dev/null && cd ~/src/rush-lang/rush/build && cmake -DCMAKE_INSTALL_PREFIX=/opt/rush ../ && cmake --build . -j 8 ; popd > /dev/null'
alias install='pushd . > /dev/null && cd ~/src/rush-lang/rush/build && sudo cmake --build . -j 8 --target install ; popd > /dev/null'
alias rebuild='rm -rf ~/src/rush-lang/rush/build && mkdir ~/src/rush-lang/rush/build && build'

alias lex='~/src/rush-lang/rush/build/crush --dump-lex ~/src/rush-lang/rush/main.rush'
alias prs='~/src/rush-lang/rush/build/crush --dump-parse ~/src/rush-lang/rush/main.rush'
alias cmp='~/src/rush-lang/rush/build/crush --dump-llvm-ir ~/src/rush-lang/rush/main.rush'

alias rush='function _run_crush_lli(){ /home/rtlayzell/src/rush-lang/rush/build/crush $* > /home/rtlayzell/src/rush-lang/rush/out.ll && lli /home/rtlayzell/src/rush-lang/rush/out.ll && echo success!; rm /home/rtlayzell/src/rush-lang/rush/out.ll; };_run_crush_lli'

export CC=clang
export CXX=clang++
export CMAKE_PREFIX_PATH=/opt/llvm-10
export PATH=$PATH:/usr/lib/llvm-10/bin
