cmake_minimum_required(VERSION 3.18)

# プロジェクト名と使用言語(C言語)の指定
project(multitexture LANGUAGES C)

# C99標準を使用
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# OpenGL パッケージの検索
find_package(OpenGL REQUIRED)

if(WIN32)
    # Windows: OpenGL Registry (Khronos) から glext.h と KHR/khrplatform.h を自動ダウンロード
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
        # khrplatform.h は EGL 側の領域で管理・共有されている場合があるため、EGLのパスを指定
        file(DOWNLOAD "https://registry.khronos.org/EGL/api/KHR/khrplatform.h" "${KHR_H_FILE}" SHOW_PROGRESS)
    endif()

    # Windows: まずローカルのGLUT(freeglut等)を検索
    find_package(GLUT QUIET)
    if(NOT GLUT_FOUND)
        message(STATUS "GLUT not found. Downloading precompiled freeglut binaries...")
        # 公式の NuGet リポジトリにある nupengl.core (freeglut 等を含むパッケージ) からバイナリをダウンロード
        set(GLUT_ZIP "${CMAKE_BINARY_DIR}/nupengl.core.nupkg")
        set(GLUT_EXTRACT_DIR "${CMAKE_BINARY_DIR}/freeglut-bin")
        
        if(NOT EXISTS "${GLUT_EXTRACT_DIR}")
            file(DOWNLOAD "https://globalcdn.nuget.org/packages/nupengl.core.0.1.0.1.nupkg" "${GLUT_ZIP}" SHOW_PROGRESS)
            file(ARCHIVE_EXTRACT INPUT "${GLUT_ZIP}" DESTINATION "${GLUT_EXTRACT_DIR}")
            file(DOWNLOAD "https://globalcdn.nuget.org/packages/nupengl.core.redist.0.1.0.1.nupkg" "${CMAKE_BINARY_DIR}/nupengl.core.redist.nupkg" SHOW_PROGRESS)
            file(ARCHIVE_EXTRACT INPUT "${CMAKE_BINARY_DIR}/nupengl.core.redist.nupkg" DESTINATION "${GLUT_EXTRACT_DIR}")
        endif()
        
        # zip内ディレクトリ構造 (ビルド済バイナリがある配置)
        set(FREEGLUT_ROOT "${GLUT_EXTRACT_DIR}/build/native")
        
        # main.c で <GL/glut.h> と "glut.h" の両方に対応できるようインクルードパスを設定
        set(GLUT_INCLUDE_DIR "${FREEGLUT_ROOT}/include" "${FREEGLUT_ROOT}/include/GL")
        
        # 32bit / 64bit を判定してライブラリとDLLパスを決定
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(GLUT_LIBRARIES "${FREEGLUT_ROOT}/lib/x64/freeglut.lib")
            set(FREEGLUT_DLL "${FREEGLUT_ROOT}/bin/x64/freeglut.dll")
        else()
            set(GLUT_LIBRARIES "${FREEGLUT_ROOT}/lib/Win32/freeglut.lib")
            set(FREEGLUT_DLL "${FREEGLUT_ROOT}/bin/Win32/freeglut.dll")
        endif()
    endif()
else()
    if(UNIX AND NOT APPLE)
        # Ubuntu Linux: pkg-config を使用して freeglut3-dev の情報を取得
        # ディストリビューションにより .pc 名が freeglut / glut で異なるため両方を試す
        find_package(PkgConfig REQUIRED)
        pkg_search_module(FREEGLUT REQUIRED IMPORTED_TARGET freeglut glut)
    else()
        # macOS: 標準付属の GLUT.framework を使用
        find_package(GLUT REQUIRED)
    endif()
endif()

# 実行ファイルの定義
add_executable(${PROJECT_NAME} main.c)

# 実行時に main.c から texture*.raw を確実に読み込めるよう、
# 実行ファイル出力先へテクスチャファイルを自動コピーする
set(TEXTURE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/texture0.raw"
    "${CMAKE_CURRENT_SOURCE_DIR}/texture1.raw"
)

foreach(TEXTURE_FILE IN LISTS TEXTURE_FILES)
    if(EXISTS "${TEXTURE_FILE}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${TEXTURE_FILE}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMENT "Copying ${TEXTURE_FILE} to output directory"
        )
    endif()
endforeach()

# インクルードパスとライブラリの設定
if(WIN32)
    target_include_directories(${PROJECT_NAME} PRIVATE 
        ${OPENGL_INCLUDE_DIR}
        ${GLUT_INCLUDE_DIR}
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        ${OPENGL_LIBRARIES}
        ${GLUT_LIBRARIES}
    )
elseif(UNIX AND NOT APPLE)
    # Ubuntu Linux: pkg-config から取得した情報を使用
    target_include_directories(${PROJECT_NAME} PRIVATE ${OPENGL_INCLUDE_DIR})

    # glActiveTextureARB などのシンボル解決のため OpenGL/GLU を明示リンクする
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        PkgConfig::FREEGLUT
        OpenGL::GL
        OpenGL::GLU
    )
else()
    # macOS
    target_include_directories(${PROJECT_NAME} PRIVATE 
        ${OPENGL_INCLUDE_DIR}
        ${GLUT_INCLUDE_DIR}
    )
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        ${OPENGL_LIBRARIES}
        ${GLUT_LIBRARIES}
    )
endif()

# プラットフォームごとの固有設定
if(WIN32)
    # Windows (Visual Studio用)
    target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32)
    # initmultitexture.h などのローカルヘッダ用と、ダウンロードした glext.h 用のパス
    target_include_directories(${PROJECT_NAME} PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GLEXT_H_DIR}
    )
    
    # ダウンロードした freeglut.dll を実行ファイルと同じ出力ディレクトリにコピー（実行時に必要）
    if(DEFINED FREEGLUT_DLL AND EXISTS "${FREEGLUT_DLL}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${FREEGLUT_DLL}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMENT "Copying freeglut.dll to output directory"
        )
    endif()
elseif(APPLE)
    # macOS (Xcode用)
    # 特に追加の設定は不要
elseif(UNIX AND NOT APPLE)
    # Ubuntu Linux (Makefile用)
    # Ubuntu では mathライブラリ (libm) の明示的なリンクが必要
    target_link_libraries(${PROJECT_NAME} PRIVATE m)
endif()

# Visual Studio のソリューションでこのプロジェクトをデフォルトのスタートアッププロジェクトに設定
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# デバッグ時に texture0.raw 等を読み込めるよう、作業ディレクトリをプロジェクトルート(ソース側)に指定
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

