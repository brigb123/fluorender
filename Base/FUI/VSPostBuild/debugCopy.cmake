add_custom_command(TARGET FUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${GLEW_DLL_DIR}/glew32d.dll"
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug"
	)

add_custom_command(TARGET FUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${Teem_DIR}/teem.dll"
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug"
	)

add_custom_command(TARGET FUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${TIFF_LIBRARY}/tiffd.dll"
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug"
	)

add_custom_command(TARGET FUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${Zlib_ROOT_DIRECTORY}/Debug/zlibd.dll"
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug"
	)

add_custom_command(TARGET FUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${PNG_ROOT_DIR}/Debug/libpng16d.dll"
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug"
	)

add_custom_command(TARGET FUI POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
	"${pole_LIBRARY_DIR}/Poledump.dll"
	"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug"
	)