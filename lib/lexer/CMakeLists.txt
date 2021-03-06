project(lexer VERSION ${RUSH_VERSION} LANGUAGES CXX)


rush_add_library(lexer
	${CMAKE_CURRENT_BINARY_DIR}/_symbols.cpp
	${CMAKE_CURRENT_BINARY_DIR}/_keywords.cpp
	token.cpp
	lexer.cpp
	dump.cpp
)


rush_render_templates(
	${CMAKE_SOURCE_DIR}/include/rush/lexer/_tokens.hpp.j2
	${CMAKE_SOURCE_DIR}/include/rush/lexer/_symbols.hpp.j2
	${CMAKE_SOURCE_DIR}/include/rush/lexer/_keywords.hpp.j2
	${CMAKE_SOURCE_DIR}/lib/lexer/_symbols.cpp.j2
	${CMAKE_SOURCE_DIR}/lib/lexer/_keywords.cpp.j2
	JSON ${CMAKE_SOURCE_DIR}/include/rush/lexer/tokens.json
	DEPENDENTS lexer
)

target_compile_features(lexer PUBLIC cxx_std_17)

target_include_directories(lexer PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../../include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
	$<INSTALL_INTERFACE:../../include>
)

target_link_libraries(lexer PUBLIC core)
