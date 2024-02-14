#!/bin/bash

#    This file is part of darktable.
#    Copyright (C) 2016-2024 darktable developers.
#
#    darktable is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    darktable is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with darktable.  If not, see <http://www.gnu.org/licenses/>.

# This script is supposed to be run by Travis CI or GitHub workflow.
# It expects a few env variables to be set:
#   BUILD_DIR - the working directory where the program will be built
#   INSTALL_DIR - the installation prefix
#   SRC_DIR - directory with the source code to be compiled
#   CC, CXX, CFLAGS, CXXFLAGS are optional, but make sense for build
#   TARGET - either build or skiptest
#   ECO - some other flags for cmake

set -ex

if [ "$GENERATOR" = "Ninja" ];
then
  VERBOSE="-v"
  KEEPGOING="-k0"
  JOBS=""
fi;

if [ "$GENERATOR" = "Unix Makefiles" ];
then
  VERBOSE="VERBOSE=1";
  KEEPGOING="-k"
  JOBS="-j2"
fi;

if [ "$GENERATOR" = "MSYS Makefiles" ];
then
  VERBOSE="VERBOSE=1";
  KEEPGOING="-k"
  JOBS="-j2"
fi;

target_build()
{
  cmake --build "$BUILD_DIR" -- $JOBS "$VERBOSE" "$KEEPGOING"

  ctest --output-on-failure || ctest --rerun-failed -V -VV

  cmake --build "$BUILD_DIR" --target install -- $JOBS "$VERBOSE" "$KEEPGOING"
  
}

target_notest()
{
  cmake --build "$BUILD_DIR" -- $JOBS "$VERBOSE" "$KEEPGOING"

  cmake --build "$BUILD_DIR" --target install -- $JOBS "$VERBOSE" "$KEEPGOING"
}

diskspace()
{
  df
  du -hcs "$SRC_DIR"
  du -hcs "$BUILD_DIR"
  du -hcs "$INSTALL_PREFIX"
}

diskspace

cd "$BUILD_DIR"

