#include <iostream>
#include "ObjectSerializer.h"
#include <iostream>
#include "tests.h"


int main(int argc, char* argv[])
{
	ObjectSerializer::LibraryInfo::printInfo();

	std::cout << "Running "<< UnitTest::Test::getTests().size() << " tests...\n";
	UnitTest::Test::TestResults results;
	UnitTest::Test::runAllTests(results);
	UnitTest::Test::printResults(results);

	return results.getSuccess();
}