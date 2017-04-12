#ifdef WIN32
#pragma warning(disable:4018)
#pragma warning(disable:4996)
#endif

#include <StateMachine.h>
#include <globals.h>
#include <iostream>
#include <string>

using namespace std;
using namespace STATEMACHINE_UTILS;
using namespace STATEMACHINE;

State::State(Region* pRegion, State* pState,  const char* name, StateType t, STATEID sid, bool defaultActionEnabled):
m_pRegion(pRegion),m_pState(pState),m_name(name),m_type(t),m_id(sid), m_DefaultActionSet(defaultActionEnabled),m_ds(0)
{
	Globals::Instance()->reg<<" Created State "<<m_name <<" with id "<<sid<<"\n";
	m_setOrderedOutTransitions.clear();
	m_setTimeOutTransitions.clear();
	m_setOrderedRegions.clear();
	m_RegionPathToRoot.clear();
	m_StatePathToRoot.clear();
	this->computePathToRoot();
}

State::~State()
{
	m_setTimeOutTransitions.clear();
	m_setOrderedRegions.clear();
	m_RegionPathToRoot.clear();
	m_StatePathToRoot.clear();
}

void State::addOutgoingTransition(Transition* t)
{
	Globals::Instance()->reg<<" Added outgoing transition "<<t->m_id <<" to State "<<m_name<<"\n";
	m_setOrderedOutTransitions.insert(t);	
}

void State::addTimeOutTransition(Transition* t)
{
	Globals::Instance()->reg<<" Added timeout  transition "<<t->m_id <<" to State "<<m_name<<"\n";
	m_setTimeOutTransitions.insert(t);	
}

void State::addChildRegion(Region* cRegion)
{
	Globals::Instance()->reg<<" Added child region "<<cRegion->m_name <<" to State "<<m_name<<"\n";
	m_setOrderedRegions.insert(cRegion);
}

void State::SetDataSource(DataSource* ds)
{
	m_ds = ds;
}

void State::SetDefaultAction()
{
	m_DefaultActionSet = true;
}

void State::performEntry()
{
	if (m_ds==0)
		return;
	EvalTransitionsForTimeOut(false);
	return m_ds->performStateEntryAction(m_id);
}

void State::performExit()
{
	if (m_ds==0)
		return;
	return m_ds->performStateExitAction(m_id);
}

void State::performDuring()
{
	if (m_ds==0)
		return;
	return m_ds->performStateDuringAction(m_id);
}


void State::computePathToRoot()
{
	m_RegionPathToRoot.clear();
	m_StatePathToRoot.clear();
	Region* region = m_pRegion;
	State* state = 0;
	while (region)
	{
		m_RegionPathToRoot.push_front(region);
		state = region->m_pState;
		m_StatePathToRoot.push_front(state);
		region = state->m_pRegion;
	}
}

void State::EvalTransitionsForTimeOut(bool evalduring)
{
	if (m_setTimeOutTransitions.empty())
		return;

	Globals::Instance()->reg<<" Evaluating time-out transitiona in state "<<m_name<<"\n";
	if (evalduring)
		performDuring();
	for(set<Transition*,TransitionComp>::iterator it = m_setTimeOutTransitions.begin();
	    it != m_setTimeOutTransitions.end(); ++it)
	{
		(*it)->checkTimeOut();
		
	}
	
}

