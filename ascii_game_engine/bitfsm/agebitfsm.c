/*
** This source file is part of AGE
**
** For the latest info, see http://code.google.com/p/ascii-game-engine/
**
** Copyright (c) 2011 Tony & Tony's Toy Game Development Team
**
** Permission is hereby granted, free of charge, to any person obtaining a copy of
** this software and associated documentation files (the "Software"), to deal in
** the Software without restriction, including without limitation the rights to
** use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
** the Software, and to permit persons to whom the Software is furnished to do so,
** subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
** FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
** COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
** IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
** CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
** This implemention of agebitfsm is a C porting of the original bitfsm in C++
** For more info about bitfsm, see http://code.google.com/p/bitfsm/
*/

#include "agebitfsm.h"

static bl _ensure_index_and_step_index_valid(Fsm* _fsm, s32 _index, s32 _step) {
	assert(_index >= 0 && _index < _fsm->current->status->bit_count);
	if(!(_index >= 0 && _index < _fsm->current->status->bit_count)) {
		return FALSE;
	}
	assert(_step >= 0 && _step < _fsm->rule_steps[_index].steps_count);
	if(!(_step >= 0 && _step < _fsm->rule_steps[_index].steps_count)) {
		return FALSE;
	}

	return TRUE;
}

FsmStatus* create_fsm_status(s32 _bitsCount) {
	FsmStatus* result = AGE_MALLOC(FsmStatus);
	result->status = bs_create(_bitsCount);
	result->index = -1;

	return result;
}

void destroy_fsm_status(FsmStatus* _obj) {
	bs_destroy(_obj->status);
	AGE_FREE(_obj);
}

FsmStep* create_fsm_step(s32 _bitsCount) {
	FsmStep* result = AGE_MALLOC(FsmStep);
	result->condition = bs_create(_bitsCount);
	result->next = -1;
	result->exact = FALSE;

	return result;
}

void destroy_fsm_step(FsmStep* _obj) {
	bs_destroy(_obj->condition);
	AGE_FREE(_obj);
}

FsmRuleStep* create_fsm_rule_step(s32 _count) {
	FsmRuleStep* result = AGE_MALLOC_N(FsmRuleStep, _count);
	result->index = -1;
	result->steps = 0;
	result->steps_count = 0;
	result->tag = 0;

	return result;
}

void destroy_fsm_rule_step(FsmRuleStep* _obj) {
	s32 i = 0;

	if(_obj) {
		if(_obj->steps) {
			for(i = 0; i < _obj->steps_count; ++i) {
				destroy_fsm_step(_obj->steps[i]);
			}
			AGE_FREE(_obj->steps);
		}
	}
}

void append_fsm_rule_step(FsmRuleStep* _ruleStep, FsmStep* _step) {
	assert(_ruleStep && _step);

	_ruleStep->steps = AGE_REALLOC(FsmStep*, _ruleStep->steps, sizeof(FsmStep*) * (++_ruleStep->steps_count));
	_ruleStep->steps[_ruleStep->steps_count - 1] = _step;
}

void remove_fsm_rule_step(FsmRuleStep* _ruleStep, s32 _index) {
	assert(_ruleStep);
	assert(_index >= 0 && _index < _ruleStep->steps_count);

	AGE_FREE(_ruleStep->steps[_index]);
	_ruleStep->steps[_index] = _ruleStep->steps[_ruleStep->steps_count - 1];
	--_ruleStep->steps_count;
}

bl walk_rule_step(FsmRuleStep* _ruleStep, FsmStatus* _curr, Bitset* _status, bl _exact) {
	Bitset* _s = 0;
	s32 i = 0;
	FsmStep* _ck = 0;

	if(_ruleStep->index == -1) {
		return FALSE;
	}

	_s = bs_create(_status->bit_count);
	if(_exact) {
		bs_set_all_bits(_s, _status);
	} else {
		bs_arithmetic_or(_curr->status, _status, _s);
	}
	for(i = 0; i < _ruleStep->steps_count; ++i) {
		_ck = _ruleStep->steps[i];
		if((_ck->exact && bs_equals(_ck->condition, _s)) || (!_ck->exact && (bs_logic_and(_ck->condition, _s)))) {
			_curr->index = _ck->next;
			bs_set_all_bits(_curr->status, _s);
			bs_destroy(_s);

			return TRUE;
		}
	}
	bs_destroy(_s);

	return FALSE;
}

