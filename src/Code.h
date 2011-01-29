/*  GRAPHITE2 LICENSING

    Copyright 2010, SIL International
    All rights reserved.

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should also have received a copy of the GNU Lesser General Public
    License along with this library in the file named "LICENSE".
    If not, write to the Free Software Foundation, Inc., 59 Temple Place, 
    Suite 330, Boston, MA 02111-1307, USA or visit their web page on the 
    internet at http://www.fsf.org/licenses/lgpl.html.
*/
// This class represents loaded graphite stack machine code.  It performs 
// basic sanity checks, on the incoming code to prevent more obvious problems
// from crashing graphite.
// Author: Tim Eves

#pragma once

#include <cassert>
#include <utility>
#include <graphite2/Types.h>
#include "Main.h"
#include "Machine.h"

namespace graphite2 {

class Silf;
class Face;

namespace vm {

class Code 
{
public:
    enum status_t 
    {
        loaded,
        alloc_failed, 
        invalid_opcode, 
        unimplemented_opcode_used,
        out_of_range_data,
        jump_past_end,
        arguments_exhausted,
        missing_return
    };

private:
    class decoder;

    instr *     _code;
    byte  *     _data;
    size_t      _data_size,
                _instr_count;
    byte        _max_ref;
    mutable status_t _status;
    bool        _constraint,
                _modify,
                _delete;
    mutable bool _own;

    void release_buffers() throw ();
    void failure(const status_t) throw();

public:
    Code() throw();
    Code(bool is_constraint, const byte * bytecode_begin, const byte * const bytecode_end, 
         uint8 pre_context, uint16 rule_length, const Silf &, const Face &);
    Code(const Code &) throw();
    ~Code() throw();
    
    Code & operator=(const Code &rhs) throw();
    operator bool () const throw();
    status_t      status() const throw();
    bool          constraint() const throw();
    size_t        dataSize() const throw();
    size_t        instructionCount() const throw();
    bool          immutable() const throw();
    bool          deletes() const throw();

    int32 run(Machine &m, slotref * & map, Machine::status_t & status) const;
    CLASS_NEW_DELETE
};

inline Code::Code() throw()
: _code(0), _data(0), _data_size(0), _instr_count(0), _max_ref(0),
  _status(loaded), _own(false) {
}

inline Code::Code(const Code &obj) throw ()
 :  _code(obj._code), 
    _data(obj._data), 
    _data_size(obj._data_size), 
    _instr_count(obj._instr_count),
    _max_ref(obj._max_ref),
    _status(obj._status), 
    _constraint(obj._constraint),
    _modify(obj._modify),
    _delete(obj._delete),
    _own(obj._own) 
{
    obj._own = false;
}

inline Code & Code::operator=(const Code &rhs) throw() {
    if (_instr_count > 0)
        release_buffers();
    _code        = rhs._code; 
    _data        = rhs._data;
    _data_size   = rhs._data_size; 
    _instr_count = rhs._instr_count;
    _status      = rhs._status; 
    _constraint = rhs._constraint;
    _modify      = rhs._modify;
    _delete      = rhs._delete;
    _own         = rhs._own; 
    rhs._own = false;
    return *this;
}

inline Code::operator bool () const throw () {
    return _code && status() == loaded;
}

inline Code::status_t Code::status() const throw() {
    return _status;
}

inline bool Code::constraint() const throw() {
    return _constraint;
}

inline size_t Code::dataSize() const throw() {
    return _data_size;
}

inline size_t Code::instructionCount() const throw() {
    return _instr_count;
}

inline bool Code::immutable() const throw()
{
  return !(_delete || _modify);
}

inline bool Code::deletes() const throw()
{
  return _delete;
}

inline int32 Code::run(Machine & m, slotref * & map, Machine::status_t & status_out) const
{
    assert(_own);
    assert(*this);          // Check we are actually runnable

    if (!m.bounds_check(_max_ref))
    {
        status_out = Machine::slot_offset_out_bounds;
        return 1;
    }
    return  m.run(_code, _data, map, status_out);
}

} // namespace vm
} // namespace graphite2
