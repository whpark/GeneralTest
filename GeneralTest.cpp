// GeneralTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <catch2/catch_session.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

int main(int argc, char* argv[]) {
	Catch::Session session; // There must be exactly one instance
	session.applyCommandLine(std::max(1, argc), argv);
	return session.run();
}
