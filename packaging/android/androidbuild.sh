#!/bin/bash -e

# Example: androidbuild.sh -p=all --src=/home/sh_zam/workspace/krita --build-type=Release --build-root=/home/sh_zam/workspace/test-kreeta --ndk-path=/home/sh_zam/Android/Sdk/ndk-bundle --sdk-path=/home/sh_zam/Android/Sdk --api-level=21 --android-abi=armeabi-v7a --qt-path=/home/sh_zam/Qt/5.12.1/android_armv7

echoerr() { printf "ERROR: %s\n" "$*" >&2;  }

print_usage() {
    printf "\nUsage: "$0" [-p=PACKAGE] [ARGUMENTS..]\n"
    printf "Packages: [all|krita-bin|apk|qt|3rdparty|boost|kf5]\n"
    printf "Arguments: \n"
    printf  "\t--src=PATH                  Source files\n"
    printf  "\t--build-type=TYPE          TYPE=[Debug|RelWithDebInfo|Release]\n"
    printf  "\t--build-root=PATH          Path to build folder\n"
    printf  "\t--qt-path=PATH             Path to qt libs(optional)\n"
    printf  "\t--ndk-path=PATH            Android NDK root path\n"
    printf  "\t--sdk-path=PATH            Android SDK root path\n"
    printf  "\t--api-level=NUMBER         API level >= 21\n"
    printf  "\t--android-abi=ABI          ABI=[armeabi-v7a|arm64-v8a|x86|x86_64]\n"
}

# check if the argument is passed
check_exists() {
    if [[ ! -d ${!1} ]]; then
        echoerr "$1 not specified or does not exist"
        print_usage
        exit
    fi
}

setup_directories() {
    export DOWNLOADS_DIR=$BUILD_ROOT/d
    export DEPS_BUILD=$BUILD_ROOT/b
    export THIRDPARTY_INSTALL=$BUILD_ROOT/i

    if [[ ! -d $DOWNLOADS_DIR ]]; then
        mkdir $DOWNLOADS_DIR -p
    fi

    if [[ ! -d $DEPS_BUILD ]]; then
        mkdir $DEPS_BUILD -p
    fi

    if [[ ! -d $THIRDPARTY_INSTALL ]]; then
        mkdir $THIRDPARTY_INSTALL -p
    fi
}

configure_ext() {
    cd $DEPS_BUILD
    cmake $KRITA_ROOT/3rdparty                                                          \
        -DINSTALL_ROOT=$THIRDPARTY_INSTALL                                              \
        -DEXTERNALS_DOWNLOAD_DIR=$DOWNLOADS_DIR                                         \
        -DCMAKE_INSTALL_PREFIX=$THIRDPARTY_INSTALL                                      \
        -DCMAKE_TOOLCHAIN_FILE=$CMAKE_ANDROID_NDK/build/cmake/android.toolchain.cmake   \
        -DANDROID_ABI=$ANDROID_ABI                                                      \
        -DANDROID_PLATFORM=$ANDROID_NATIVE_API_LEVEL                                    \
        -DANDROID_SDK_ROOT=$ANDROID_SDK_ROOT
    cd $BUILD_ROOT
}

PROC_COUNT=`grep processor /proc/cpuinfo | wc -l`

build_qt() {
    if [[ ! -z $QT_ANDROID && -e $QT_ANDROID/lib/libQt5AndroidExtras.so ]]; then
        echo "Qt path provided; Skipping Qt build"
        return 0
    fi
    configure_ext
    cd $DEPS_BUILD
    cmake --build . --config $BUILD_TYPE --target ext_qt -- -j$PROC_COUNT
    cd $BUILD_ROOT
}

build_ext() {
    if [[ ! -d $QT_ANDROID ]]; then
        echoerr "qt libs not found"
        echo "Please run -p=qt prior to this"
        exit
    fi

    configure_ext
    cd $DEPS_BUILD
    # Please do not change the order
    cmake --build . --config $BUILD_TYPE --target ext_png -- -j$PROC_COUNT
    cmake --build . --config $BUILD_TYPE --target ext_zlib -- -j$PROC_COUNT
    cmake --build . --config $BUILD_TYPE --target ext_quazip -- -j$PROC_COUNT
    cmake --build . --config $BUILD_TYPE --target ext_lcms2 -- -j$PROC_COUNT
    cmake --build . --config $BUILD_TYPE --target ext_expat -- -j$PROC_COUNT
    cmake --build . --config $BUILD_TYPE --target ext_exiv2 -- -j$PROC_COUNT
    cmake --build . --config $BUILD_TYPE --target ext_gsl -- -j$PROC_COUNT
    cmake --build . --config $BUILD_TYPE --target ext_tiff -- -j$PROC_COUNT
    cmake --build . --config $BUILD_TYPE --target ext_fftw3 -- -j$PROC_COUNT
    cmake --build . --config $BUILD_TYPE --target ext_jpeg -- -j$PROC_COUNT
    cmake --build . --config $BUILD_TYPE --target ext_giflib -- -j$PROC_COUNT
    cmake --build . --config $BUILD_TYPE --target ext_eigen3 -- -j$PROC_COUNT

    cd $BUILD_ROOT
}

