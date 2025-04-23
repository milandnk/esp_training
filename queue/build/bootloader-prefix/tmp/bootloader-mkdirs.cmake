# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/dnk120/esp/esp-idf-release-v5.3/components/bootloader/subproject"
  "/home/dnk120/Desktop/milan/trainning/esp32/esp trainning/queue/build/bootloader"
  "/home/dnk120/Desktop/milan/trainning/esp32/esp trainning/queue/build/bootloader-prefix"
  "/home/dnk120/Desktop/milan/trainning/esp32/esp trainning/queue/build/bootloader-prefix/tmp"
  "/home/dnk120/Desktop/milan/trainning/esp32/esp trainning/queue/build/bootloader-prefix/src/bootloader-stamp"
  "/home/dnk120/Desktop/milan/trainning/esp32/esp trainning/queue/build/bootloader-prefix/src"
  "/home/dnk120/Desktop/milan/trainning/esp32/esp trainning/queue/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/dnk120/Desktop/milan/trainning/esp32/esp trainning/queue/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/dnk120/Desktop/milan/trainning/esp32/esp trainning/queue/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
