

//=== Cpp2 type declarations ====================================================


#include "cpp2util.h"

#line 1 "cppfront_test.cpp2"


//=== Cpp2 type definitions and function declarations ===========================

#line 1 "cppfront_test.cpp2"
#include "fmt/core.h"
#include "cppfront_test.h"

#line 4 "cppfront_test.cpp2"
auto PrintSomething() -> void;

//=== Cpp2 function definitions =================================================

#line 1 "cppfront_test.cpp2"

#line 4 "cppfront_test.cpp2"
auto PrintSomething() -> void{
 xStart var {}; 
 CPP2_UFCS(foo)(var);
 CPP2_UFCS(bar)(std::move(var));
}

