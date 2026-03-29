function(add_clang_tidy_for TARGET_NAME)
    cmake_parse_arguments(FMT "" "" "FILES" ${ARGN})

    if (NOT TARGET ${TARGET_NAME})
        message(FATAL_ERROR "add_clang_tidy_for: target '${TARGET_NAME}' not found")
    endif ()

    if (NOT DEFINED CLANG_TIDY_EXE)
        find_program(CLANG_TIDY_EXE NAMES clang-tidy clang-tidy-18 clang-tidy-17 clang-tidy-16 clang-tidy-15)
    endif ()
    if (NOT CLANG_TIDY_EXE)
        message(WARNING "clang-tidy not found. 'clang-tidy' targets will be skipped.")
        return()
    endif ()

    # Determine candidate files: FILES keyword > positional args > target SOURCES
    if (FMT_FILES)
        set(_candidate_files "${FMT_FILES}")
    elseif(FMT_UNPARSED_ARGUMENTS)
        set(_candidate_files "${FMT_UNPARSED_ARGUMENTS}")
    else()
        get_target_property(_candidate_files ${TARGET_NAME} SOURCES)
    endif()

    set(_tidy_exts c;cc;cxx;cpp;h;hh;hpp)
    set(_tidy_files)
    foreach (f IN LISTS _candidate_files)
        get_filename_component(_abs "${f}" ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}")
        get_filename_component(_ext "${_abs}" EXT)
        string(REGEX REPLACE "^\\." "" _ext "${_ext}")
        list(FIND _tidy_exts "${_ext}" _idx)
        if (NOT _idx EQUAL -1)
            list(APPEND _tidy_files "${_abs}")
        endif ()
    endforeach ()

    if (NOT _tidy_files)
        return()
    endif ()

    # Per-target clang-tidy
    set(_tidy_target "clang-tidy_${TARGET_NAME}")

    set(_HEADER_FILTER "^(src|include|tests)/")

    # Collect include directories from the target and its dependencies
    set(_include_dirs)
    get_target_property(_inc_dirs ${TARGET_NAME} INTERFACE_INCLUDE_DIRECTORIES)
    if (_inc_dirs)
        list(APPEND _include_dirs ${_inc_dirs})
    endif()

    get_target_property(_link_libs ${TARGET_NAME} INTERFACE_LINK_LIBRARIES)
    if (_link_libs)
        foreach(_lib IN LISTS _link_libs)
            if (TARGET ${_lib})
                get_target_property(_lib_inc_dirs ${_lib} INTERFACE_INCLUDE_DIRECTORIES)
                if (_lib_inc_dirs)
                    list(APPEND _include_dirs ${_lib_inc_dirs})
                endif()
            endif()
        endforeach()
    endif()

    # Build extra args for include directories
    set(_extra_include_args)
    if (_include_dirs)
        list(REMOVE_DUPLICATES _include_dirs)
        foreach(_inc_dir IN LISTS _include_dirs)
            list(APPEND _extra_include_args "-extra-arg=-I${_inc_dir}")
        endforeach()
    endif()

    if (APPLE)
        execute_process(
                COMMAND xcrun --show-sdk-path
                OUTPUT_VARIABLE MACOS_SDK_PATH
                OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        if (NOT EXISTS "${MACOS_SDK_PATH}")
            message(FATAL_ERROR "Cannot determine macOS SDK path via xcrun.")
        endif ()

        set(APPLE_LIBCXX_INCLUDE "${MACOS_SDK_PATH}/usr/include/c++/v1")
        if (NOT EXISTS "${APPLE_LIBCXX_INCLUDE}")
            message(FATAL_ERROR "Apple libc++ include dir not found at ${APPLE_LIBCXX_INCLUDE}")
        endif ()
    endif ()

    if (APPLE)
        add_custom_target(${_tidy_target}
                COMMAND ${CLANG_TIDY_EXE}
                -header-filter=${_HEADER_FILTER}
                -extra-arg=-nostdinc++
                -extra-arg=-isysroot
                -extra-arg=${MACOS_SDK_PATH}
                -extra-arg=-isystem
                -extra-arg=${APPLE_LIBCXX_INCLUDE}
                -extra-arg=-fno-modules
                -extra-arg=-fno-implicit-modules
                -extra-arg=-fno-implicit-module-maps
                ${_extra_include_args}
                -p "${CMAKE_BINARY_DIR}" ${_tidy_files}
                COMMENT "Running clang-tidy for target ${TARGET_NAME}"
                VERBATIM)
    else ()
        add_custom_target(${_tidy_target}
                COMMAND ${CLANG_TIDY_EXE}
                -header-filter=${_HEADER_FILTER}
                ${_extra_include_args}
                -p "${CMAKE_BINARY_DIR}" ${_tidy_files}
                COMMENT "Running clang-tidy for target ${TARGET_NAME}"
                VERBATIM)
    endif ()

    if (NOT TARGET clang-tidy_all)
        add_custom_target(clang-tidy_all COMMENT "Run clang-tidy for all targets")
    endif ()
    add_dependencies(clang-tidy_all ${_tidy_target})
endfunction()