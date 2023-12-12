# DebbieSH - Simple Shell Implementation

DebbieSH is a basic shell program written in C that provides a simple command-line interface with history tracking and basic pipeline functionality.

## Features

- **Command Execution**: DebbieSH allows users to execute commands provided via the command line.

- **Command History**: Keep track of the most recent commands entered by the user.

- **Command Retrieval**: Retrieve and execute commands from history using `!!` or `!N`, where N is the command number.

- **Pipeline Support**: Execute commands in a pipeline by using the `|` character.

## Notes

- Use `history` to view the command history.

- Use `!!` to execute the most recent command.

- Use `!N` to execute the N-th command from history.

- Pipeline (`|`) is supported for basic command chaining.

## Disclaimer

This is a simple shell implementation intended for educational purposes. It may lack advanced features and robust error handling. Use it at your own risk.
