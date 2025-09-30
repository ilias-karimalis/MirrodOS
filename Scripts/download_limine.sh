#!/bin/bash 

git clone https://github.com/limine-bootloader/limine.git --branch=v9.x-binary --depth=1
cd limine
make CC="${HOST_CC}" CFLAGS="${HOST_CFLAGS}" CPPFLAGS="${HOST_CPPFLAGS}" LDFLAGS="${HOST_LDFLAGS}" LIBS="${HOST_LIBS}"
cd ..