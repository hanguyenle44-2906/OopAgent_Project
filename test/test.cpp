#include <iostream>
#include <cassert>
#include "../src/agent/loop_detector.h"
#include "../src/harness/evaluator.h"

void test_loop_detector() {
    LoopDetector detector(3);
    
    // Test phát hiện vòng lặp
    assert(!detector.detectLoop("exec", "ls -la"));
    assert(!detector.detectLoop("exec", "ls -la"));
    assert(detector.detectLoop("exec", "ls -la") == true);
    
    std::cout << "[PASS] test_loop_detector successful!\n";
}

void test_keyword_evaluator() {
    KeywordEvaluator eval;
    
    // Test bộ chấm điểm từ khóa
    double score = eval.evaluate("The answer is 460", "460");
    assert(score == 1.0);
    
    std::cout << "[PASS] test_keyword_evaluator successful!\n";
}

int main() {
    std::cout << "=====================================\n";
    std::cout << "        RUNNING UNIT TESTS           \n";
    std::cout << "=====================================\n";

    test_loop_detector();
    test_keyword_evaluator();

    std::cout << "\n>>> ALL UNIT TESTS PASSED! <<<\n";
    return 0;
}