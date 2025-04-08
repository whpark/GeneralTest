module;

#include <catch.hpp>
#include <leveldb/db.h>

export module leveldb;

import std;

export {

	TEST_CASE("leveldb") {

		leveldb::DB* pdb{};
		leveldb::Options options;
		options.create_if_missing = true;
		auto status = leveldb::DB::Open(options, "testdb", &pdb);
		std::unique_ptr<leveldb::DB> db(pdb);
		REQUIRE(status.ok());
		leveldb::WriteOptions write_options;
		write_options.sync = true;
		leveldb::ReadOptions read_options;
		read_options.verify_checksums = true;
		std::string key = "key";
		std::string value = "value";
		status = db->Put(write_options, key, value);
		REQUIRE(status.ok());
		std::string retrieved_value;
		status = db->Get(read_options, key, &retrieved_value);
		REQUIRE(status.ok());
		REQUIRE(retrieved_value == value);

	}

}
