#pragma once

#include <string>
#include <vector>

class CLI {
public:
    static void print_help();

    // Parses a raw CLI line into argv-like tokens.
    // Returns false if the line is syntactically invalid.
    static bool tokenize_command_line(const std::string& line, std::vector<std::string>& tokens);

    // Parses and executes a CLI command.
    // Returns false if the command asks to terminate the loop (exit/quit).
    static bool dispatch_cli_command(const std::vector<std::string>& tokens);

    // Blocking REPL loop.
    static void run_interactive_loop();
};