Transition* State::EvalTransitions(deque<Transition*>& activeTransitions/*, set<TRANSID>& timeOutsToEval,*/, int& defaultStateActionID, bool lookForDefaultStateAction)
{
	Globals::Instance()->reg<<" Evaluating transition in state "<<m_name<<"\n";
	Transition* activeTransition = 0;
	//if (activeTransitions.empty())
	{
		performDuring();
		for(set<Transition*,TransitionComp>::iterator it = m_setOrderedOutTransitions.begin();
			it != m_setOrderedOutTransitions.end(); ++it)
		{
			if ((*it)->checkTimeOut() || ((*it)->checkEvent() && (*it)->checkGuard()))
			{
				activeTransition = (*it);
				activeTransitions.push_back((*it));
				Globals::Instance()->reg<<" Active Transition to "<<(*it)->m_id<<"\n";
				Globals::Instance()->reg<<" Active Transition to "<<(*it)->m_TargetState->m_name<<"\n";
				break;
				//return (*it);
			}
		}
	}
	// else
	// {
		// for(set<Transition*,TransitionComp>::iterator it = m_setTimeOutTransitions.begin();
	    // it != m_setTimeOutTransitions.end(); ++it)
		// {
			// if (timeOutsToEval.find((*it)->m_id) != timeOutsToEval.end())
			// {
				// activeTransitions.push_back((*it));
				// if ((*it)->checkGuard())
					// activeTransitions.push_back((*it));
			// }
		// }
	// }
	
	if (activeTransition != 0)
		return activeTransition;

	if (lookForDefaultStateAction && m_DefaultActionSet)
		defaultStateActionID = m_id;

	bool lookForDef = lookForDefaultStateAction;
	int defActionID = -1;
	for(set<Region*,RegionComp>::iterator it = m_setOrderedRegions.begin();
	    it != m_setOrderedRegions.end(); ++it)
	{
		if ((*it)->m_currentState == 0)
		{
			Globals::Instance()->reg<<" Current state is null in region "<<(*it)->m_name<<"\n";
			continue;
		}
		
		activeTransition = (*it)->m_currentState->EvalTransitions(activeTransitions/*, timeOutsToEval*/,defActionID, lookForDef);
		// if (t!=0)
			// return t;
		
		if (defActionID != -1)
			lookForDef = false;
	}

	if (activeTransitions.empty())
		defaultStateActionID = defActionID;

	Globals::Instance()->reg<<"No Active Transition "<<"\n";
	return 0;
}

//perform the exit of the child regions (invoke in-order the EvalExit of the child regions)
//invoke the performExit method
void State::EvalExit(set<TRANSID>& setTimeOutTransIDs, bool doExit)
{
	Globals::Instance()->reg<<" Eval Exit State "<<m_name<<"\n";
	for(set<Region*,RegionComp>::iterator it = m_setOrderedRegions.begin();
		it != m_setOrderedRegions.end(); ++it)
	{
		(*it)->EvalExit(setTimeOutTransIDs);
	}
	if (doExit)
	{
		for(set<Transition*,TransitionComp>::iterator it = m_setTimeOutTransitions.begin(); it!=m_setTimeOutTransitions.end(); ++it)
			setTimeOutTransIDs.insert((*it)->m_id);
		performExit();
	}
}



void State::EvalTimeOut(deque<State*>& statesToEnter, bool applyHistory, bool doEntry)
{

	Globals::Instance()->reg<<" Eval TimeOut "<<m_name<<" history "<<(applyHistory?1:0)<<"\n";
	if (doEntry)
		EvalTransitionsForTimeOut(false);
	
	
	bool checkHistory = false;
	if (!statesToEnter.empty() && (statesToEnter.front() == this))
	{
		statesToEnter.pop_front();
		if (statesToEnter.empty() && applyHistory)
			checkHistory = true;
	}

	for(set<Region*,RegionComp>::iterator it = m_setOrderedRegions.begin();
		it != m_setOrderedRegions.end(); ++it)
	{
		(*it)->EvalTimeOut(statesToEnter,applyHistory,checkHistory,doEntry);
	}
	
}

//invoke performEntry Method
//perform Entry of the child regions (invoke in-order the EvalEntry of the child regions)
//History is applied on the target state. 
//If History is set to be applied on the target state, and 
//if history_type == DeepHistory, 
//then the history on all its  child state are applied
//else history_type == ShallowHistory, 
//    history is applied only on the target state and not on its child state
//else if no history
//	  apply initial.
void State::EvalEntry(deque<State*>& statesToEnter, bool applyHistory, bool doEntry)
{
	Globals::Instance()->reg<<" Eval Entry State "<<m_name<<" history "<<(applyHistory?1:0)<<"\n";
	if (doEntry)
		performEntry();
	
	
	
	bool checkHistory = false;
	if (!statesToEnter.empty() && (statesToEnter.front() == this))
	{
		statesToEnter.pop_front();
		if (statesToEnter.empty() && applyHistory)
			checkHistory = true;
	}

	if (m_pRegion)
		m_pRegion->setCurrentState(this);

	for(set<Region*,RegionComp>::iterator it = m_setOrderedRegions.begin();
		it != m_setOrderedRegions.end(); ++it)
	{
		(*it)->EvalEntry(statesToEnter,applyHistory,checkHistory);
	}
	
}

