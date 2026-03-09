## Naming conventions

Classes: PascalCase
Functions: snake_case
Variables: snake_case
Private: _underscore_prefix

Example:

class VirtualDevice
{
    void update_state();
};

## Coding practices
Use const where possible
Pass by reference for large objects
Avoid global variables

## Object-oriented design
Use encapsulation to hide implementation details
Use inheritance and polymorphism where appropriate
Use responsibility-driven design to assign clear responsibilities to classes
Use design patterns where applicable (e.g., Factory, Observer)

## Error handling
Use exceptions for error handling
Catch exceptions at appropriate levels

## Memory management
Prefer smart pointers
Avoid raw pointers

## Code formatting
Use tabs for indentation
Line length: 80 characters
Use spaces around operators
Use blank lines to separate logical blocks of code
Don't use trailing whitespace

## Comments
Limit comments to explain why code does something, not what it does
Use docstrings for functions and classes
Avoid redundant comments

## File organization
Organize code into modules based on functionality
Use clear and descriptive file names
Keep classes hpp and cpp files together
