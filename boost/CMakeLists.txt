#	/boost-process/boost/process/posix/CMakeLists.txt
#	
#	@author zmij
#	@date Jan 15, 2016

cmake_minimum_required(VERSION 2.6)

set(ROOT_HDRS process.hpp)

set(PROCESS_HDRS
    process/all.hpp
    process/child.hpp
    process/config.hpp
    process/create_pipe.hpp
    process/execute.hpp
    process/executor.hpp
    process/initializers.hpp
    process/mitigate.hpp
    process/pipe.hpp
    process/search_path.hpp
    process/shell_path.hpp
    process/terminate.hpp
    process/wait_for_exit.hpp    
)

set(POSIX_HDRS
    process/posix/child.hpp
    process/posix/create_pipe.hpp
    process/posix/execute.hpp
    process/posix/executor.hpp
    process/posix/initializers.hpp
    process/posix/pipe.hpp
    process/posix/search_path.hpp
    process/posix/shell_path.hpp
    process/posix/terminate.hpp
    process/posix/wait_for_exit.hpp
)

set(POSIX_INITIALIZERS_HDRS
    process/posix/initializers/bind_fd.hpp
    process/posix/initializers/bind_stderr.hpp
    process/posix/initializers/bind_stdin.hpp
    process/posix/initializers/bind_stdout.hpp
    process/posix/initializers/close_fd.hpp
    process/posix/initializers/close_fds.hpp
    process/posix/initializers/close_fds_if.hpp
    process/posix/initializers/close_stderr.hpp
    process/posix/initializers/close_stdin.hpp
    process/posix/initializers/close_stdout.hpp
    process/posix/initializers/hide_console.hpp
    process/posix/initializers/inherit_env.hpp
    process/posix/initializers/initializer_base.hpp
    process/posix/initializers/notify_io_service.hpp
    process/posix/initializers/on_exec_error.hpp
    process/posix/initializers/on_exec_setup.hpp
    process/posix/initializers/on_fork_error.hpp
    process/posix/initializers/on_fork_setup.hpp
    process/posix/initializers/on_fork_success.hpp
    process/posix/initializers/run_exe.hpp
    process/posix/initializers/set_args.hpp
    process/posix/initializers/set_cmd_line.hpp
    process/posix/initializers/set_env.hpp
    process/posix/initializers/set_on_error.hpp
    process/posix/initializers/start_in_dir.hpp
    process/posix/initializers/throw_on_error.hpp
)

set(WINDOWS_HDRS
    process/windows/child.hpp
    process/windows/create_pipe.hpp
    process/windows/execute.hpp
    process/windows/executor.hpp
    process/windows/initializers.hpp
    process/windows/pipe.hpp
    process/windows/search_path.hpp
    process/windows/shell_path.hpp
    process/windows/terminate.hpp
    process/windows/wait_for_exit.hpp
)

set(WINDOWS_INITIALIZERS_HDRS
    process/windows/initializers/bind_stderr.hpp
    process/windows/initializers/bind_stdin.hpp
    process/windows/initializers/bind_stdout.hpp
    process/windows/initializers/close_stderr.hpp
    process/windows/initializers/close_stdin.hpp
    process/windows/initializers/close_stdout.hpp
    process/windows/initializers/hide_console.hpp
    process/windows/initializers/inherit_env.hpp
    process/windows/initializers/initializer_base.hpp
    process/windows/initializers/on_CreateProcess_error.hpp
    process/windows/initializers/on_CreateProcess_setup.hpp
    process/windows/initializers/on_CreateProcess_success.hpp
    process/windows/initializers/run_exe.hpp
    process/windows/initializers/set_args.hpp
    process/windows/initializers/set_cmd_line.hpp
    process/windows/initializers/set_env.hpp
    process/windows/initializers/set_on_error.hpp
    process/windows/initializers/show_window.hpp
    process/windows/initializers/start_in_dir.hpp
    process/windows/initializers/throw_on_error.hpp
)

install(
    FILES ${ROOT_HDRS}
    DESTINATION include/boost
)

install(
    FILES ${PROCESS_HDRS}
    DESTINATION include/boost/process
)

install(
    FILES ${POSIX_HDRS}
    DESTINATION include/boost/process/posix
)

install(
    FILES ${POSIX_INITIALIZERS_HDRS}
    DESTINATION include/boost/process/posix/initializers
)

install(
    FILES ${WINDOWS_HDRS}
    DESTINATION include/boost/process/windows
)

install(
    FILES ${WINDOWS_INITIALIZERS_HDRS}
    DESTINATION include/boost/process/windows/initializers
)