Region::Region(const char* name, REGIONID rid, int priorityID, State* pState, bool enableHistory, bool enableDeepHistory):
m_name(name), m_id(rid),m_priority(priorityID), m_pState(pState), m_historyEnabled(enableHistory), m_isDeepHistory(enableDeepHistory),
m_initState(0), m_currentState(0)
{
	m_childStates.clear();
}

Region::~Region()
{
	m_pState = 0;
	m_childStates.clear();
}

void Region::addState(State* state, bool isInitial)
{
	m_childStates.insert(state);
	
	if (isInitial)
	{
		if (!m_initState)
		{
			m_initState = state;
			setCurrentState(m_initState);
			
		}
		else
			Globals::Instance()->reg<<"Initial State is already Set in Region "<<m_name<<"\n";
	}
}

void Region::setCurrentState(State* state)
{
	if (m_childStates.empty() || (state==0))
		return;
	
	if (m_childStates.find(state) == m_childStates.end())
	{
		Globals::Instance()->reg<<" Failed to set current state in region "<<m_name<<"\n";
		Globals::Instance()->reg<<" State "<<state->m_name <<" not a part of the region "<< m_name<<"\n";
		return;
	}

	Globals::Instance()->reg<<" Setting current state in "<<m_name << " to State "<<state->m_name<<"\n";

	m_currentState = state;
}

void Region::EnableShallowHistory()
{
	m_historyEnabled = true;
	m_isDeepHistory = false;
}
		
void Region::EnableDeepHistory()
{
	m_historyEnabled = true;
	m_isDeepHistory = true;
}


//invoke the EvalExit of the current state
void Region::EvalExit(set<TRANSID>& setTimeOutTransIDs)
{
	if (!m_currentState)
	{
		Globals::Instance()->reg<<" Failed to invoke EvalExit in Region "<<m_name <<" as current state is not set"<<"\n";
		return;
	}
	m_currentState->EvalExit(setTimeOutTransIDs);
}

void Region::EvalTimeOut(deque<State*>& statesToEnter,bool applyHistory, bool checkHistory, bool doEntry)
{
	//Initial Step: Identify the exact state to enter
	State* stateToEnter = 0;
	
	if (statesToEnter.empty())
	{//if input parameter is empty
	
		if (applyHistory)
			stateToEnter = m_currentState;
		else
			stateToEnter = m_initState;
		
		//use of history will continue to child states only if the history is marked as "DeepHistory"
		if (applyHistory && checkHistory && !m_isDeepHistory)
			applyHistory = false;

		
	}
	else
	{
		//On the way to the target state,
		//if the top state in the queue does not belong to this region, 
		//then enter the init state (as the top state might be a state in a sibling region)
		stateToEnter = statesToEnter.front();
		
		if (m_childStates.find(stateToEnter) == m_childStates.end())
		{
			stateToEnter = m_initState;
		}
		
		
		
		
	}

	if (stateToEnter != 0)
		stateToEnter->EvalTimeOut(statesToEnter,applyHistory,doEntry);

}


//if top state in statesToEnter is present in the region,
	//pop top state
	//invoke EvalEntry to top State.
//else 
	//apply init state
	//History is applied on the target state. 
		//If History is set to be applied on the target state, and 
		//if history_type == DeepHistory, 
			//then the history on all its  child state are applied
		//else history_type == ShallowHistory, 
		//    history is applied only on the target state and not on its child state
		//else if no history
		//	  apply initial.
		
void Region::EvalEntry(deque<State*>& statesToEnter,bool applyHistory, bool checkHistory)
{
	//Initial Step: Identify the exact state to enter
	State* stateToEnter = 0;
	
	if (statesToEnter.empty())
	{//if input parameter is empty
	
		if (applyHistory)
			stateToEnter = m_currentState;
		else
			stateToEnter = m_initState;
		
		//use of history will continue to child states only if the history is marked as "DeepHistory"
		if (applyHistory && checkHistory && !m_isDeepHistory)
			applyHistory = false;

		
		
	}
	else
	{
		//On the way to the target state,
		//if the top state in the queue does not belong to this region, 
		//then enter the init state (as the top state might be a state in a sibling region)
		stateToEnter = statesToEnter.front();
		
		if (m_childStates.find(stateToEnter) == m_childStates.end())
		{
			stateToEnter = m_initState;
		}
		
		
		
		
	}

	if (stateToEnter != 0)
	{
	
		setCurrentState(stateToEnter);
		stateToEnter->EvalEntry(statesToEnter,applyHistory);
	}

}

