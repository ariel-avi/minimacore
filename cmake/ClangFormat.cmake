function(add_clang_format_for TARGET_NAME)
    cmake_parse_arguments(FMT "" "" "FILES" ${ARGN})

    if (NOT DEFINED CLANG_FORMAT_EXE)
        find_program(CLANG_FORMAT_EXE NAMES clang-format clang-format-17 clang-format-16 clang-format-15)
    endif()
    if (NOT CLANG_FORMAT_EXE)
        message(WARNING "clang-format not found. 'format' targets will be skipped.")
        return()
    endif()

    set(_fmt_exts c;cc;cxx;cpp;h;hh;hpp)

    # Determine candidate files: FILES keyword > positional args > target SOURCES
    if (FMT_FILES)
        set(_candidate_files "${FMT_FILES}")
    elseif(FMT_UNPARSED_ARGUMENTS)
        set(_candidate_files "${FMT_UNPARSED_ARGUMENTS}")
    else()
        get_target_property(_candidate_files ${TARGET_NAME} SOURCES)
    endif()

    set(_fmt_files)
    foreach(f IN LISTS _candidate_files)
        get_filename_component(_abs "${f}" ABSOLUTE BASE_DIR "${CMAKE_SOURCE_DIR}")
        get_filename_component(_ext "${_abs}" EXT)
        string(REGEX REPLACE "^\\." "" _ext "${_ext}")
        list(FIND _fmt_exts "${_ext}" _idx)
        if (NOT _idx EQUAL -1)
            list(APPEND _fmt_files "${_abs}")
        endif()
    endforeach()

    if (NOT _fmt_files)
        return()
    endif()

    set(_format_target "clang-format_${TARGET_NAME}")
    add_custom_target(${_format_target}
            COMMAND ${CLANG_FORMAT_EXE} -i ${_fmt_files}
            COMMENT "Formatting sources for target ${TARGET_NAME}"
            VERBATIM)

    if (NOT TARGET clang-format_all)
        add_custom_target(clang-format_all COMMENT "Run clang-format (in-place) for all targets")
    endif()
    add_dependencies(clang-format_all ${_format_target})
endfunction()
