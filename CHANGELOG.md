## Version 1.1.2 | 2022/08/19

### Changed
* added more comments to the code, to explain what is happening more in detail

### Added
* `.gitignore` file
* `help` builtin, so users can type *"help"* to list all the available commands


## Version 1.1.1 | 2020/06/02

### Changed
* `README.md` to reflect all existing compiler flags

### Added
* *"command not found"* message in case an issued command has not been registered

### Fixed
* *Off by one* error, which caused the library to crash on `esp8266` and `esp32` boards