Fsm* create_bitfsm(s32 _statusCount, s32 _commandCount, ObjToIndexFunc _objToIndex, ObjToCommandFunc _objToCommand, IntStepHendlerFunc _intHandler, ObjStepHandlerFunc _objHandler, destroyer _tagDestructor) {
	Fsm* result = 0;

	result = AGE_MALLOC(Fsm);
	result->current = create_fsm_status(_commandCount);
	result->rule_steps = create_fsm_rule_step(_statusCount);
	result->rule_steps_count = _statusCount;
	result->obj_to_index = _objToIndex;
	result->obj_to_command = _objToCommand;
	result->int_handler = _intHandler;
	result->obj_handler = _objHandler;
	result->tag_destructor = _tagDestructor;

	return result;
}

void destroy_bitfsm(Fsm* _fsm) {
	s32 i = 0;
	if(_fsm->rule_steps) {
		for(i = 0; i < _fsm->rule_steps_count; ++i) {
			destroy_fsm_rule_step(&_fsm->rule_steps[i]);
		}
		AGE_FREE(_fsm->rule_steps);
		_fsm->rule_steps = 0;
		_fsm->rule_steps_count = 0;
	}
	destroy_fsm_status(_fsm->current);
	AGE_FREE(_fsm);
}

void reset_bitfsm(Fsm* _fsm) {
	bs_clear(_fsm->current->status);
	_fsm->current->index = -1;
	_fsm->terminal_index = -1;
}

void clear_bitfsm(Fsm* _fsm) {
	reset_bitfsm(_fsm);
	clear_bitfsm_all_rule_steps(_fsm);
}

bl set_bitfsm_current_step_index(Fsm* _fsm, s32 _index) {
	assert(_index >= 0 && _index < _fsm->rule_steps_count);
	if(!(_index >= 0 && _index < _fsm->rule_steps_count)) {
		return FALSE;
	}
	_fsm->current->index = _index;

	return TRUE;
}

bl set_bitfsm_current_step_tag(Fsm* _fsm, Ptr _obj) {
	s32 _index = _fsm->obj_to_index(_obj);

	return set_bitfsm_current_step_index(_fsm, _index);
}

bl set_bitfsm_terminal_step_index(Fsm* _fsm, s32 _index) {
	assert(_index >= 0 && _index < _fsm->rule_steps_count);
	if(!(_index >= 0 && _index < _fsm->rule_steps_count)) {
		return FALSE;
	}
	_fsm->terminal_index = _index;

	return TRUE;
}

bl set_bitfsm_terminal_step_tag(Fsm* _fsm, Ptr _obj) {
	s32 _index = _fsm->obj_to_index(_obj);

	return set_bitfsm_terminal_step_index(_fsm, _index);
}

void set_bitfsm_step_handler(Fsm* _fsm, IntStepHendlerFunc _intHdl, ObjStepHandlerFunc _objHdl) {
	_fsm->int_handler = _intHdl;
	_fsm->obj_handler = _objHdl;
}

bl register_bitfsm_rule_step_index_to_tag(Fsm* _fsm, s32 _index, Ptr _tag) {
	assert(_index >= 0 && _index < _fsm->rule_steps_count);
	if(!(_index >= 0 && _index < _fsm->rule_steps_count)) {
		return FALSE;
	}
	_fsm->rule_steps[_index].tag = _tag;

	return TRUE;
}

bl register_bitfsm_rule_step_tag(Fsm* _fsm, Ptr _obj) {
	s32 _index = _fsm->obj_to_index(_obj);

	return register_bitfsm_rule_step_index_to_tag(_fsm, _index, _obj);
}