Transition::Transition(State* source, State* target,  int priorityID, TRANSID id, SYSTEM_TIME_TYPE timeout, bool setHistory, bool transitionBetweenParallelMachines):
m_SourceState(source),m_TargetState(target),m_priority(priorityID), m_id(id),m_timeout(timeout),m_setHistory(setHistory),m_TransitionBetweenParallelMachines(transitionBetweenParallelMachines), m_ds(0)
{
	m_StateToExit = 0;
	m_StatesToEnter.clear();
	m_RegionsToExit.clear();
	m_RegionsToEnter.clear();
	m_evalTimeOut = (m_timeout > 0);
	m_SourceState->addOutgoingTransition(this);
	if (m_evalTimeOut)
		m_SourceState->addTimeOutTransition(this);
	
}

Transition::~Transition()
{
	m_SourceState=0; 
	m_TargetState=0;
	m_StateToExit = 0;
	m_StatesToEnter.clear();
	m_RegionsToExit.clear();
	m_RegionsToEnter.clear();
}

void Transition::SetDataSource(DataSource* ds)
{
	m_ds = ds;
}

void Transition::SetEvalTimeOut(bool b)
{
	m_evalTimeOut = b;

	if (m_evalTimeOut && m_SourceState)
		m_SourceState->addTimeOutTransition(this);
}

bool Transition::checkTimeOut()
{
	if (m_ds==0)
		return false;
	if (!m_evalTimeOut)
		return false;

	m_ds->m_evaluatingTransition = true;
	m_ds->m_currentTransition = m_id;
	bool ret = m_ds->TIMEOUT(this->m_timeout);
	m_ds->m_evaluatingTransition = false;
	return ret;
	
}
		
bool Transition::checkEvent()
{
	if (m_ds==0)
		return true;
	return m_ds->evalTransitionEventTrigger(m_id);
}

bool Transition::checkGuard()
{
	if (m_ds==0)
		return true;
	m_ds->m_evaluatingTransition = true;
	m_ds->m_currentTransition = m_id;
	bool ret=m_ds->evalTransitionGuard(m_id);
	m_ds->m_evaluatingTransition = false;
	return ret;

}

void Transition::performAction()
{
	Globals::Instance()->reg<<" Performing transition action in transition id:"<<this->m_id << "between " <<this->m_SourceState->m_name<< " to "<<this->m_TargetState->m_name << "set history "<<(this->m_setHistory ?1:0)<<"\n";
	if (m_ds==0)
		return;
	return m_ds->performTransitionAction(m_id);
}

//This method is executed once, during initialization/build process
void Transition::computeEntryAndExitPoints()
{
	unsigned int size = m_SourceState->m_StatePathToRoot.size();
	if (size > m_TargetState->m_StatePathToRoot.size())
		size = m_TargetState->m_StatePathToRoot.size();
	
	unsigned int i=0;
	//bool sameRegion = true;
	m_StateToExit = m_SourceState;
	for(; i<size;i++)
	{
		if (m_SourceState->m_StatePathToRoot[i] != m_TargetState->m_StatePathToRoot[i])
		{
			m_StateToExit = m_SourceState->m_StatePathToRoot[i];
		
			if (m_SourceState->m_StatePathToRoot[i]->m_pRegion != m_TargetState->m_StatePathToRoot[i]->m_pRegion)
			{
				m_StateToExit = m_SourceState->m_StatePathToRoot[i-1];
				m_TransitionBetweenParallelMachines = true;
			}
			break;
		}
	}
	
	if (m_TransitionBetweenParallelMachines)
		m_StatesToEnter.push_back(m_StateToExit);
		
	for(unsigned int j=i; j<m_TargetState->m_StatePathToRoot.size(); j++)
		m_StatesToEnter.push_back(m_TargetState->m_StatePathToRoot[j]);
	
	m_StatesToEnter.push_back(m_TargetState);
		
}

bool TransitionComp::operator()(Transition* t1, Transition* t2) const 
{
	if ((t1 !=0 ) && (t2 !=0))
		return (t1->m_priority < t2->m_priority);
	
	if ((t1 !=0) && (t2 == 0))
		return true;

	return false;
}

