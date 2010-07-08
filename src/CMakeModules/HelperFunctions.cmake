# Append values to the list, ensuring that only one instance of each value
# exists within the list.  If the value already exists in the list, it is
# moved to the end of the list.
macro(unique_append TARGET)

  foreach(value ${ARGN})

    # Look for the item in the list
    list(FIND ${TARGET} ${value} index)

    # Remove the item if it was found
    if(NOT index EQUAL -1)
      list(REMOVE_AT ${TARGET} ${index})
    endif(NOT index EQUAL -1)

    # Append the item to the list
    list(APPEND ${TARGET} ${value})

  endforeach(value ${ARGN})

endmacro(unique_append TARGET)


# Append target string with another string only if the target string
# does not already contain the append string
macro(unique_append_string TARGET APPEND_STRING)
  if(NOT ${TARGET} MATCHES "${APPEND_STRING}[ ]" AND NOT ${TARGET} MATCHES "${APPEND_STRING}$")
    set(${TARGET} "${${TARGET}} ${APPEND_STRING}")
  endif(NOT ${TARGET} MATCHES "${APPEND_STRING}[ ]" AND NOT ${TARGET} MATCHES "${APPEND_STRING}$")
endmacro(unique_append_string TARGET APPEND_STRING)


# Remove all instances of REMOVE_STRING string from the TARGET string
macro(remove_all_string TARGET REMOVE_STRING)
  STRING(REGEX REPLACE "${REMOVE_STRING}[ ]+" "" ${TARGET} ${${TARGET}})
  STRING(REGEX REPLACE "${REMOVE_STRING}$" "" ${TARGET} ${${TARGET}})
endmacro(remove_all_string TARGET REMOVE_STRING)


# Disable Console for Release Modes (Release, MinSizeRel, ReleaseWithDebInfo)
macro(disable_win32_console_release EXENAME)
  if(WIN32)
    set(configs ${CMAKE_CONFIGURATION_TYPES})
    list(REMOVE_ITEM configs "Debug")
    foreach(value ${configs})
      string(TOUPPER ${value} config)
      set_target_properties(${EXENAME} PROPERTIES LINK_FLAGS_${config} "/SUBSYSTEM:WINDOWS /ENTRY:\"mainCRTStartup\"")
      endforeach(value ${configs})
  endif(WIN32)  
endmacro(disable_win32_console_release EXENAME)


# Enable the Console for Debug Mode
macro(enable_win32_console_debug EXENAME)
  if(WIN32)
    set_target_properties(${EXENAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
  endif(WIN32)
endmacro(enable_win32_console_debug EXENAME)


# Obtain the target of a symbolic link
# TARGET is an output that will be filled with the target's name if LINK
# is a symbolic link, and will be cleared/unset if LINK is a regular file
function(get_symlink_target LINK TARGET)
  execute_process(COMMAND "ls" "-l" "${LINK}" OUTPUT_VARIABLE LISTING)
  string(STRIP ${LISTING} LISTING)
  if(${LISTING} MATCHES "^l.*")
    # Obtain the target of the link
    string(REGEX REPLACE ".* -> (.*$)" "\\1" FILENAME ${LISTING})

    # "ls -l" displays target as relative to symbolic link's path
    # Extract the path from the symbolic link and add to the actual name
    string(REGEX MATCH ".*/" FILEPATH ${LINK})
    set(${TARGET} ${FILEPATH}${FILENAME} PARENT_SCOPE)
  else(${LISTING} MATCHES "^l.*")
    set(${TARGET} "" PARENT_SCOPE)
  endif(${LISTING} MATCHES "^l.*")
endfunction(get_symlink_target LINK TARGET)

