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

    # Collect include directories from the target and its dependencies (recursively)
    set(_include_dirs)
    set(_targets_to_process ${TARGET_NAME})
    set(_visited_targets)

    while(_targets_to_process)
        list(POP_FRONT _targets_to_process _current_target)

        # Skip if already visited
        if (_current_target IN_LIST _visited_targets)
            continue()
        endif()

        # Try to resolve alias-style names (e.g., namespace::target -> namespace_target)
        if (NOT TARGET ${_current_target})
            string(REPLACE "::" "_" _alt_target "${_current_target}")
            if (TARGET ${_alt_target})
                set(_current_target ${_alt_target})
            else()
                continue()
            endif()
        endif()

        # Resolve alias targets to their actual targets
        get_target_property(_aliased_target ${_current_target} ALIASED_TARGET)
        if (_aliased_target)
            set(_current_target ${_aliased_target})
            # Check again if we've already visited the aliased target
            if (_current_target IN_LIST _visited_targets)
                continue()
            endif()
        endif()

        list(APPEND _visited_targets ${_current_target})

        # Get include directories from this target
        get_target_property(_inc_dirs ${_current_target} INTERFACE_INCLUDE_DIRECTORIES)
        if (_inc_dirs AND NOT _inc_dirs STREQUAL "_inc_dirs-NOTFOUND")
            list(APPEND _include_dirs ${_inc_dirs})
        endif()

        # Add linked libraries to the process queue
        get_target_property(_link_libs ${_current_target} INTERFACE_LINK_LIBRARIES)
        if (_link_libs AND NOT _link_libs STREQUAL "_link_libs-NOTFOUND")
            list(APPEND _targets_to_process ${_link_libs})
        endif()
    endwhile()

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