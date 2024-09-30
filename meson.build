project(
  'todo_app_raylib', 
  'c', 
  version: '0.1.0', 
  default_options: [
    'c_std=c23',
    'warning_level=2',
    'buildtype=debug',
    'optimization=0'
  ],
)

# Compiler options based on system type
if host_machine.system() == 'linux' or host_machine.system() == 'darwin' # macOS
    add_project_arguments([
        # '-Wpedantic',
        #'-Wno-implicit-function-declaration', 
    ], language: 'c')
elif host_machine.system() == 'windows'
    add_project_arguments(['/W4'], language: 'c')
endif

# Find the system-installed Raylib library
raylib_dep = dependency('raylib', required: true)

# Include directories
inc_dir = include_directories('includes')

# Linker arguments
linker_args = ['-lm']

# Define the executable
exe = executable(
    'todo_app_raylib', 
    'src/main.c',
    dependencies: raylib_dep,
    include_directories: inc_dir,
    link_args: linker_args,
    install: false,
)

