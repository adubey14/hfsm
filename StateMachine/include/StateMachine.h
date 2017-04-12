#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <deque>
#include <vector>
#include <map>
#include <set>
//#include <ACM/USER/ACM_UTILITIES.h>
//using namespace ACM_USER;

#include "utils.h"
using namespace STATEMACHINE_UTILS;

using namespace std;




namespace STATEMACHINE
{

	typedef unsigned long				STATEID;
	typedef unsigned long				TRANSID;
	typedef unsigned long				REGIONID;
	typedef unsigned long				EVENTID;
	typedef SYSTEM_TIMESPEC				TIMETYPE;
	

	
	

	enum StateType { OR, AND };
	enum EventType { INTERNAL, EXTERNAL, BOTH };

	class Region;
	class State;
	class Transition;
	class Event;
	class DataSource;
	class Timer;
	class TransitionComp;
	class RegionComp;


	

	class TransitionComp : public binary_function<Transition*, Transition*, bool> 
	{
	public:
		bool operator()(Transition* t1, Transition* t2) const; 
	};

	class RegionComp : public binary_function<Region*, Region*, bool> 
	{
	public:
		bool operator()(Region* r1, Region* r2) const; 
	};
	
	class EventComp : public binary_function<Event, Event, bool> 
	{
	public:
		bool operator()(Event& e1, Event& e2) const; 
	};

	


	class State
	{
	public:

		State(Region* pRegion, State* pState,  const char* name, StateType t, STATEID sid, bool defaultActionEnabled = false);
		virtual ~State();
		void SetDataSource(DataSource* ds);
		void SetDefaultAction();
		void addOutgoingTransition(Transition* t);
		void addTimeOutTransition(Transition* t);
		void addChildRegion(Region* cRegion);

		virtual void performEntry();
		virtual void performExit();
		virtual void performDuring();

		Transition* EvalTransitions(deque<Transition*>& activeTransitions, 
									/*set<TRANSID>& timeOutsToEval,*/
									int& defaultStateActionID, 
									bool lookForDefaultStateAction = true);
		void EvalTransitionsForTimeOut(bool evalduring=true);

		void EvalExit(set<TRANSID>& setTimeOutTransIDs, bool doExit = true);
		void EvalTimeOut(deque<State*>& statesToEnter, bool applyHistory, bool doEntry=true);
		void EvalEntry(deque<State*>& statesToEnter,bool applyHistory, bool doEntry = true);

		void computePathToRoot();

		Region* m_pRegion;
		State* m_pState;
		string m_name;
		StateType m_type;
		STATEID m_id;
		bool m_DefaultActionSet;
		DataSource* m_ds;

		set<Transition*,TransitionComp> m_setOrderedOutTransitions, m_setTimeOutTransitions;
		set<Region*,RegionComp> m_setOrderedRegions;
		deque<Region*> m_RegionPathToRoot;
		deque<State*> m_StatePathToRoot;
	};

	class Region
	{
	public:
		Region(const char* name, REGIONID rid, int priorityID=0, State* pState=0, bool enableHistory=false, bool enableDeepHistory=false);
		virtual ~Region();

		void addState(State* state, bool isInitial=false);
		void setCurrentState(State* state);
		
		void EnableShallowHistory();
		void EnableDeepHistory();
		void EvalExit(set<TRANSID>& setTimeOutTransIDs);
		void EvalTimeOut(deque<State*>& statesToEnter, bool applyHistory, bool checkHistory = false, bool doEntry = true);
		void EvalEntry(deque<State*>& statesToEnter,bool applyHistory, bool checkHistory = false);


		
		string m_name;		
		REGIONID m_id;
		int m_priority;
		State* m_pState;
		bool m_historyEnabled, m_isDeepHistory; //default false;

		State* m_initState; //stores init state or history (if history is enabled)
		State* m_currentState;
		
		set<State*> m_childStates;

	};

	class Transition
	{
	public:

		Transition(State* source, State* target, int priorityID, TRANSID id, SYSTEM_TIME_TYPE t=0, bool setHistory=false, bool transitionBetweenParallelMachines=false);
		virtual ~Transition();

		void SetDataSource(DataSource* ds);

		void SetEvalTimeOut(bool);

		virtual bool checkEvent();
		virtual bool checkGuard();
		virtual bool checkTimeOut();

		virtual void performAction();

		void computeEntryAndExitPoints();


		State* m_SourceState, *m_TargetState;
		int m_priority;
		TRANSID m_id;
		SYSTEM_TIME_TYPE m_timeout;
		bool m_evalTimeOut;
		bool m_setHistory;
		bool m_TransitionBetweenParallelMachines;
		
		DataSource* m_ds;
		State* m_StateToExit;
		vector<State*>  m_StatesToEnter;
		vector<Region*> m_RegionsToExit, m_RegionsToEnter;
	};

	

