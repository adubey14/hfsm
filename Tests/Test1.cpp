
#include "SMTest.h"
using namespace std;
using namespace STATEMACHINE;

using namespace SMTest_Behavior;

int mainTest()
{
	Repo* r = Repo::Instance();

	if (r)
	{
		cout<<"init step"<<endl;
		r->SM()->step();
		

		r->DS()->setCurrentTime();
		r->DS()->setVariable_FC(1);
		cout<<"step 2"<<endl;
		r->SM()->step();

	}

	

	Repo::Terminate();

	return 0;

}

int main()
{
	//return mainTest1();
	//return mainTest2();
	return mainTest();
}
