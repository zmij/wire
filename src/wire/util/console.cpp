/*
 * console.cpp
 *
 *  Created on: May 19, 2016
 *      Author: zmij
 */

#include <wire/util/console.hpp>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <functional>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <map>

#include <cstdlib>
#include <readline/readline.h>
#include <readline/history.h>

namespace wire {
namespace util {

namespace {
    HISTORY_STATE* empty_history_   = history_get_history_state();
}  /* namespace  */

struct console::impl {
    using command_list = ::std::map<::std::string, command>;

    promt_func      prompt_;
    command_list    commands_;
    HISTORY_STATE*  history_    = nullptr;

    static impl*    current_;

    impl(::std::string const& greeting, bool add_default)
        : impl{
            [greeting](){
                return greeting;
            }, add_default} {}
    impl(promt_func prompt, bool add_default)
        : prompt_{prompt}
    {
        // Init readline basics
        rl_attempted_completion_function = &impl::get_command_completions;
        if (add_default) {
            add_command("quit", quit_command());
            add_command("exit", quit_command());
            add_command("help", help_command());
            add_command("run", run_command());
        }
    }
    ~impl() {
        free(history_);
    }

    impl(impl const&) = delete;
    impl(impl&&) = delete;
    impl& operator = (impl const&) = delete;
    impl& operator = (impl&&) = delete;

    void
    get_history()
    {
        if (history_)
            free(history_);
        history_ = history_get_history_state();
    }

    void
    set_history()
    {
        if (history_) {
            history_set_history_state(history_);
        } else {
            history_set_history_state(empty_history_);
        }
    }

    string_list
    command_names() const
    {
        string_list cmds;
        for (auto const& cmd : commands_) {
            cmds.push_back(cmd.first);
        }
        return cmds;
    }

    void
    add_command(::std::string const& name, command const& cmd)
    {
        commands_[name] = cmd;
    }

    ret_code_type
    execute(::std::string const& command)
    {
        // Convert input to vector
        ::std::vector<::std::string> inputs;
        {
            ::std::istringstream iss(command);
            ::std::copy(::std::istream_iterator<::std::string>(iss),
                    ::std::istream_iterator<::std::string>(),
                    ::std::back_inserter(inputs));
        }

        if ( inputs.size() == 0 ) return return_code::ok;

        impl::command_list::iterator it;
        if ( ( it = commands_.find(inputs[0]) ) != end(commands_) ) {
            return (it->second)(inputs);
        }

        ::std::cout << "Command '" << inputs[0] << "' not found.\n";
        return return_code::error;
    }

    ret_code_type
    run(::std::string const& filename)
    {
        ::std::ifstream input(filename);
        if ( ! input ) {
            ::std::cout << "Could not find the specified file to execute.\n";
            return return_code::error;
        }
        ::std::string command;
        int counter = 0, result;

        while ( ::std::getline(input, command)  ) {
            if ( command[0] == '#' ) continue; // Ignore comments
            // Report what the Console is executing.
            ::std::cout << "[" << counter << "] " << command << '\n';
            if ( (result = execute(command)) ) return result;
            ++counter; ::std::cout << '\n';
        }

        // If we arrived successfully at the end, all is ok
        return return_code::ok;
    }

    ret_code_type
    read_line()
    {
        reserve();

        auto prompt = prompt_();
        char * buffer = readline(prompt.c_str());
        if ( !buffer ) {
            ::std::cout << '\n'; // EOF doesn't put last endline so we put that so that it looks uniform.
            return return_code::eof;
        }

        // TODO: Maybe add commands to history only if succeeded?
        if ( buffer[0] != '\0' )
            add_history(buffer);

        ::std::string line(buffer);
        free(buffer);

        return execute(line);
    }

    void
    reserve()
    {
        if ( current_ == this ) return;

        // Save state of other Console
        if ( current_ )
            current_->get_history();

        // Else we swap state
        set_history();

        // Tell others we are using the console
        current_ = this;
    }

    static char **
    get_command_completions(char const* text, int start, int)
    {
        char ** completion_list = nullptr;

        if ( start == 0 )
            completion_list = rl_completion_matches(text, &impl::command_iterator);

        return completion_list;
    }
    static char *
    command_iterator(char const* text, int state)
    {
        static impl::command_list::iterator it;
        if (!current_)
            return nullptr;
        auto& commands = current_->commands_;

        if ( state == 0 ) {
            it = begin(commands);

            while ( it != end(commands) ) {
                auto const& command = it->first;
                ++it;
                if ( command.find(text) != ::std::string::npos ) {
                    char * completion = new char[command.size()];
                    strcpy(completion, command.c_str());
                    return completion;
                }
            }
        }
        return nullptr;
    }

    command
    quit_command()
    {
        return {
            [](args_type const&) { return return_code::quit; },
            [](){ return "Exit program"; }
        };
    }

    command
    help_command()
    {
        return {
            [this](args_type const&){

                auto commands = command_names();
                ::std::cout << "Available commands are:\n";
                size_t width{0};
                for (auto const& cmd : commands_) {
                    if (cmd.first.size() > width)
                        width = cmd.first.size();
                }
                width += 4;

                for (auto const& cmd : commands_) {
                    ::std::cout << "\t" << ::std::setw(width)
                        << ::std::left << cmd.first;
                    if (cmd.second.help)
                        ::std::cout << cmd.second();
                    ::std::cout << "\n";
                }
                return 0;
            },
            []() { return "Show help"; }
        };
    }

    command
    run_command()
    {
        return {
            [this](args_type const& args) {
                if ( args.size() < 2 ) {
                    ::std::cout << "Usage: " << args[0] << " script_filename\n";
                    return 1;
                }
                return run(args[1]);
            },
            [](){ return "Run a script"; }
        };
    }
};

console::impl* console::impl::current_ = nullptr;

// Here we set default commands, they do nothing since we quit with them
// Quitting behaviour is hardcoded in readLine()
console::console(::std::string const& greeting, bool add_default)
    : pimpl_{ new impl{ greeting, add_default } }
{
}

console::console(promt_func prompt, bool add_default)
    : pimpl_{ new impl{ prompt, add_default } }
{
}

console::~console() = default;

void
console::add_command(::std::string const& name, command const& f)
{
    pimpl_->add_command(name, f);
}

void
console::add_command(::std::string const& name, command::command_func f)
{
    pimpl_->add_command(name, command{ f });
}

console::string_list
console::get_commands() const
{
    return pimpl_->command_names();
}

console::ret_code_type
console::execute(::std::string const& command)
{
    return pimpl_->execute(command);
}

console::ret_code_type
console::run(const ::std::string & filename)
{
    return pimpl_->run(filename);
}

console::ret_code_type
console::read_line()
{
    return pimpl_->read_line();
}

console::command
console::quit_command(::std::string const& help) const
{
    ::std::string msg = help.empty() ? "Exit program" : help;
    return {
        [](args_type const&) { return return_code::quit; },
        [msg](){ return msg; }
    };
}

console::command
console::help_command() const
{
    return pimpl_->help_command();
}

console::command
console::run_command() const
{
    return pimpl_->run_command();
}

}  /* namespace util */
}  /* namespace wire */
