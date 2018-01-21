# - Provide a macro to embed binary files into the executable.
# The module defines the following variables:
#
#  The module defines the macros:
#
#  EMBED_TARGET(<Name> <BinFile>)
#
# which will create  a custom rule to a assembly file. <BinFile> is
# the path to the binary file.
#
# The macro defines a set of variables:
#  EMBED_${Name}_DEFINED       - true is the macro ran successfully
#  EMBED_${Name}_INPUT         - The input source file, an alias for <BinFile>
#  EMBED_${Name}_OUTPUT        - The source file generated
#
#  ====================================================================
#  Example:
#
#   find_package(Embed)
#   EMBED_TARGET(mysource source.glsl)
#   add_executable(Foo main.cc ${EMBED_mysource_OUTPUTS})
#  ====================================================================

cmake_minimum_required(VERSION 3.0)
enable_language(ASM)

macro(EMBED_TARGET Name Input)
	get_filename_component(InputAbs "${Input}" REALPATH BASE_DIR "${CMAKE_SOURCE_DIR}")
	set(PTR "quad")
	if(CMAKE_SIZEOF_VOID_P EQUAL 4)
		set(PTR "long")
	endif()
	set(ASM ".section .rodata\n.align ${CMAKE_SIZEOF_VOID_P}\ndata: .incbin \"${InputAbs}\"\n.globl ${Name}\n${Name}:\n.${PTR} data\n.long ${Name} - data")
	set(Output "${CMAKE_CURRENT_BINARY_DIR}/${Name}.S")
	file(WRITE ${Output} ${ASM})

	add_custom_command(
		OUTPUT ${Output}
		COMMAND ${CMAKE_COMMAND} -E touch ${Output}
		DEPENDS ${Input}
	)
	set(EMBED_${Name}_DEFINED TRUE)
	set(EMBED_${Name}_INPUT ${Input})
	set(EMBED_${Name}_OUTPUT ${Output})
endmacro()
