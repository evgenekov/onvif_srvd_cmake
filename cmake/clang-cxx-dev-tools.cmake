# Adding clang-format target if executable is found
find_program(CLANG_FORMAT "clang-format")
if(CLANG_FORMAT)
  message("CLANG-FORMAT FOUND")
  add_custom_target(
    "clang-format-src"
    COMMAND ${CLANG_FORMAT}
    -i
    -verbose
    -style=file
    ${CLANG_TOOLS_FILES}
    )
else()
    message("CLANG-FORMAT NOT FOUND")
endif()

foreach( FILENAME ${CLANG_TOOLS_FILES} )
    get_filename_component( EXTENSION ${FILENAME} EXT )
    string( TOLOWER "${EXTENSION}" EXTENSION_LOWER )
    string( SUBSTRING "${EXTENSION_LOWER}" 1 1 EXTENSION_FIRST_LETTER )
    if ( "${EXTENSION_FIRST_LETTER}" STREQUAL "c")
        set( CLANG_TIDY_SRC_FILES ${CLANG_TIDY_SRC_FILES} ${FILENAME})
    endif()
endforeach( FILENAME )

# message( "Clang tidy files: " ${CLANG_TIDY_SRC_FILES} )

# Adding clang-tidy target if executable is found
find_program(CLANG_TIDY "run-clang-tidy")
if(CLANG_TIDY)
  message("CLANG-TIDY FOUND")
  add_custom_target(
    "clang-tidy-src"
    COMMAND ${CLANG_TIDY}
    -header-filter=${PROJECT_SOURCE_DIR}/src/
    ${CLANG_TIDY_SRC_FILES}
    -export-fixes clang-tidy-fixes.yaml
    )
else()
    message("CLANG-TIDY NOT FOUND")
endif()
