# OHQ-GA

Build and usage instructions for both headless runners -- including the full
codegen `--kernel` workflow -- are in one place:

    ../OHQ-Common/README.md

Quick start:

    mkdir -p build && cd build
    qmake6 ../OHQ-GA.pro
    make -j$(nproc)
