# Improved and simplified version of CMake's ExternalProject (https://cmake.org/cmake/help/v3.5/module/ExternalProject.html)
# This module provides a way to build external projects without including the cruft and complexity

# ExternalBuild_Add
#   TARGET          - Name of the target to create
#   SOURCE_DIR      - Absolute location of the source
#   BUILD_DIR_VAR   - Default working directory
function(ExternalBuild_Add TARGET)
    set(ONE_VALUE_ARGS SOURCE_DIR BUILD_DIR_VAR)
    cmake_parse_arguments(ARGS "" "${ONE_VALUE_ARGS}" "" ${ARGN})

    set(EB_DIR "${CMAKE_CURRENT_BINARY_DIR}/eb" CACHE INTERNAL "" FORCE)
    set(EB_CURRENT_TARGET "${TARGET}" CACHE INTERNAL "" FORCE)

    set(BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/build_eb")
    set(${ARGS_BUILD_DIR_VAR} "${BUILD_DIR}" PARENT_SCOPE)
    file(MAKE_DIRECTORY "${BUILD_DIR}")
    file(MAKE_DIRECTORY "${EB_DIR}")

    add_custom_target(
        ${TARGET}
        DEPENDS "eb-phony"
    )
    # Command with an output to hang steps off
    add_custom_command(
        OUTPUT "eb-phony"
        COMMAND ${CMAKE_COMMAND} -E echo ""
    )
endfunction()

# ExternalBuild_Add_Step
# Adds a step to the build similar to ExternalProject_Add_Step
# Output is logged so strings containing "error" will not cause the IDE to incorrectly think the build failed
#   NAME              - The name of the step
#   WORKING_DIRECTORY - Working directory for the COMMAND
#   COMMAND           - COMMAND to run, semi colons should use $<SEMICOLON> to avoid premature processing
#
function(ExternalBuild_Add_Step)
    set(ONE_VALUE_ARGS NAME WORKING_DIRECTORY)
    set(MULTI_VALUE_ARGS COMMAND)
    cmake_parse_arguments(ARGS "" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    if(NOT ARGS_WORKING_DIRECTORY)
        set(ARGS_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/build_eb")
    endif()

    set(STEP_STAMP "${EB_DIR}/${ARGS_NAME}-$<CONFIG>.success")
    set(STEP_SCRIPT "${EB_DIR}/${ARGS_NAME}-$<CONFIG>.cmake")
    set(STEP_SCRIPT_LOG "${STEP_SCRIPT}.log")
    set(STEP_SCRIPT_ERROR_LOG "${STEP_SCRIPT}.error.log")

    # Generate a script for the step so we can control logging
    set(ESCAPED_ARGS "${ARGS_COMMAND}")
    string(REPLACE "\"" "\\\"" ESCAPED_ARGS "${ARGS_COMMAND}")
    file(GENERATE
        OUTPUT "${STEP_SCRIPT}"
        CONTENT
"if(NOT EXISTS \"${STEP_STAMP}\")
    message(STATUS \"Running ${ARGS_NAME} of external build ${EB_CURRENT_TARGET}\")
    set(LIST_ARGS \"${ESCAPED_ARGS}\")
    execute_process(
        COMMAND \${LIST_ARGS}
        WORKING_DIRECTORY \"${ARGS_WORKING_DIRECTORY}\"
        OUTPUT_FILE \"${STEP_SCRIPT_LOG}\"
        ERROR_FILE \"${STEP_SCRIPT_ERROR_LOG}\"
        RESULT_VARIABLE ERROR_CODE
    )

    if(ERROR_CODE)
        message(FATAL_ERROR \"Failed with \${ERROR_CODE}, see ${STEP_SCRIPT_LOG} and ${STEP_SCRIPT_ERROR_LOG}\")
    endif()

    file(WRITE \"${STEP_STAMP}\" \":)\")
endif()
"
    )

    # Collect step stamp outputs so the IDE cleans them if it sees them
    set(STAMP_OUTPUTS)
    foreach(CONFIG IN LISTS CMAKE_CONFIGURATION_TYPES)
        list(APPEND STAMP_OUTPUTS "${EB_DIR}/${ARGS_NAME}-${CONFIG}.success")
    endforeach()

    add_custom_command(
        OUTPUT "eb-phony" ${STAMP_OUTPUTS}
        COMMAND ${CMAKE_COMMAND} -P "${STEP_SCRIPT}"
        APPEND
    )
endfunction()

