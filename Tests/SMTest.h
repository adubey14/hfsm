#ifndef SMTest_h
#define SMTest_h

#include <iostream>
#include <set>
#include <vector>
#include <map>

#include <StateMachine.h>

using namespace std;
using namespace STATEMACHINE;


enum SystemHMActionType {		 RESTORE 		, CHECKPOINT 		, STOPALL 		, RESETALL 	};

namespace SMTest_Behavior
{

	

	class DataSource : public STATEMACHINE::DataSource
	{
	
	public:
		DataSource();
		virtual ~DataSource();
		
		void Init();
		void Destroy();
		
		
		//Execute state entry-action 
		virtual void performStateEntryAction(STATEID sid);
		//Execute state exit-action		
		virtual void performStateExitAction(STATEID sid);
		//Execute state during-action
		virtual void performStateDuringAction(STATEID sid);
		//Execute default action in a state when there are no active outgoing transitions
		virtual void performDefaultStateAction(STATEID sid);
		
		//Execute default event action
		virtual void checkAndPerformDefaultEventAction();
			
		virtual void performTransitionAction(TRANSID tid);// Execute transitions -action
		virtual bool evalTransitionGuard(TRANSID tid); //Evaluate transitions - guard
		virtual bool evalTransitionEventTrigger(TRANSID tid); //Evaluate transitions - trigger

		bool TG_T1();
		bool TG_T2();
		bool TG_T3();
		bool TG_T4();

		void TA_T1();
		void TA_T2();
		void TA_T3();
		void TA_T4();
		
		
		// Function declarations
		void RESPONSE ( SystemHMActionType action , int PROCESS );

		//Method to Set Variable FC
		void setVariable_FC(int FC_in);

		//Method to Get Variable FC
		int getVariable_FC();


		//Variable declarations
		int FC ;

		//DATATYPES::int Accelerator1 ;
		int Accelerator1 ;
		//DATATYPES::int Accelerator2 ;
		int Accelerator2 ;
		//DATATYPES::int Accelerator3 ;
		int Accelerator3 ;
		//DATATYPES::int Accelerator4 ;
		int Accelerator4 ;
		//DATATYPES::int Accelerator5 ;
		int Accelerator5 ;
		//DATATYPES::int Accelerator6 ;
		int Accelerator6 ;
		//DATATYPES::int ADIRUComputer1 ;
		int ADIRUComputer1 ;
		//DATATYPES::int ADIRUComputer2 ;
		int ADIRUComputer2 ;
		//DATATYPES::int ADIRUComputer3 ;
		int ADIRUComputer3 ;
		//DATATYPES::int ADIRUComputer4 ;
		int ADIRUComputer4 ;
		//DATATYPES::int VoterCenter ;
		int VoterCenter ;
		//DATATYPES::int VoterLeft ;
		int VoterLeft ;
		//DATATYPES::int VoterRight ;
		int VoterRight ;
		//DATATYPES::int DisplayComponentC ;
		int DisplayComponentC ;
		//DATATYPES::int DisplayComponentL ;
		int DisplayComponentL ;
		//DATATYPES::int DisplayComponentR ;
		int DisplayComponentR ;

		
		
		
		

	}; //end class SHMEngine_CompBehavior::DataSource
	
	class Repo
	{
		public:
			static Repo* Instance();
			static void Terminate();
			StateMachine* SM();
			DataSource* DS();
			~Repo();
		private:
			Repo();
			void build();
			static Repo* _pInstance;
			StateMachine* _pStateMachine;
			DataSource* _pDataSource;
			
	};//end Repo class
	
	

	
}//end namespace SMTest_Behavior
#endif //SMTest_h