# This module provides function for joining paths
# known from from most languages
#
# Original license:
# SPDX-License-Identifier: (MIT OR CC0-1.0)
# Explicit permission given to distribute this module under
# the terms of the project as described in /LICENSE.rst.
# Copyright 2020 Jan Tojnar
# https:
#
# Modelled after Python’s os.path.join
# https:
# Windows not supported
function(join_paths joined_path first_path_segment)
    set(temp_path "${first_path_segment}")
    foreach(current_segment IN LISTS ARGN)
        if(NOT ("${current_segment}" STREQUAL ""))
            if(IS_ABSOLUTE "${current_segment}")
                set(temp_path "${current_segment}")
            else()
                set(temp_path "${temp_path}/${current_segment}")
            endif()
        endif()
    endforeach()
    set(${joined_path} "${temp_path}" PARENT_SCOPE)
endfunction()