	class Event
	{
	public:
		Event(const char* name, EVENTID id);
		virtual ~Event();
		bool operator<(const Event& rhs) const;
		string m_name;
		EVENTID m_id;
	};

	class Timer
	{

		public:
			Timer();
			virtual ~Timer();
			void setCallbackObject(DataSource *ds);
			void addNewTimerEvent(TIMETYPE t);

			void reportTimerExpiredEvent(TIMETYPE t);

			DataSource* m_ds;
	};

	

	class DataSource
	{

	public:

		DataSource();
		virtual ~DataSource();

		virtual void setTimer(Timer* timer);

		Event* addEvent(EVENTID eid, const char* name);

		virtual void ResetEvents();//reset input,output,internal, current event list
		virtual void ResetOutputEvents(); // Resets output event list
		virtual void ResetInputEvents(); // Resets input event list
		virtual void ResetInternalEvents(); // Resets internal event list
		virtual void ResetCurrentEvents() ; // Resets current event list

		virtual void ApplyAndResetInputEvents(); // Resets current events. Apply input events as current event and reset input event list
		virtual void ApplyAndResetInternalEvents();//Resets current events. Apply internal events as current event and reset input event list
		virtual bool IsCurrentEventSet(); // Checks if there are any internal events set
		
		virtual void Output(EVENTID eid); //  output event
		virtual void Output(const char* name);// output event

		virtual bool ProcessEvent(EVENTID eid); // input event
		virtual bool ProcessEvent(const char* name); // input event

		virtual void GetOutputEvents(set<string>& events); // Gets the list of Output Events generated 

		//virtual bool CheckEvent(EVENTID eid);

		
		virtual void performStateEntryAction(STATEID sid) =0;	//Execute state entry-action 
		virtual void performStateExitAction(STATEID sid) =0; //Execute state exit-action
		virtual void performStateDuringAction(STATEID sid) =0;//Execute state during-action
		virtual void performDefaultStateAction(STATEID sid)=0;//Execute default action from a state when no outgoing transitions are active

		//virtual void checkAndPerformDefaultEventAction() =0;//Execute default event action

		virtual void performTransitionAction(TRANSID tid) =0; // Execute transaction -action
		virtual bool evalTransitionGuard(TRANSID tid) =0; //Evaluate transaction - guard
		virtual bool evalTransitionEventTrigger(TRANSID tid) =0; //Evaluate transaction - trigger

	
		
		virtual void clearTimeOutCalls(set<TRANSID>& setTimeOutTransIDs);//clear any pending timeout calls
		virtual void EvaluateTimeOuts();//evaluate new time-out calls
		virtual bool TIMEOUT(double t);

		virtual void AppyTimerEvents();

		virtual void addNewTimerRequests(map<TRANSID, TIMETYPE>& newtimeouts);
		virtual void SetTimerEvent(TIMETYPE t);

		virtual void setCurrentTime();//set current time
		virtual void setCurrentTime(TIMETYPE t);
		


		virtual void clearlists();


		
		map<TRANSID, TIMETYPE>  m_transTimeOutTime, m_transTimeOutTimeToEval;
		deque<TIMETYPE>         m_timerEvents;
		TIMETYPE				m_latestTimerEvent;

		set<EVENTID>            m_inputEvents, m_outputEvents, m_currentEvents, m_internalEvents;
		map<EVENTID, Event*>    m_eventsID;
		map<string, Event*>	    m_eventsName;

		

		Timer*	   m_timer;
		bool       m_EvaluatingTimeOuts;
		bool       m_evaluatingTransition;

		TRANSID    m_currentTransition;
		TIMETYPE   m_currentTime;
		bool       m_processInput;

		
	};


	class StateMachine : public State
	{

	
	public:
		StateMachine(const char* name, DataSource* ds);//create state with null parent(region and state) and sets the statetype to OR
		
		virtual ~StateMachine();

		void build();

		void start();

		void step(bool init=false);
	
		State*  addState(REGIONID prid, STATEID psid,  const char* name, StateType t, STATEID sid, bool defaultActionEnabled = false, bool isInitialState=false);

		Region* addRegion(const char* name, REGIONID rid, STATEID psid,int priorityID=0);

		Transition* addTransition(STATEID sourcesid, STATEID targetsid, int priorityID, TRANSID tid, SYSTEM_TIME_TYPE timeout=0, bool setHistory=false, bool transitionBetweenParallelMachines=false);

		Event* addEvent(EVENTID eid, const char* name);
		
	private:

		DataSource*				   m_ds;
		map<STATEID, State*>       m_states;
		map<REGIONID, Region*>     m_regions;
		map<TRANSID, Transition*>  m_transitions;
		
	};
}

#endif //STATE_MACHINE_H
