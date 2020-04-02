set(SDL2_INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/sdl2/include" CACHE INTERNAL "SDL2 include directory")
set(SDL2_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/sdl2/lib/x64/" CACHE INTERNAL "SDL2 lib directory")
set(SDL2_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/sdl2/lib/x64/SDL2.lib;${CMAKE_CURRENT_BINARY_DIR}/sdl2/lib/x64/SDL2main.lib" CACHE INTERNAL "SDL2 lib files")
set(SDL2_RUNTIME_LIBRARIES "${CMAKE_CURRENT_BINARY_DIR}/sdl2/lib/x64/SDL2.dll" CACHE INTERNAL "SDL2 runtime lib files")