build_boost() {
    VERSION="1_69"
    if [[ ! -d $DOWNLOADS_DIR/boost ]]; then
        git clone https://github.com/moritz-wundke/Boost-for-Android $DOWNLOADS_DIR/boost
    fi

    cd $DOWNLOADS_DIR/boost
    ./build-android.sh --prefix=$THIRDPARTY_INSTALL --with-libraries=system \
        --boost=1.69.0 --arch=$ANDROID_ABI $CMAKE_ANDROID_NDK

    cd $THIRDPARTY_INSTALL/$ANDROID_ABI/lib

    # possible because just one library is being used
    mv libboost_system-*-$VERSION.a libboost_system.a

    cd $BUILD_ROOT
}

build_kf5() { 
    if [[ ! -d $QT_ANDROID ]]; then
        echoerr "qt libs not found"
        echo "Please run -p=qt prior to this"
        exit
    fi

    cd $BUILD_ROOT

    if [[ $ANDROID_ABI == "armeabi-v7a" ]]; then
        ANDROID_ARCHITECTURE=arm
    elif [[ $ANDROID_ABI == "arm64-v8a" ]]; then
        ANDROID_ARCHITECTURE=arm64
    elif [[ $ANDROID_ABI == "x86" || $ANDROID_ABI == "x86_64" ]]; then
        ANDROID_ARCHITECTURE=$ANDROID_ABI
    fi

    if [[ ! -d $BUILD_ROOT/kf5 ]]; then
        mkdir $BUILD_ROOT/kf5 -p
    fi 
    cd $BUILD_ROOT/kf5

    cp $KRITA_ROOT/packaging/android/kdesrc-buildrc $BUILD_ROOT/kf5/

    if [[ ! -d extragear/kdesrc-build ]]; then 
        mkdir -p extragear/kdesrc-build
        git clone http://invent.kde.org/sdk/kdesrc-build extragear/kdesrc-build
    fi
    if [[ ! -e  $BUILD_ROOT/kf5/kdesrc-build ]]; then 
        ln -s extragear/kdesrc-build/kdesrc-build kdesrc-build
    fi

    # Change the kdesrc-buildrc configuration
    sed -E -i "s|build-dir.*|build-dir $BUILD_ROOT/kf5/kde/build |g" $BUILD_ROOT/kf5/kdesrc-buildrc
    sed -E -i "s|source-dir.*|source-dir $BUILD_ROOT/kf5/kde/src |g" $BUILD_ROOT/kf5/kdesrc-buildrc
    sed -E -i "s|kdedir.*|kdedir $BUILD_ROOT/kf5/kde/install |g" $BUILD_ROOT/kf5/kdesrc-buildrc

    sed -E -i "s|cmake-options -DCMAKE_TOOLCHAIN_FILE=#replace-ecm#|cmake-options -DCMAKE_TOOLCHAIN_FILE=$CMAKE_ANDROID_NDK/build/cmake/android.toolchain.cmake|g" $BUILD_ROOT/kf5/kdesrc-buildrc
    # build first, so toolchain could be used
    $BUILD_ROOT/kf5/kdesrc-build --debug extra-cmake-modules

    if [[ -e $QT_ANDROID ]]; then
        sed -E -i "s|cmake-options -DCMAKE_TOOLCHAIN_FILE=#replace#|cmake-options -DCMAKE_PREFIX_PATH=$QT_ANDROID- -DCMAKE_ANDROID_NDK=$CMAKE_ANDROID_NDK -DECM_ADDITIONAL_FIND_ROOT_PATH=$QT_ANDROID\;$BUILD_ROOT/kf5/kde/install -DANDROID_STL=c++_static -DCMAKE_TOOLCHAIN_FILE=$BUILD_ROOT/kf5/kde/install/share/ECM/toolchain/Android.cmake -DKCONFIG_USE_DBUS=OFF -DANDROID_PLATFORM=$ANDROID_NATIVE_API_LEVEL -DANDROID_API_LEVEL=$ANDROID_API_LEVEL -DANDROID_ABI=$ANDROID_ABI -DANDROID_ARCHITECTURE=$ANDROID_ARCHITECTURE |g" $BUILD_ROOT/kf5/kdesrc-buildrc

        # add __ANDROID_API__ to cxxflags
        sed -E -i "s|cxxflags.*|cxxflags -D__ANDROID_API__=$ANDROID_API_LEVEL|g" $BUILD_ROOT/kf5/kdesrc-buildrc
    else
        echoerr "Qt Android libraries path doesn't exist. Exiting."
        exit
    fi

    sed -E -i "s|use-modules.+|use-modules kconfig ki18n |g" $BUILD_ROOT/kf5/kdesrc-buildrc
    rm -rf $BUILD_ROOT/kf5/kde/build/* # clean build folders

    # Please do not change the order
    ./kdesrc-build --debug libintl-lite
    ./kdesrc-build --debug kcoreaddons \
         kconfig ki18n                 \
         kwidgetsaddons kcompletion    \
         kguiaddons kitemmodels        \
         kitemviews kwindowsystem      

    cd $BUILD_ROOT
}

build_krita() {
    cd $BUILD_ROOT
    # Configure files using cmake
    cmake $KRITA_ROOT -DCMAKE_INSTALL_PREFIX=$INSTALL_PREFIX                                \
         -DDEFINE_NO_DEPRECATED=1                                                           \
         -DCMAKE_BUILD_TYPE=$BUILD_TYPE                                                     \
         -DCMAKE_TOOLCHAIN_FILE=$CMAKE_ANDROID_NDK/build/cmake/android.toolchain.cmake      \
         -DANDROID_PLATFORM=$ANDROID_NATIVE_API_LEVEL                                       \
         -DBUILD_TESTING=OFF -DKDE4_BUILD_TESTS=OFF                                         \
         -DBoost_NO_BOOST_CMAKE=TRUE                                                        \
         -DBoost_NO_SYSTEM_PATHS=TRUE                                                       \
         -DQTANDROID_EXPORTED_TARGET=krita                                                  \
         -DANDROID_APK_DIR=$KRITA_ROOT/packaging/android/apk                                \
         -DANDROID_STL=c++_shared                                                           \
         -DANDROID_ABI=$ANDROID_ABI                                                         \
         -DCMAKE_FIND_ROOT_PATH="$QT_ANDROID;$BUILD_ROOT/kf5/kde/install/;$BUILD_ROOT/i"

    make -j$PROC_COUNT install
}

build_apk() {
    cd $BUILD_ROOT
    if [[ $BUILD_TYPE == "Release" ]]; then
        make create-apk ARGS="--release"
    else
        make create-apk
    fi
}

# if no arguments are passed
if [[ "$#" == 0 ]]; then
    print_usage
    exit
fi

for i in "$@"
do
case $i in
    -p=*)
        PACKAGE="${i#*=}"
        shift
        ;;
    --src=*)
        export KRITA_ROOT="${i#*=}"
        shift
        ;;
    --build-type=*)
        export BUILD_TYPE="${i#*=}"
        ;;
    --build-root=*)
        export BUILD_ROOT="${i#*=}"
        shift
        ;;
    --qt-path=*)
        export QT_ANDROID="${i#*=}"
        shift
        ;;
    --ndk-path=*)
        export CMAKE_ANDROID_NDK="${i#*=}"
        shift
        ;;
    --sdk-path=*)
        export ANDROID_SDK_ROOT="${i#*=}"
        shift
        ;;
    --api-level=*)
        export ANDROID_API_LEVEL="${i#*=}"
        shift
        ;;
    --android-abi=*)
        export ANDROID_ABI="${i#*=}"
        shift
        ;;
    --help)
        print_usage
        exit
        ;;
esac
done

if [[ -z $ANDROID_ABI ]]; then
    echo "Warning: ANDROID_ABI not specified, using the default one: armeabi-v7a"
    export ANDROID_ABI=armeabi-v7a
fi

if [[ $ANDROID_ABI != "armeabi-v7a" && $ANDROID_ABI != "arm64-v8a" && \
    $ANDROID_ABI != "x86" && $ANDROID_ABI != "x86_64" ]]; then
    echoerr "Invalid ABI, please choose among: armeabi-v7a, arm64-v8a, x86, x86_64"
    echo "Exiting Now."
    exit
fi

if [[ -z $ANDROID_API_LEVEL ]]; then
    echo "Warning: ANDROID_API_LEVEL not set, using the default one: 21"
    export ANDROID_API_LEVEL=21
fi

if [[ -z $BUILD_ROOT ]]; then
    echoerr "Build root not specified"
    print_usage
elif [[ ! -d $BUILD_ROOT ]]; then
    mkdir $BUILD_ROOT -p
fi

check_exists CMAKE_ANDROID_NDK
check_exists ANDROID_SDK_ROOT
check_exists KRITA_ROOT

export ANDROID_NATIVE_API_LEVEL=android-$ANDROID_API_LEVEL
export INSTALL_PREFIX=$BUILD_ROOT/krita-android-build
if [[ -z $QT_ANDROID ]]; then
    export QT_ANDROID=$BUILD_ROOT/i
fi

setup_directories

case $PACKAGE in
    all)
        build_qt
        build_kf5
        build_ext
        build_boost
        build_krita
        build_apk
        ;;
    krita-bin)
        build_krita
        ;;
    apk)
        build_apk
        ;;
    qt)
        build_qt
        ;;
    3rdparty)
        build_ext
        ;;
    boost)
        build_boost
        ;;
    kf5)
        build_qt
        build_kf5
        ;;
    *)
        echoerr "Invalid package"
        print_usage
        ;;
esac

