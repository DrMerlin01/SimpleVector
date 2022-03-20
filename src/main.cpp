#include "../inc/simple_vector.h"
#include "../inc/tests.h"

using namespace std;

int main() {
	Test1();
	Test2();
	TestReserveConstructor();
    TestReserveMethod();
	TestTemporaryObjConstructor();
	TestTemporaryObjOperator();
	TestNamedMoveConstructor();
	TestNamedMoveOperator();
	TestNoncopiableMoveConstructor();
	TestNoncopiablePushBack();
	TestNoncopiableInsert();
	TestNoncopiableErase();

	return 0;
}