bool RegionComp::operator()(Region* r1, Region* r2) const
{
	if ((r1 !=0 ) && (r2 !=0))
		return (r1->m_priority < r2->m_priority);
	
	if ((r1 !=0) && (r2 == 0))
		return true;

	return false;
}

Event::Event(const char* name, EVENTID id):m_name(name), m_id(id)
{
}

Event::~Event()
{
}


bool Event::operator<(const Event& rhs) const
{
	return ((this->m_id < rhs.m_id));
}

bool EventComp::operator()(Event& e1, Event& e2) const
{
	return (e1.m_id < e2.m_id);
}



//create state with null parent(region and state) and sets the statetype to OR
StateMachine::StateMachine(const char* name, DataSource* ds)
:State(0,0,name,OR,0), m_ds(ds)
{
	m_states.clear();
	m_regions.clear();
	m_transitions.clear();

	m_states[m_id] = this;

}

StateMachine::~StateMachine()
{
	m_states[m_id] = 0;
	for(map<STATEID, State*>::iterator it = m_states.begin(); it!= m_states.end(); it++)
	{
		State* s = (*it).second;
		(*it).second = 0;
		delete s;
	}

	for(map<REGIONID, Region*>::iterator it = m_regions.begin(); it!= m_regions.end(); it++)
	{
		Region* r = (*it).second;
		(*it).second = 0;
		delete r;
	}

	for(map<TRANSID, Transition*>::iterator it = m_transitions.begin(); it!= m_transitions.end(); it++)
	{
		Transition* t = (*it).second;
		(*it).second = 0;
		delete t;
	}

	
	m_states.clear();
	m_regions.clear();
	m_transitions.clear();
	
}



State*  StateMachine::addState(REGIONID prid, STATEID psid,  const char* name, StateType t, STATEID sid, bool defaultActionEnabled, bool isInitialState)
{
	if (m_regions.find(prid) == m_regions.end())
	{
		Globals::Instance()->reg<<"Error: Unknown Region. ID = "<<prid<<". while creating State: "<< name<<"\n";
		return 0 ;
	}
	
	if ( m_states.find(psid) == m_states.end())
	{
		Globals::Instance()->reg<<"Error: Unknown State. ID = "<<psid<<". while creating State: "<< name<<"\n";
		return 0 ;
	}

	Region* pregion = m_regions[prid];
	State* pstate = m_states[psid];
	State* state = new State(pregion,pstate,name,t,sid, defaultActionEnabled);
	state->SetDataSource(m_ds);
	m_states[sid] = state;

	pregion->addState(state,isInitialState);

	return state;
}

Region* StateMachine::addRegion(const char* name, REGIONID rid,  STATEID psid,int priorityID)
{
	if ( m_states.find(psid) == m_states.end())
	{
		Globals::Instance()->reg<<"Error: Unknown State. ID = "<<psid<<". while creating Region: "<< name<<"\n";
		return 0 ;
	}

	State* pstate = m_states[psid];
	Region* region = new Region(name, rid, priorityID, pstate);
	
	m_regions[rid] = region;
	pstate->addChildRegion(region);
	
	return region;
}

Transition* StateMachine::addTransition(STATEID sourcesid, STATEID targetsid, int priorityID, TRANSID tid, SYSTEM_TIME_TYPE timeout, bool setHistory, bool transitionBetweenParallelMachines)
{

	if (m_states.find(sourcesid) == m_states.end())
	{
		Globals::Instance()->reg<<"Error: Unknown Source State. ID = "<<sourcesid<<". while creating Transition: "<< tid<<"\n";
		return 0 ;
	}

	if (m_states.find(targetsid) == m_states.end())
	{
		Globals::Instance()->reg<<"Error: Unknown Target State. ID = "<<targetsid<<". while creating Transition: "<< tid<<"\n";
		return 0 ;
	}

	State* srcState = m_states[sourcesid];
	State* dstState = m_states[targetsid];
	Transition* trans = new Transition(srcState, dstState,priorityID,tid,timeout, setHistory,transitionBetweenParallelMachines);
	trans->SetDataSource(m_ds);
	m_transitions[tid]=trans;

	return trans;

}




