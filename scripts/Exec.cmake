include_guard(GLOBAL)
# execute_process but nicer ig

include_guard(GLOBAL)

function(exec)
  execute_process(COMMAND ${ARGN} COMMAND_ERROR_IS_FATAL ANY)
endfunction()

function(execUnsafe)
  execute_process(COMMAND ${ARGN} RESULT_VARIABLE _)
endfunction()
