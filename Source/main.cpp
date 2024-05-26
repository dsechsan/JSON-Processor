#include <iostream>
#include <fstream>
#include <functional>
#include <map>
#include "JSONParser.h"
#include "AutoGrader.h"
#include <memory>
#include <sstream>
#include "Testable.h"
#include "Debug.h"
//#include <gtest.h>
// STUDENT: Be sure to update this path if necessary (should point to the repo directory)
inline std::string getWorkingDirectoryPath() {
    return "/Users/dsechs/Library/CloudStorage/OneDrive-UCSanDiego/Desktop/ECE 141A/PA3";
}

bool runAutoGraderTest(const std::string& aPath, const std::string& aTestName) {
    ECE141::AutoGrader autoGrader(aPath);
    return autoGrader.runTest(aTestName);
}

bool runNoFilterTest(const std::string& aPath) {
    return runAutoGraderTest(aPath, "NoFilterTest");
}

bool runBasicTest(const std::string& aPath) {
    return runAutoGraderTest(aPath, "BasicTest");
}

bool runAdvancedTest(const std::string& aPath) {
    return runAutoGraderTest(aPath, "AdvancedTest");
}

int runTest(const int argc, const char* argv[]) {
    const std::string thePath = argc > 2 ? argv[2] : getWorkingDirectoryPath();
    const std::string theTest = argv[1];

    std::map<std::string, std::function<bool(const std::string&)>> theTestFunctions {
        { "compile", [](const std::string&) { return true; } },
        { "nofilter", runNoFilterTest },
        { "query", ECE141::runModelQueryTest },
        { "basic", runBasicTest },
        { "advanced", runAdvancedTest }
    };

    if (theTestFunctions.count(theTest) == 0) {
        std::clog << "Unknown test '" << theTest << "'\n";
        return 1;
    }

    const bool hasPassed = theTestFunctions[theTest](thePath);
    std::cout << "Test '" << theTest << "' " << (hasPassed ? "PASSED" : "FAILED") << "\n";
    return !hasPassed;
}

int main(const int argc, const char* argv[]) {
    if (argc > 1)
        return runTest(argc, argv);

    // Add your testing code here!
    using ModelNode = ECE141::ModelNode;
    ModelNode myNode;
//    myNode.value = ModelNode::ObjectType{{"key", ModelNode{5.0}}};

    std::string filepath = "/Users/dsechs/Library/CloudStorage/OneDrive-UCSanDiego/Desktop/ECE 141A/PA3/Resources/sammy.json";
    std::ifstream inputFile(filepath);

    ECE141::JSONParser jsonParser(inputFile);
    ECE141::Model model;
    jsonParser.parse(&model);


    auto theQuery1 = model.createQuery();
    auto theResult1 = theQuery1.select("'sammy'.'followers'");

//    if(theResult) std::cout << *theResult << "\n";


    ECE141::Model model2(model);
    auto model3 = model2;


    auto theQuery2 = model.createQuery();
    auto theResult2 = theQuery2.select("'list'").filter("index < 2").get("*");

    if (theResult2) std::cout << *theResult2 << "\n";

    std::string aPath = "/Users/dsechs/Library/CloudStorage/OneDrive-UCSanDiego/Desktop/ECE 141A/PA3";
    std::string aTestName = "NoFilterTest";

    runAutoGraderTest(aPath,aTestName);


}