void StateMachine::build()
{
	for(map<TRANSID, Transition*>::iterator it = m_transitions.begin(); it!= m_transitions.end(); it++)
	{
		Transition* t = (*it).second;
		t->computeEntryAndExitPoints();
	}
}


void StateMachine::start()
{
	Globals::Instance()->reg<<"************************ StateMachine started *******************************"<<"\n";
	step(true);
}

//assumption:
		//	the new set of events and new set of inputs have been copied into data source
			//  visit the cur_state and execute its performDuring
		// Transition* t = cur_state->EvalTransition();
		// if (t==null)
		//   do the needfull ( throw deadlock exception or just ignore and proceed)
		// State * s= t->getTopStatetoExit();
		// s->EvalExit();
		// State * s= t->listOfStatesToEnterInOrder();
		// s->EvalEntry();
		// check if internal events in internalEventQueue
		// if so repeat step
void StateMachine::step(bool init)
{
	//ACM_INTERNAL::AUTO_LOCK mylock (m_StateMutex);
	m_ds->ResetOutputEvents();
	m_ds->ApplyAndResetInputEvents();
	m_ds->AppyTimerEvents();
	cout<<" before logger"<<endl;	
	Globals::Instance()->reg<<"************************ step invoked *******************************"<<"\n";
	cout<<" after logger"<<endl;
	
	if (init)
	{
		deque<State*> statesToEnterInit;
		statesToEnterInit.clear();
		this->EvalTimeOut(statesToEnterInit,false,true);
		m_ds->EvaluateTimeOuts();
		this->EvalEntry(statesToEnterInit,false,true);
		m_ds->ApplyAndResetInternalEvents();
		if (!m_ds->IsCurrentEventSet())
			return;
	}

	//set<TRANSID> timeOutTransitions;
	//timeOutTransitions.clear();
	//m_ds->GetTimeOutTransitionsToEvaluate(timeOutTransitions);

	deque<Transition*> deqTransitions;
	deqTransitions.clear();
	while (true)
	{
		
		int defaultActionID = -1;
		
		m_ds->m_EvaluatingTimeOuts = true;
	
		Transition* t = 0;
		this->EvalTransitions(deqTransitions/*,timeOutTransitions*/,defaultActionID);
		
		m_ds->m_EvaluatingTimeOuts = false;
		
		if (t==0 && deqTransitions.empty() )
		{
			if (defaultActionID ==-1)
				Globals::Instance()->reg<<" No valid transition found! Quitting Step "<<"\n";
			else
			{
				STATEID sid = defaultActionID;
				m_ds->performDefaultStateAction(sid);
			}
			break;
		}
		
		set<TRANSID> setTimeOutTransIDs;
		setTimeOutTransIDs.clear();
		
		while (!deqTransitions.empty())
		{
			t=deqTransitions.front();
			deqTransitions.pop_front();
			
			Globals::Instance()->reg<<" evaluating transition "<<t->m_id<<"\n";
			
					
			
			State* exitState = t->m_StateToExit;
			//m_ds->clearTimeOutCalls();
			exitState->EvalExit( setTimeOutTransIDs,!t->m_TransitionBetweenParallelMachines);

			if (t->m_StatesToEnter.empty())
			{
				Globals::Instance()->reg<<" Problem ... States to enter is empty! Quitting Step !!"<<"\n";
				break;
			}

			t->performAction();
		
			deque<State*> statesToEnter, statesToEnterToEvalTimeOut;
			statesToEnter.clear();
			statesToEnterToEvalTimeOut.clear();
			for(unsigned int i=0; i!= t->m_StatesToEnter.size(); i++)
			{
				statesToEnter.push_back(t->m_StatesToEnter[i]);
				statesToEnterToEvalTimeOut.push_back(t->m_StatesToEnter[i]);
			}
		

			State* firstState = statesToEnterToEvalTimeOut.front();
			firstState->EvalTimeOut(statesToEnterToEvalTimeOut,t->m_setHistory,!t->m_TransitionBetweenParallelMachines);
			

			State* entryState = statesToEnter.front();
			entryState->EvalEntry(statesToEnter,t->m_setHistory,!t->m_TransitionBetweenParallelMachines);
		}
		
		m_ds->clearTimeOutCalls(setTimeOutTransIDs);
		m_ds->EvaluateTimeOuts();
		//m_ds->ResetInputEvents();
		m_ds->ApplyAndResetInternalEvents();

		if (m_ds->IsCurrentEventSet())
			continue;
		else
			break;
	}
	
	//m_ds->ResetInputEvents();
	//m_ds->ResetInternalEvents();
	
	m_ds->m_processInput = true;
		
	
}

