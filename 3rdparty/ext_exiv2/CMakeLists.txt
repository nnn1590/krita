SET(PREFIX_ext_exiv2  "${EXTPREFIX}" )

if (ANDROID)
    ExternalProject_Add( ext_exiv2
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/Exiv2/exiv2/archive/v0.27.2.tar.gz
        URL_MD5 15780152b1dfb5a0d2c0cdfe5f5a1588

        INSTALL_DIR ${PREFIX_ext_exiv2}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_exiv2} -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DEXIV2_BUILD_SAMPLES=OFF -DICONV_LIBRARY=${PREFIX_ext_exiv2}/lib -DEXIV2_ENABLE_NLS=OFF -DICONV_INCLUDE_DIR=${PREFIX_ext_exiv2}/include -DEXPAT_LIBRARY=$ENV{BUILD_ROOT}/i/lib/libexpat.so -DEXPAT_INCLUDE_DIR=$ENV{BUILD_ROOT}/i/include -DEXIV2_BUILD_EXIV2_COMMAND=OFF
        UPDATE_COMMAND ""
        INSTALL_COMMAND $(MAKE) install
        DEPENDS ext_expat
    )

elseif(MINGW)

    ExternalProject_Add( ext_exiv2
        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/Exiv2/exiv2/archive/v0.27.2.tar.gz
        URL_MD5 15780152b1dfb5a0d2c0cdfe5f5a1588

        PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/tzname.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/patch_mingw.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/disable_exiv_apps.diff
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/gcccheck.patch
        COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/01_fpermissive_error.patch
        
        INSTALL_DIR ${PREFIX_ext_exiv2}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_exiv2} -DCMAKE_LIBRARY_PATH=${PREFIX_ext_exiv2}/lib -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DEXIV2_BUILD_SAMPLES=OFF -DIconv_INCLUDE_DIR=${PREFIX_ext_exiv2}/include -DEXIV2_ENABLE_NLS=OFF -DEXIV2_BUILD_EXIV2_COMMAND=OFF

        UPDATE_COMMAND ""
        INSTALL_COMMAND $(MAKE) install
        DEPENDS ext_iconv ext_expat
    )
else()
    ExternalProject_Add( ext_exiv2

        DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
        URL https://github.com/Exiv2/exiv2/archive/v0.27.2.tar.gz
        URL_MD5 15780152b1dfb5a0d2c0cdfe5f5a1588

        INSTALL_DIR ${PREFIX_ext_exiv2}
        CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${PREFIX_ext_exiv2} -DCMAKE_LIBRARY_PATH=${PREFIX_ext_exiv2}/lib -DCMAKE_BUILD_TYPE=${GLOBAL_BUILD_TYPE} ${GLOBAL_PROFILE} -DEXIV2_BUILD_SAMPLES=OFF -DIconv_INCLUDE_DIR=${PREFIX_ext_exiv2}/include -DEXIV2_ENABLE_NLS=OFF -DEXIV2_BUILD_EXIV2_COMMAND=OFF

        UPDATE_COMMAND ""
        INSTALL_COMMAND $(MAKE) install
        DEPENDS ext_iconv ext_expat
    )
    
endif()
