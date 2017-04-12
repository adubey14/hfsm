#include "SMTest.h"

using namespace SMTest_Behavior;

Repo* SMTest_Behavior::Repo::_pInstance = 0;



SMTest_Behavior::DataSource::DataSource()
{
	clearlists();
	Init();
}

SMTest_Behavior::DataSource::~DataSource()
{
	clearlists();
	Destroy();
}


//Execute state entry-action 
void SMTest_Behavior::DataSource::performStateEntryAction(STATEID sid)
{

	
	return;

}

//Execute state exit-action		
void SMTest_Behavior::DataSource::performStateExitAction(STATEID sid)
{
	return;

} 

//Execute state during-action
void SMTest_Behavior::DataSource::performStateDuringAction(STATEID sid)
{
	return;

}

//Execute default action in a state when there are no active outgoing transitions
void SMTest_Behavior::DataSource::performDefaultStateAction(STATEID sid)
{
	return;

}
		


// Execute transitions -action
void SMTest_Behavior::DataSource::performTransitionAction(TRANSID tid)
{
	switch(tid)
	{
		case 1:	return TA_T1();
		case 2:	return TA_T2();
		case 3:	return TA_T3();
		case 4:	return TA_T4();
		default: break;
	}
	return;
} 

bool SMTest_Behavior::DataSource::evalTransitionGuard(TRANSID tid) //Evaluate transitions - guard
{

	switch(tid)
	{
		case 1:	return TG_T1();
		case 2:	return TG_T2();
		case 3:	return TG_T3();
		case 4:	return TG_T4();
	
		default: break;
	}
	return true;
}

bool SMTest_Behavior::DataSource::evalTransitionEventTrigger(TRANSID tid) //Evaluate transitions - trigger
{
	return true;
}

		
bool SMTest_Behavior::DataSource::TG_T1()
{
	bool ret =((FC==Accelerator1));
	return ret;
}
bool SMTest_Behavior::DataSource::TG_T2()
{
	bool ret =((FC==Accelerator1));
	return ret;
}
bool SMTest_Behavior::DataSource::TG_T3()
{
	bool ret =((FC==Accelerator2));
	return ret;
}
bool SMTest_Behavior::DataSource::TG_T4()
{
	bool ret =((FC==Accelerator2));
	return ret;
}
void SMTest_Behavior::DataSource::TA_T1()
{
	RESPONSE(RESETALL,Accelerator1);;
}
void SMTest_Behavior::DataSource::TA_T2()
{
	RESPONSE(STOPALL,Accelerator1);;
}
void SMTest_Behavior::DataSource::TA_T3()
{
	RESPONSE(STOPALL,Accelerator2);;
}
void SMTest_Behavior::DataSource::TA_T4()
{
	RESPONSE(RESETALL,Accelerator2);;
}



//Execute default event action
void SMTest_Behavior::DataSource::checkAndPerformDefaultEventAction()
{

}


//Method to Set Variable FC
void SMTest_Behavior::DataSource::setVariable_FC(int FC_in)
{
	FC = FC_in;
}

//Method to Get Variable FC
int SMTest_Behavior::DataSource::getVariable_FC()
{
	return FC;
}


void STEP_CompBehavior()
{
	Repo::Instance()->SM()->step();
}

Repo* Repo::Instance()
{
	if (!Repo::_pInstance)
		Repo::_pInstance = new Repo();
	return Repo::_pInstance;
}

void Repo::Terminate()
{
	delete _pInstance;
}

Repo::Repo()
{
	build();
}

Repo::~Repo()
{
	delete _pStateMachine;
	delete _pDataSource;
}

StateMachine* Repo::SM()
{
	return _pStateMachine;
}

SMTest_Behavior::DataSource* Repo::DS()
{
	return _pDataSource;
}


void Repo::build()
{
	_pDataSource = new SMTest_Behavior::DataSource();
		
	
	_pStateMachine = new STATEMACHINE::StateMachine("CompBehavior", _pDataSource);
	_pStateMachine->addRegion("R_CompBehavior",0,0);
	_pStateMachine->addState(0,0,"MachineState",STATEMACHINE::AND,1,false,true);
	
	_pStateMachine->addRegion("R_MachineState_MANAGE_A1",0,1);
	_pStateMachine->addState(0,1,"MANAGE_A1",STATEMACHINE::OR,2,false,true);
	_pStateMachine->addRegion("R_MANAGE_A1",0,2);
	_pStateMachine->addState(0,2,"Nominal",STATEMACHINE::OR,3,false,true);
	_pStateMachine->addRegion("R_Nominal",0,3);
	_pStateMachine->addState(0,2,"STOP",STATEMACHINE::OR,4);
	_pStateMachine->addRegion("R_STOP",0,4);
	_pStateMachine->addState(0,2,"Reset",STATEMACHINE::OR,5);
	_pStateMachine->addRegion("R_Reset",0,5);

	_pStateMachine->addRegion("R_MachineState_MANAGE_A2",1,1);
	_pStateMachine->addState(1,1,"MANAGE_A2",STATEMACHINE::OR,6,false,true);
	_pStateMachine->addRegion("R_MANAGE_A1",0,6);
	_pStateMachine->addState(0,6,"Nominal",STATEMACHINE::OR,7,false,true);
	_pStateMachine->addRegion("R_Nominal",0,7);
	_pStateMachine->addState(0,6,"STOP",STATEMACHINE::OR,8);
	_pStateMachine->addRegion("R_STOP",0,8);
	_pStateMachine->addState(0,6,"Reset",STATEMACHINE::OR,9);
	_pStateMachine->addRegion("R_Reset",0,9);

	_pStateMachine->addTransition(3,5,0,1);
	_pStateMachine->addTransition(5,4,0,2);
	_pStateMachine->addTransition(7,9,0,3);
	_pStateMachine->addTransition(9,8,0,4);

	_pStateMachine->build();
	
		

}






void SMTest_Behavior::DataSource::Init()
{
	Accelerator1 = 1 ;
	Accelerator2 = 2 ;
	Accelerator3 = 3 ;
	Accelerator4 = 4 ;
	Accelerator5 = 5 ;
	Accelerator6 = 6 ;
	ADIRUComputer1 = 11 ;
	ADIRUComputer2 = 12 ;
	ADIRUComputer3 = 13 ;
	ADIRUComputer4 = 14 ;
	VoterCenter = 21 ;
	VoterLeft = 22 ;
	VoterRight = 23 ;
	DisplayComponentC = 31 ;
	DisplayComponentL = 32 ;
	DisplayComponentR = 33 ;

	//Enter code here
	FC = -1;
}
		
void SMTest_Behavior::DataSource::Destroy()
{
	//Enter code here
}
		
void SMTest_Behavior::DataSource::RESPONSE ( SystemHMActionType action , int PROCESS )
{
	//Enter code here
	cout<<"SMTest_Behavior. Action "<<action <<"Component "<<PROCESS<<endl;
	
}