DataSource::DataSource():m_timer(0), m_EvaluatingTimeOuts(false), m_evaluatingTransition(false)
{
	clearlists();
	m_eventsID.clear();
	m_eventsName.clear();
	


}

DataSource::~DataSource()
{
	
	m_eventsName.clear();
	for(map<EVENTID, Event*>::iterator it = m_eventsID.begin(); it!= m_eventsID.end(); it++)
	{
		Event* e = (*it).second;
		(*it).second = 0;
		delete e;
	}
	m_eventsID.clear();

	clearlists();

}

void DataSource::setTimer(Timer* t)
{
	m_timer = t;
	if (m_timer)
		m_timer->setCallbackObject(this);
}


void DataSource::clearlists()
{
	m_inputEvents.clear();
	m_outputEvents.clear();
	m_currentEvents.clear();
	m_internalEvents.clear();
	m_timerEvents.clear();
	m_transTimeOutTime.clear();
	m_transTimeOutTimeToEval.clear();
}

Event* DataSource::addEvent(EVENTID eid, const char* name)
{

	Event* e = new Event(name, eid);
	m_eventsName[e->m_name] = e;
	m_eventsID[eid] = e;

	return e;

}

void DataSource::setCurrentTime()
{
	m_currentTime = currentTime();
}

void DataSource::setCurrentTime(TIMETYPE t)
{
	m_currentTime = t;
}

bool DataSource::TIMEOUT(double val)
{
	if (!m_evaluatingTransition)
		return false;
		
	
	if (!m_EvaluatingTimeOuts)
	{
		TIMETYPE timeout = addTimeNano(m_currentTime,convertSecsToNano(val));
		m_transTimeOutTimeToEval[m_currentTransition] = timeout;
		return false;
	}
		
	if ((m_EvaluatingTimeOuts) && (m_transTimeOutTime.find(m_currentTransition)!= m_transTimeOutTime.end()))
	{
		return (compareTime(m_transTimeOutTime[m_currentTransition],m_currentTime));
	}
		
	return false;
}


void DataSource::clearTimeOutCalls(set<TRANSID>& setTimeOutTransIDs)
{
	
	map<TRANSID, TIMETYPE> templist;
	templist.clear();

	for (map<TRANSID, TIMETYPE>::iterator it = m_transTimeOutTime.begin(); it != m_transTimeOutTime.end(); it++)
	{
		TRANSID t = (*it).first;
		if (setTimeOutTransIDs.find(t) != setTimeOutTransIDs.end())
			continue;
		templist[t] = (*it).second;
	}

	m_transTimeOutTime.clear();
	for (map<TRANSID, TIMETYPE>::iterator it = templist.begin(); it != templist.end(); it++)
	{
		m_transTimeOutTime[(*it).first] = (*it).second;
	}
}

//get the list of time out transitions
//deprecated
/*
void DataSource::GetTimeOutTransitionsToEvaluate(set<TRANSID>& timeOutTransitions)
{
	timeOutTransitions.clear();
	for (map<TRANSID, TIMETYPE>::iterator it = m_transTimeOutTime.begin(); it != m_transTimeOutTime.end(); it++)
	{
		TRANSID t = (*it).first;
		if (compareTime(m_transTimeOutTime[t],m_currentTime))
		{
			timeOutTransitions.insert(t);
		}
	}
	
}
*/


void DataSource::EvaluateTimeOuts()
{
	
	if (m_transTimeOutTimeToEval.empty())
		return;
	
	//m_transTimeOutTime.clear();//should we clear this?
	
		
	for (map<TRANSID,TIMETYPE>::iterator it = m_transTimeOutTimeToEval.begin(); it!= m_transTimeOutTimeToEval.end(); it++)
	{
		
		m_transTimeOutTime[(*it).first] = (*it).second;
	}

	addNewTimerRequests(m_transTimeOutTimeToEval);
	
	m_transTimeOutTimeToEval.clear();


}

void DataSource::addNewTimerRequests(map<TRANSID, TIMETYPE>& newtimeouts)
{
	if (!m_timer)
		return;

	for (map<TRANSID,TIMETYPE>::iterator it = newtimeouts.begin(); it!= newtimeouts.end(); it++)
	{
		m_timer->addNewTimerEvent((*it).second);
	}
}