case "$TARGET" in
  "build")
    cmake -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
      -G"$GENERATOR" \
      -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
      -DVALIDATE_APPDATA_FILE=ON \
      -DBUILD_TESTING=ON \
      -DTESTBUILD_OPENCL_PROGRAMS=ON \
      $ECO "$SRC_DIR" || (cat "$BUILD_DIR"/CMakeFiles/CMakeOutput.log; cat "$BUILD_DIR"/CMakeFiles/CMakeError.log)
    # target_build
    /Applications/Xcode_15.2.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc \-DAVIF_DLL \-DGDK_DISABLE_DEPRECATED \-DGDK_VERSION_MIN_REQUIRED=GDK_VERSION_3_24 \-DGLIB_VERSION_MAX_ALLOWED=GLIB_VERSION_MIN_REQUIRED \-DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_56 \-DGTK_DISABLE_DEPRECATED \-DGTK_DISABLE_SINGLE_INCLUDES \-DHAVE_APPLE_KEYCHAIN \-DHAVE_BUILTIN_CPU_SUPPORTS \-DHAVE_CONFIG_H \-DHAVE_GPHOTO2 \-DHAVE_ICU \-DHAVE_IMAGEMAGICK \-DHAVE_IMATH \-DHAVE_ISO_CODES \-DHAVE_KWALLET \-DHAVE_LIBAVIF=1 \-DHAVE_LIBEXIV2_WITH_ISOBMFF=1 \-DHAVE_LIBHEIF=1 \-DHAVE_LIBJXL \-DHAVE_LIBRAW=1 \-DHAVE_LIBSECRET \-DHAVE_LIBSHARPYUV=1 \-DHAVE_MAP \-DHAVE_OPENCL \-DHAVE_OPENEXR \-DHAVE_OPENJPEG \-DHAVE_OSMGPSMAP_110_OR_NEWER \-DHAVE_OSMGPSMAP_NEWER_THAN_110 \-DHAVE_P11KIT \-DHAVE_PRINT \-DHAVE_SQLITE_324_OR_NEWER \-DHAVE_VISIBILITY \-DHAVE_WEBP \-DLIBHEIF_EXPORTS \-DLIBRAW_NODLL \-DMAC_INTEGRATION \-DMAGICKCORE_HDRI_ENABLE=0 \-DMAGICKCORE_QUANTUM_DEPTH=16 \-DOS_OBJECT_USE_OBJC=0 \-DSQLITE_CORE \-DSQLITE_ENABLE_ICU \-DUSE_LUA \-D_XOPEN_SOURCE=700 \-Dlib_darktable_EXPORTS \-I/Users/runner/work/darktable/darktable/src/build/bin \-I/Users/runner/work/darktable/darktable/src/src \-I/Users/runner/work/darktable/darktable/src/src/external/LuaAutoC \-I/Users/runner/work/darktable/darktable/src/src/external/whereami/src \-I/Users/runner/work/darktable/darktable/src/src/external/LibRaw \-isystem \/Users/runner/work/darktable/darktable/src/src/external \-isystem \/Users/runner/work/darktable/darktable/src/src/external/OpenCL \-isystem \/usr/local/include \-isystem \/usr/local/include/glib-2.0 \-isystem \/usr/local/include/../lib/glib-2.0/include \-isystem \/usr/local/Cellar/gtk+3/3.24.41/include/gtk-3.0 \-isystem \/usr/local/Cellar/glib/2.78.4/include/gio-unix-2.0 \-isystem \/usr/local/Cellar/cairo/1.18.0/include \-isystem \/usr/local/Cellar/libepoxy/1.5.10/include \-isystem \/usr/local/Cellar/pango/1.50.14/include/pango-1.0 \-isystem \/usr/local/Cellar/harfbuzz/8.3.0/include/harfbuzz \-isystem \/usr/local/Cellar/fribidi/1.0.13/include/fribidi \-isystem \/usr/local/Cellar/graphite2/1.3.14/include \-isystem \/usr/local/Cellar/at-spi2-core/2.50.1/include/atk-1.0 \-isystem \/usr/local/Cellar/cairo/1.18.0/include/cairo \-isystem \/usr/local/Cellar/fontconfig/2.15.0/include \-isystem \/usr/local/opt/freetype/include/freetype2 \-isystem \/usr/local/Cellar/libxext/1.3.5/include \-isystem \/usr/local/Cellar/libxrender/0.9.11/include \-isystem \/usr/local/Cellar/libx11/1.8.7/include \-isystem \/usr/local/Cellar/libxcb/1.16/include \-isystem \/usr/local/Cellar/libxau/1.0.11/include \-isystem \/usr/local/Cellar/libxdmcp/1.1.4/include \-isystem \/usr/local/Cellar/pixman/0.42.2/include/pixman-1 \-isystem \/usr/local/Cellar/gdk-pixbuf/2.42.10_1/include/gdk-pixbuf-2.0 \-isystem \/usr/local/opt/libpng/include/libpng16 \-isystem \/usr/local/Cellar/libtiff/4.6.0/include \-isystem \/usr/local/opt/zstd/include \-isystem \/usr/local/Cellar/xz/5.4.5/include \-isystem \/usr/local/Cellar/jpeg-turbo/3.0.1/include \-isystem \/usr/local/Cellar/glib/2.78.4/include \-isystem \/usr/local/Cellar/glib/2.78.4/include/glib-2.0 \-isystem \/usr/local/Cellar/glib/2.78.4/lib/glib-2.0/include \-isystem \/usr/local/opt/gettext/include \-isystem \/usr/local/Cellar/pcre2/10.42/include \-isystem \/usr/local/Cellar/xorgproto/2023.2/include \-isystem \/Library/Developer/CommandLineTools/SDKs/MacOSX13.sdk/usr/include/ffi \-isystem \/Library/Developer/CommandLineTools/SDKs/MacOSX13.sdk/usr/include/libxml2 \-isystem \/usr/local/Cellar/lensfun/0.3.4/include/lensfun \-isystem \/usr/local/include/pango-1.0 \-isystem \/usr/local/include/librsvg-2.0 \-isystem \/usr/local/include/json-glib-1.0 \-isystem \/usr/local/Cellar/openjpeg/2.5.0_1/include/openjpeg-2.5 \-isystem \/usr/local/Cellar/libsecret/0.21.3/include/libsecret-1 \-isystem \/usr/local/Cellar/libgcrypt/1.10.3/include \-isystem \/usr/local/opt/libgpg-error/include \-isystem \/usr/local/include/gtkmacintegration \-isystem \/usr/local/Cellar/p11-kit/0.25.3/include/p11-kit-1 \-isystem \/usr/local/Cellar/imagemagick@6/6.9.13-6/include/ImageMagick-6 \-isystem \/usr/local/include/lua \-isystem \/usr/local/Cellar/osm-gps-map/1.2.0_2/include/osmgpsmap-1.0 \-isystem \/usr/local/Cellar/libsoup@2/2.74.2_1/include/libsoup-2.4 \-isystem \/usr/local/Cellar/libpsl/0.21.5/include \-isystem \/usr/local/Cellar/icu4c/73.2/include \-isystem \/usr/local/include/Imath \-isystem \/usr/local/include/OpenEXR \-isystem \/usr/local/opt/icu4c/include \-fopenmp-version=45 \-D_DARWIN_C_SOURCE \-Wall \-Wformat \-Wformat-security \-Wshadow \-Wtype-limits \-Wvla \-Wthread-safety \-Wno-unknown-pragmas \-Wno-error=varargs \-Wno-error=address-of-packed-member \-Xclang \-fopenmp \-mtune=generic \-msse2 \-g \-O3 \-DNDEBUG \-O3 \-ffast-math \-fno-finite-math-only \-std=c99 \-isysroot \/Applications/Xcode_15.2.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX14.2.sdk \-mmacosx-version-min=13.5 \-fPIC \ \ \-Werror \-Wfatal-errors \-Wno-error=pass-failed \-MD \-MT \bin/CMakeFiles/lib_darktable.dir/common/iop_profile.c.o \-MF \bin/CMakeFiles/lib_darktable.dir/common/iop_profile.c.o.d \-o \bin/CMakeFiles/lib_darktable.dir/common/iop_profile.c.o \-c \/Users/runner/work/darktable/darktable/src/src/common/iop_profile.c
    ;;
  "skiptest")
    cmake -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
      -G"$GENERATOR" \
      -DCMAKE_BUILD_TYPE="$CMAKE_BUILD_TYPE" \
      $ECO "$SRC_DIR" || (cat "$BUILD_DIR"/CMakeFiles/CMakeOutput.log; cat "$BUILD_DIR"/CMakeFiles/CMakeError.log)
    target_notest
    ;;
  *)
    exit 1
    ;;
esac