bl add_bitfsm_rule_step_by_index(Fsm* _fsm, s32 _index, Bitset* _cond, s32 _next, bl _exact) {
	FsmStep** _steps = 0;
	FsmStep* _step = 0;
	FsmStep* _newStep = 0;
	s32 i = 0;

	assert(_index >= 0 && _index < _fsm->rule_steps_count);
	if(!(_index >= 0 && _index < _fsm->rule_steps_count)) {
		return FALSE;
	}

	_steps = _fsm->rule_steps[_index].steps;
	for(i = 0; i < _fsm->rule_steps[_index].steps_count; ++i) {
		_step = _steps[i];
		if(bs_equals(_step->condition, _cond)) {
			return FALSE;
		}
	}

	_fsm->rule_steps[_index].index = _index;
	_newStep = create_fsm_step(_cond->bit_count);
	bs_set_all_bits(_newStep->condition, _cond);
	_newStep->next = _next;
	_newStep->exact = _exact;
	append_fsm_rule_step(&_fsm->rule_steps[_index], _newStep);

	return TRUE;
}

bl add_bitfsm_rule_step_by_tag(Fsm* _fsm, Ptr _indexObj, Bitset* _cond, Ptr _nextObj, bl _exact) {
	s32 _index = _fsm->obj_to_index(_indexObj);
	s32 _next = _fsm->obj_to_index(_nextObj);

	return add_bitfsm_rule_step_by_index(_fsm, _index, _cond, _next, _exact);
}

bl add_bitfsm_rule_step_by_tag_params(Fsm* _fsm, Ptr _indexObj, ls_node_t* _cond, Ptr _nextObj, bl _exact) {
	bl result = TRUE;
	s32 _index = _fsm->obj_to_index(_indexObj);
	s32 _next = _fsm->obj_to_index(_nextObj);
	Bitset* _condBits = bs_create(_fsm->current->status->bit_count);
	s32 i = 0;
	s32 idx = 0;
	Ptr data = 0;
	for(i = 0; i < (s32)ls_count(_cond); ++i) {
		data = ls_at(_cond, i)->data;
		idx = _fsm->obj_to_command(data);
		bs_set_bit(_condBits, idx, TRUE);
	}
	result = add_bitfsm_rule_step_by_index(_fsm, _index, _condBits, _next, _exact);
	bs_destroy(_condBits);

	return result;
}

bl remove_bitfsm_rule_step_by_index(Fsm* _fsm, s32 _index, Bitset* _cond) {
	FsmStep** _steps = 0;
	FsmStep* _step = 0;
	s32 i = 0;

	assert(_index >= 0 && _index < _fsm->rule_steps_count);
	if(!(_index >= 0 && _index < _fsm->rule_steps_count)) {
		return FALSE;
	}

	_steps = _fsm->rule_steps[_index].steps;
	for(i = 0; i < _fsm->rule_steps[_index].steps_count; ++i) {
		_step = _steps[i];
		if(bs_equals(_step->condition, _cond)) {
			remove_fsm_rule_step(_fsm->rule_steps, i);

			return TRUE;
		}
	}

	return FALSE;
}

bl remove_bitfsm_rule_step_by_tag(Fsm* _fsm, Ptr _indexObj, Bitset* _cond) {
	s32 _index = _fsm->obj_to_index(_indexObj);

	return remove_bitfsm_rule_step_by_index(_fsm, _index, _cond);
}

bl remove_bitfsm_rule_step_by_tag_params(Fsm* _fsm, Ptr _indexObj, ls_node_t* _cond) {
	bl result = TRUE;
	s32 _index = _fsm->obj_to_index(_indexObj);
	Bitset* _condBits = bs_create(_fsm->current->status->bit_count);
	s32 i = 0;
	s32 idx = 0;
	Ptr data = 0;
	for(i = 0; i < (s32)ls_count(_cond); ++i) {
		data = ls_at(_cond, i)->data;
		idx = _fsm->obj_to_command(data);
		bs_set_bit(_condBits, idx, TRUE);
	}
	result = remove_bitfsm_rule_step_by_index(_fsm, _index, _condBits);
	bs_destroy(_condBits);

	return result;
}

void clear_bitfsm_rule_step(Fsm* _fsm, s32 _index) {
	assert(_index >= 0 && _index < _fsm->rule_steps_count);
	if(_index >= 0 && _index < _fsm->rule_steps_count) {
		_fsm->rule_steps[_index].index = -1;
		if(_fsm->tag_destructor) {
			_fsm->tag_destructor(_fsm->rule_steps[_index].tag);
			_fsm->rule_steps[_index].tag = 0;
		}
	}
}

