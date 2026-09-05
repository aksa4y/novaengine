# Nova Engine architecture guard.
# This check is intentionally small and deterministic so it can run in CI
# without building third-party dependencies.

function(nova_assert_target_exists target_name)
    if(NOT TARGET ${target_name})
        message(FATAL_ERROR "Nova architecture target is missing: ${target_name}")
    endif()
endfunction()

function(nova_assert_target_does_not_exist target_name)
    if(TARGET ${target_name})
        message(FATAL_ERROR "Forbidden Nova dependency target exists: ${target_name}")
    endif()
endfunction()
