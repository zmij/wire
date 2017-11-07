/*
 * connection_debug_observer.hpp
 *
 *  Created on: Nov 16, 2016
 *      Author: zmij
 */

#ifndef WIRE_CORE_DETAIL_CONNECTION_FSM_OBSERVER_HPP_
#define WIRE_CORE_DETAIL_CONNECTION_FSM_OBSERVER_HPP_

#include <afsm/fsm.hpp>
#include <pushkin/util/demangle.hpp>
#include <iostream>
#include <sstream>

namespace wire {
namespace core {
namespace detail {


struct conection_fsm_observer : ::afsm::detail::null_observer {

    ::std::string
    cut_type_name(::std::string const& name) const noexcept
    {
        auto pos = name.find_last_of("::");
        if (pos != ::std::string::npos) {
            return name.substr(pos + 1);
        }
        return name;
    }

    template < typename FSM, typename SourceState, typename TargetState, typename Event>
    void
    state_changed(FSM const& fsm, SourceState const&, TargetState const&, Event const&) const noexcept
    {
        using ::psst::util::demangle;
        ::std::ostringstream os;
        root_machine(fsm)->tag(os) << " Connection state changed "
                << cut_type_name(demangle< typename SourceState::state_definition_type >())
                << " -> "
                << cut_type_name(demangle< typename TargetState::state_definition_type >())
                << " (" << cut_type_name(demangle<Event>()) << ")\n";
        ::std::cerr << os.str();
    }
    #if DEBUG_OUTPUT >= 5
    template < typename FSM, typename State >
    void
    state_cleared(FSM const& fsm, State const&) const noexcept
    {
        using ::psst::util::demangle;
        ::std::ostringstream os;
        root_machine(fsm)->tag(os) << " Connection state cleared "
                << cut_type_name(demangle<typename State::state_definition_type>()) << "\n";
        ::std::cerr << os.str();
    }

    #endif /* DEBUG_OUTPUT >= 5 */
    #if DEBUG_OUTPUT >= 4
    template < typename FSM, typename Event >
    void
    start_process_event(FSM const& fsm, Event const&) const noexcept
    {
        using ::psst::util::demangle;
        ::std::ostringstream os;
        root_machine(fsm)->tag(os) << " Connection start process event "
                << cut_type_name(demangle<Event>()) << "\n";
        ::std::cerr << os.str();
    }

    template < typename FSM, typename Event >
    void
    processed_in_state(FSM const& fsm, Event const&) const noexcept
    {
        using ::psst::util::demangle;
        ::std::ostringstream os;
        root_machine(fsm)->tag(os) << " Connection processed event "
                << cut_type_name(demangle<Event>()) << " in state\n";
        ::std::cerr << os.str();
    }

    template < typename FSM, typename Event >
    void
    enqueue_event(FSM const& fsm, Event const&) const noexcept
    {
        using ::psst::util::demangle;
        ::std::ostringstream os;
        root_machine(fsm)->tag(os) << " Connection enqueue event "
                << cut_type_name(demangle<Event>()) << "\n";
        ::std::cerr << os.str();
    }
    template < typename FSM, typename Event >
    void
    defer_event(FSM const& fsm, Event const&) const noexcept
    {
        using ::psst::util::demangle;
        ::std::ostringstream os;
        root_machine(fsm)->tag(os) << " Connection defer event "
                << cut_type_name(demangle<Event>()) << "\n";
        ::std::cerr << os.str();
    }
    #endif /* DEBUG_OUTPUT >= 4 */
    template < typename FSM, typename Event >
    void
    reject_event(FSM const& fsm, Event const&) const noexcept
    {
        using ::psst::util::demangle;
        ::std::ostringstream os;
        root_machine(fsm)->tag(os) << " Connection reject event "
                << cut_type_name(demangle<Event>()) << "\n";
        ::std::cerr << os.str();
    }
};

}  /* namespace detail */
}  /* namespace core */
}  /* namespace wire */



#endif /* WIRE_CORE_DETAIL_CONNECTION_FSM_OBSERVER_HPP_ */