void clear_bitfsm_all_rule_steps(Fsm* _fsm) {
	s32 i = 0;
	for(i = 0; i < _fsm->rule_steps_count; ++i) {
		clear_bitfsm_rule_step(_fsm, i);
	}
	for(i = 0; i < _fsm->rule_steps_count; ++i) {
		destroy_fsm_rule_step(&_fsm->rule_steps[i]);
	}
	AGE_FREE(_fsm->rule_steps);
	_fsm->rule_steps = 0;
	_fsm->rule_steps_count = 0;
}

s32 get_bitfsm_current_step(Fsm* _fsm) {
	return _fsm->current->index;
}

Bitset* get_bitfsm_current_status(Fsm* _fsm) {
	return _fsm->current->status;
}

bl walk_bitfsm_with_int(Fsm* _fsm, s32 _status, bl _exact) {
	bl _result = TRUE;
	Bitset* _bs = 0;
	s32 _srcIdx = 0;
	Ptr _srcTag = 0;
	s32 _tgtIdx = 0;
	Ptr _tgtTag = 0;

	assert(_status >= 0 && _status < _fsm->current->status->bit_count);
	if(!(_status >= 0 && _status < _fsm->current->status->bit_count)) {
		return FALSE;
	}
	if(_fsm->current->index < 0 || _fsm->current->index >= _fsm->rule_steps_count) {
		return FALSE;
	}

	_bs = bs_create(_fsm->current->status->bit_count);
	bs_set_bit(_bs, _status, TRUE);

	_srcIdx = _fsm->current->index;
	_srcTag = _fsm->rule_steps[_srcIdx].tag;
	_result = walk_rule_step(&_fsm->rule_steps[_srcIdx], _fsm->current, _bs, _exact);
	_tgtIdx = _fsm->current->index;
	_tgtTag = _fsm->rule_steps[_tgtIdx].tag;

	if(_result && (_fsm->int_handler || _fsm->obj_handler)) {
		if(_fsm->int_handler) {
			_fsm->int_handler(_srcIdx, _tgtIdx);
		}
		if(_fsm->obj_handler) {
			_fsm->obj_handler(_srcTag, _tgtTag);
		}
	}

	bs_destroy(_bs);

	return _result;
}

bl walk_bitfsm_with_tag(Fsm* _fsm, Ptr _obj, bl _exact) {
	s32 _status = _fsm->obj_to_command(_obj);

	return walk_bitfsm_with_int(_fsm, _status, _exact);
}

bl terminated_bitfsm(Fsm* _fsm) {
	return _fsm->current->index == _fsm->terminal_index;
}

s32 get_bitfsm_status_count(Fsm* _fsm) {
	return _fsm->rule_steps_count;
}

Ptr get_bitfsm_status_tag(Fsm* _fsm, s32 _index) {
	assert(_index >= 0 && _index < _fsm->rule_steps_count);
	if(!(_index >= 0 && _index < _fsm->rule_steps_count)) {
		return 0;
	}

	return _fsm->rule_steps[_index].tag;
}

s32 get_bitfsm_command_count(Fsm* _fsm, s32 _index) {
	assert(_index >= 0 && _index < _fsm->rule_steps_count);
	if(!(_index >= 0 && _index < _fsm->rule_steps_count)) {
		return 0;
	}

	return _fsm->rule_steps[_index].steps_count;
}

Bitset* get_bitfsm_step_command_condition(Fsm* _fsm, s32 _index, s32 _step) {
	if(!_ensure_index_and_step_index_valid(_fsm, _index, _step)) {
		assert(0 && "Invalid index");
		return 0;
	}

	return _fsm->rule_steps[_index].steps[_step]->condition;
}

s32 get_bitfsm_step_command_next(Fsm* _fsm, s32 _index, s32 _step) {
	if(!_ensure_index_and_step_index_valid(_fsm, _index, _step)) {
		assert(0 && "Invalid index");
		return 0;
	}

	return _fsm->rule_steps[_index].steps[_step]->next;
}

bl get_bitfsm_step_exact(Fsm* _fsm, s32 _index, s32 _step) {
	if(!_ensure_index_and_step_index_valid(_fsm, _index, _step)) {
		assert(0 && "Invalid index");
		return 0;
	}

	return _fsm->rule_steps[_index].steps[_step]->exact;
}
