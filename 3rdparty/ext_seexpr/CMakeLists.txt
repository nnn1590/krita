SET(PREFIX_ext_seexpr "${EXTPREFIX}" )
ExternalProject_Add(
    ext_seexpr
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.kde.org/stable/kseexpr/4.0.0/kseexpr-4.0.0.0.tar.gz
    URL_MD5 52264980708826d4c38469d6571236e4

    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_seexpr} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} -DFLAVOR=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DENABLE_LLVM_BACKEND=OFF -DBUILD_DOC=OFF -DBUILD_TESTS=OFF -DBUILD_UTILS=OFF

    UPDATE_COMMAND ""
    DEPENDS ext_qt ext_extra_cmake_modules
)
