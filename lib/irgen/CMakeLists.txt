project(irgen VERSION ${RUSH_VERSION} LANGUAGES CXX)

rush_add_library(irgen
   llvm/declarations.cpp
   llvm/expressions.cpp
   llvm/statements.cpp
   llvm/types.cpp
   llvm.cpp)

target_include_directories(irgen PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
	$<INSTALL_INTERFACE:../../include>
)

target_compile_features(irgen PUBLIC cxx_std_17)

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs core support codegen irreader)
# llvm_map_components_to_libnames(llvm_libs aggressiveinstcombine analysis binaryformat bitreader bitstreamreader bitwriter cfguard codegen core coverage debuginfocodeview debuginfodwarf debuginfogsym debuginfomsf debuginfopdb demangle dlltooldriver dwarflinker engine executionengine frontendopenmp fuzzmutate globalisel instcombine instrumentation interpreter ipo irreader libdriver lineeditor linker lto mc mca mcdisassembler mcparser nativecodegen objcarcopts object objectyaml option orcerror passes profiledata remarks runtimedyld scalaropts selectiondag support symbolize tablegen target textapi transformutils vectorize windowsmanifest xray)
target_link_libraries(irgen PUBLIC diag lexer parser ast ${llvm_libs})
