project(ast VERSION ${RUSH_VERSION} LANGUAGES CXX)

rush_add_library(ast
   exprs/nil.cpp
   exprs/array.cpp
	exprs/literal.cpp
	exprs/binary.cpp
	exprs/unary.cpp
	exprs/invoke.cpp
	exprs/identifier.cpp
   types/builtin.cpp
   types/nominal.cpp
   types/type_matcher.cpp
   types/type_reducer.cpp
   ptrns/named.cpp
   ptrns/binding.cpp
   decls/storage.cpp
   decls/member.cpp
   decls/module.cpp
   decls/source.cpp
   decls/struct.cpp
   decls/class.cpp
   traversal.cpp
   iterator.cpp
   visitor.cpp
   context.cpp
   scope.cpp
   node.cpp
)

target_compile_features(ast PUBLIC cxx_std_17)


rush_render_templates(
   ${CMAKE_SOURCE_DIR}/include/rush/ast/stmts/_statements.hpp.j2
	${CMAKE_SOURCE_DIR}/include/rush/ast/exprs/_operators.hpp.j2
	${CMAKE_SOURCE_DIR}/include/rush/ast/exprs/_expressions.hpp.j2
	JSON ${CMAKE_SOURCE_DIR}/include/rush/ast/ast.json
	DEPENDENTS ast
)

target_include_directories(ast PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../../include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
	$<INSTALL_INTERFACE:../../include>
)

target_link_libraries(ast PUBLIC core)

