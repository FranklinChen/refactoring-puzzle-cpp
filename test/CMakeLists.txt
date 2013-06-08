add_executable(TestRefactoringPuzzle TestRefactoringPuzzle.cpp ../include/RefactoringPuzzle.h)
target_link_libraries(TestRefactoringPuzzle gtest)

add_test(test1 TestRefactoringPuzzle)
