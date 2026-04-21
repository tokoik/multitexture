cmake_minimum_required(VERSION 3.15)

project(multitexture C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(SOURCES "main.c")
set(TEXTURE_FILES "texture0.raw" "texture1.raw")

if(APPLE)
    # CMakeから直接 Info.plist のテンプレートを生成する(macOS専用)
    set(INFO_PLIST_CONTENT "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">
<plist version=\"1.0\">
<dict>
	<key>CFBundleDevelopmentRegion</key>
	<string>English</string>
	<key>CFBundleExecutable</key>
	<string>\${EXECUTABLE_NAME}</string>
	<key>CFBundleIdentifier</key>
	<string>com.yourcompany.\${PRODUCT_NAME:identifier}</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundlePackageType</key>
	<string>APPL</string>
	<key>CFBundleSignature</key>
	<string>????</string>
	<key>CFBundleVersion</key>
	<string>1.0</string>
	<key>CFBundleIconFile</key>
	<string>opengl.icns</string>
</dict>
</plist>
")
    file(WRITE "${CMAKE_BINARY_DIR}/Info.plist.in" "${INFO_PLIST_CONTENT}")

    set(RESOURCE_FILES "opengl.icns")

    # macOS: アプリケーションバンドルを作成
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${TEXTURE_FILES} ${RESOURCE_FILES})

    # テクスチャ等をバンドルの Resources ディレクトリに配置
    set_source_files_properties(${TEXTURE_FILES} ${RESOURCE_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    
    # 生成した Info.plist.in を指定
    set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_BINARY_DIR}/Info.plist.in")

    find_library(OPENGL_FRAMEWORK OpenGL REQUIRED)
    find_library(GLUT_FRAMEWORK GLUT REQUIRED)

    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${OPENGL_FRAMEWORK}
        ${GLUT_FRAMEWORK}
    )

    # XcodeでRunした時の作業ディレクトリをResourcesに設定し、テクスチャを正しく読み込めるようにする
    set_target_properties(${PROJECT_NAME} PROPERTIES
        XCODE_SCHEME_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Resources"
    )

elseif(WIN32)
    # Windows
    add_executable(${PROJECT_NAME} ${SOURCES})

    # プログラム名をスタートアッププロジェクトに設定
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

    # glext.h と khrplatform.h のダウンロード
    set(GLEXT_H_DIR "${CMAKE_BINARY_DIR}/opengl_registry")
    set(GLEXT_H_FILE "${GLEXT_H_DIR}/glext.h")
    if(NOT EXISTS "${GLEXT_H_FILE}")
        message(STATUS "Downloading glext.h from OpenGL Registry...")
        file(DOWNLOAD "https://registry.khronos.org/OpenGL/api/GL/glext.h" "${GLEXT_H_FILE}" SHOW_PROGRESS)
    endif()

    set(KHR_DIR "${GLEXT_H_DIR}/KHR")
    set(KHR_H_FILE "${KHR_DIR}/khrplatform.h")
    if(NOT EXISTS "${KHR_H_FILE}")
        message(STATUS "Downloading khrplatform.h from OpenGL Registry...")
        file(DOWNLOAD "https://registry.khronos.org/EGL/api/KHR/khrplatform.h" "${KHR_H_FILE}" SHOW_PROGRESS)
    endif()

    # freeglut をビルドするための設定
    set(FREEGLUT_BUILD_DEMOS OFF CACHE BOOL "Do not build freeglut demos" FORCE)
    set(FREEGLUT_BUILD_SHARED_LIBS ON CACHE BOOL "Build freeglut as shared library" FORCE)
    set(FREEGLUT_BUILD_STATIC_LIBS OFF CACHE BOOL "Do not build freeglut static library" FORCE)

    # freeglut のソースを GitHub からダウンロードしてビルド
    include(FetchContent)
    FetchContent_Declare(
        freeglut
        GIT_REPOSITORY "https://github.com/freeglut/freeglut.git"
        GIT_TAG "master"
    )
    FetchContent_MakeAvailable(freeglut)

    target_include_directories(${PROJECT_NAME} PRIVATE
        "${freeglut_SOURCE_DIR}/include"
        "${freeglut_SOURCE_DIR}/include/GL"
        "${GLEXT_H_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE freeglut opengl32.lib glu32.lib)

    # DLLとテクスチャファイルを実行ディレクトリへコピー
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:freeglut> $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/texture0.raw" $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/texture1.raw" $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )

    # Visual Studioのデバッガを直接実行ファイルに設定するために、ワーキングディレクトリを実行ファイルの場所にセットする
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

elseif(UNIX)
    # Ubuntu Linux等のUNIX環境
    add_executable(${PROJECT_NAME} ${SOURCES})

    # pkg-config を使ってコンパイル・リンクのオプションを決定
    find_package(PkgConfig REQUIRED)
    pkg_search_module(PKG_GLUT REQUIRED glut freeglut)
    pkg_search_module(PKG_GL REQUIRED gl)
    pkg_search_module(PKG_GLU REQUIRED glu)

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${PKG_GLUT_INCLUDE_DIRS}
        ${PKG_GL_INCLUDE_DIRS}
        ${PKG_GLU_INCLUDE_DIRS}
    )
    target_link_directories(${PROJECT_NAME} PRIVATE
        ${PKG_GLUT_LIBRARY_DIRS}
        ${PKG_GL_LIBRARY_DIRS}
        ${PKG_GLU_LIBRARY_DIRS}
    )
    target_compile_options(${PROJECT_NAME} PRIVATE
        ${PKG_GLUT_CFLAGS_OTHER}
        ${PKG_GL_CFLAGS_OTHER}
        ${PKG_GLU_CFLAGS_OTHER}
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${PKG_GLUT_LIBRARIES}
        ${PKG_GL_LIBRARIES}
        ${PKG_GLU_LIBRARIES}
        m
    )

    # テクスチャを実行ファイルのディレクトリにコピー
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/texture0.raw" $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/texture1.raw" $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()