void DataSource::SetTimerEvent(TIMETYPE t)
{
	if (compareTime(m_latestTimerEvent, t))
	{
		m_latestTimerEvent = t;
		m_timerEvents.push_back(t);
	}
}

void DataSource::AppyTimerEvents()
{
	if (m_timer)
	{
		setCurrentTime(m_latestTimerEvent);
		m_timerEvents.clear();
	}
	else
	{
		setCurrentTime();
	}
	
}

//reset input,output,internal event list
void DataSource::ResetEvents()
{
	ResetInputEvents();
	ResetOutputEvents();
	ResetInternalEvents();
	ResetCurrentEvents();
}
		
// Resets output event list
void DataSource::ResetOutputEvents() 
{
	m_outputEvents.clear();
	
}
		
// Resets input event list
void DataSource::ResetInputEvents() 
{
	m_inputEvents.clear();
}
		
// Resets internal event list
void DataSource::ResetInternalEvents() 
{
	m_internalEvents.clear();
}
		

// Resets current event list
void DataSource::ResetCurrentEvents() 
{
	m_currentEvents.clear();
}

// Checks if there are any events (input/internal) set in the current list
bool DataSource::IsCurrentEventSet()
{ 
	return  !m_currentEvents.empty();
} 


// Resets current events. Apply input events as current event and reset input event list
void DataSource::ApplyAndResetInputEvents() 
{
	m_currentEvents.clear();
	for(set<EVENTID>::iterator it = m_inputEvents.begin(); it!= m_inputEvents.end(); it++)
	{
		m_currentEvents.insert((*it));
	}
	m_processInput = false;
	m_inputEvents.clear();
}

//Resets current events. Apply internal events as current event and reset input event list
void DataSource::ApplyAndResetInternalEvents()
{
	m_currentEvents.clear();
	for(set<EVENTID>::iterator it = m_internalEvents.begin(); it!= m_internalEvents.end(); it++)
	{
		m_currentEvents.insert((*it));
	}
	m_internalEvents.clear();
}

void DataSource::GetOutputEvents(set<string>& events)
{
	events.clear();
	for(set<EVENTID>::iterator it = m_outputEvents.begin(); it!= m_outputEvents.end(); it++)
	{
		if (m_eventsID.find((*it)) == m_eventsID.end())
			continue;
		events.insert(m_eventsID[(*it)]->m_name);
	}

}


void DataSource::Output(EVENTID eid)
{
	if (m_eventsID.find(eid) == m_eventsID.end())
		return;
	m_outputEvents.insert(eid);

}
void DataSource::Output(const char* name)
{
	string ename(name);
	if (m_eventsName.find(ename) == m_eventsName.end())
		return;
	EVENTID eid = m_eventsName[ename]->m_id;
	m_outputEvents.insert(eid);
}

bool DataSource::ProcessEvent(EVENTID eid)
{
	if (m_evaluatingTransition)
	{
		if (m_currentEvents.find(eid) != m_currentEvents.end())
			return true;

		if (m_processInput)
		{
			if (m_inputEvents.find(eid) != m_inputEvents.end())
				return true;
		}
		else
		{
			if (m_internalEvents.find(eid) != m_internalEvents.end())
				return true;
		}
		
	}
	else
	{
		if (m_eventsID.find(eid) != m_eventsID.end())
		{
			if (m_processInput)
				m_inputEvents.insert(eid);
			else
				m_internalEvents.insert(eid);
		}

	}
	return false;


}
bool DataSource::ProcessEvent(const char* name)
{

	string ename(name);
	if (m_eventsName.find(ename) == m_eventsName.end())
		return false;
	EVENTID eid = m_eventsName[ename]->m_id;
	return ProcessEvent(eid);


}

Timer::Timer():m_ds(0)
{
}

Timer::~Timer()
{
}

void Timer::setCallbackObject(DataSource* ds)
{
	m_ds = ds;
}

void Timer::addNewTimerEvent(TIMETYPE t)
{
	//launch new timer event set to expire at t
}


void Timer::reportTimerExpiredEvent(TIMETYPE t)
{
	if (!m_ds) 
		return;
	m_ds->SetTimerEvent(t);
}


