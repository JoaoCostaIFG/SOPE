# Simpledu T4G06

This is a simpler version of the **gnu's du**. This project was
developed for a UNIX OS development university class.  
Check this repo's collaborators section to see my project partners.

## Collaborators

Project made by João de Jesus Costa, João Lucas Silva Martins and
Ricardo Jorge Cruz Fontão for the SOPE (FEUP -- MIEIC) curricular
unit.

## Compilation

Use `make` to compile the program.  
Use `make clean` to delete old compilation files + binary.  
To run the program: `simpledu -l [path] [-a] [-b] [-B size] [-L] [-S]
[--max-depth=N]`. You can also specify multiple options using a single
argument: `simpledu -lLabS`.

## Tests

Run the `ttest.sh` script like so:  
`./ttests.sh [optional_path [-L]]`  
You can pass an optional path for it to run on (instead of only the defaults).
You can also specify the -L options which runs the tests that involve dereferencing.

## License

The code in this repository is published under a GNU General
Public License: **gpl-3.0**.

