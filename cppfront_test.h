
#ifndef CPPFRONT_TEST_H_CPP2
#define CPPFRONT_TEST_H_CPP2


//=== Cpp2 type declarations ====================================================


#include "cpp2util.h"

#line 1 "cppfront_test.h2"

#line 4 "cppfront_test.h2"
class xStart;
 

//=== Cpp2 type definitions and function declarations ===========================

#line 1 "cppfront_test.h2"
#include <fmt/core.h>
#include <fmt/xchar.h>

#line 4 "cppfront_test.h2"
class xStart {
 public: static auto foo() -> void;

#line 8 "cppfront_test.h2"
 public: static auto bar() -> void;
 public: xStart() = default;
 public: xStart(xStart const&) = delete; /* No 'that' constructor, suppress copy */
 public: auto operator=(xStart const&) -> void = delete;


#line 11 "cppfront_test.h2"
};


//=== Cpp2 function definitions =================================================

#line 1 "cppfront_test.h2"

#line 5 "cppfront_test.h2"
 auto xStart::foo() -> void{
  fmt::print("cppfront: foo\n");
 }
#line 8 "cppfront_test.h2"
 auto xStart::bar() -> void{
  fmt::print("cppfront: bar\n");
 }
#endif

