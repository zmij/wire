/*
 * console.hpp
 *
 *  Created on: May 19, 2016
 *      Author: zmij
 */

#ifndef WIRE_UTIL_CONSOLE_HPP_
#define WIRE_UTIL_CONSOLE_HPP_

#include <functional>
#include <string>
#include <vector>
#include <memory>

namespace wire {
namespace util {

class console {
public:
    using ret_code_type     = ::std::int32_t;
    using string_list       = ::std::vector<::std::string>;
    using args_type         = string_list;
    struct command {
        using command_func  = ::std::function<ret_code_type(args_type const&)>;
        using help_func     = ::std::function<::std::string()>;
        command_func    func;
        help_func       help;

        ret_code_type
        operator()(args_type const& args) const
        {
            if (func)
                return func(args);
            return 1;
        }
        ::std::string
        operator()() const
        {
            if (help)
                return help();
            return "";
        }
    };
    using promt_func        = ::std::function<::std::string()>;

    enum return_code {
        eof     = -2,
        quit    = -1,
        ok      = 0,
        error   = 1
    };

    struct add_commands_helper {
        console& cons_;

        add_commands_helper&
        operator()(::std::string const& s, command const& f)
        {
            cons_.add_command(s, f);
            return *this;
        }
        add_commands_helper&
        operator()(::std::string const& s, command::command_func f)
        {
            cons_.add_command(s, f);
            return *this;
        }
    };
public:
    /**
     * @brief Constructor.
     *
     * @param greeting This represents the prompt of the Console.
     */
    explicit
    console(::std::string const& greeting, bool add_default = true);
    explicit
    console(promt_func prompt, bool add_default = true);

    /**
     * @brief Basic destructor.
     *
     * Frees the history which is been produced by GNU readline.
     */
    ~console();

    /**
     * @brief This function registers a new command within the Console.
     *
     * @param s The name of the command as inserted by the user.
     * @param f The function that will be called once the user writes the command.
     */
    void
    add_command(::std::string const& s, command const& f);
    void
    add_command(::std::string const& s, command::command_func f);
    add_commands_helper
    add_commands()
    { return { *this }; }

    /**
     * @brief This function returns a list with the currently available commands.
     *
     * @return A vector containing all registered commands names.
     */
    string_list
    get_commands() const;

    /**
     * @brief This function executes an arbitrary string as if it was inserted via stdin.
     *
     * @param command The command that needs to be executed.
     *
     * @return The result of the operation.
     */
    ret_code_type
    execute(::std::string const& command);

    /**
     * @brief This function calls an external script and executes all commands inside.
     *
     * This function stops execution as soon as any single command returns something
     * different from 0, be it a quit code or an error code.
     *
     * @param filename The pathname of the script.
     *
     * @return What the last command executed returned.
     */
    ret_code_type
    run(::std::string const& filename);

    /**
     * @brief This function executes a single command from the user via stdin.
     *
     * @return The result of the operation.
     */
    ret_code_type
    read_line();

    command
    quit_command(::std::string const& help = {}) const;
    command
    help_command() const;
    command
    run_command() const;
private:
    console(console const&) = delete;
    console(console&&) = delete;
    console& operator =(console const&) = delete;
    console& operator =(console&&) = delete;

    struct impl;
    using pimpl = ::std::unique_ptr<impl>;
    pimpl pimpl_;
};

} /* namespace util */
} /* namespace wire */

#endif /* WIRE_UTIL_CONSOLE_HPP_ */
