# C-Shell-Implementation

A custom shell implementation in C with advanced features including custom commands, piping, redirection, background processes, and signal handling.

## How to Run

### Prerequisites
- GCC compiler
- Linux/Unix environment (uses `/proc` filesystem)

### Compilation and Execution
```bash
make
./a.out
```

## Features

### Core Shell Features
- **Interactive prompt** with username, hostname, and current directory
- **Command execution** with support for external programs
- **Background processes** using `&` operator
- **Command history** with persistent logging
- **Piping** support with `|` operator
- **Input/Output redirection** using `<`, `>`, and `>>`
- **Signal handling** (Ctrl+C, Ctrl+Z)
- **Alias support** via `.myshrc` configuration file

### Custom Commands

#### 1. `hop` - Enhanced Directory Navigation
```bash
hop [path]              # Change to specified directory
hop                     # Change to home directory
hop ~                   # Change to home directory
hop -                   # Change to previous directory
hop path1 path2 path3   # Navigate through multiple directories
```

#### 2. `reveal` - Advanced File Listing
```bash
reveal [flags] [path]   # List files and directories
reveal -l              # Long format (detailed information)
reveal -a              # Show hidden files
reveal -al             # Combined flags
reveal                 # List current directory
```

#### 3. `log` - Command History Management
```bash
log                    # Display command history
log purge             # Clear command history
log execute <index>   # Execute command from history by index
```

#### 4. `proclore` - Process Information
```bash
proclore [pid]         # Show detailed process information
proclore              # Show information for current shell process
```

#### 5. `seek` - File and Directory Search
```bash
seek [flags] <target> [search_path]
seek -d <target>      # Search only directories
seek -f <target>      # Search only files
seek -e <target>      # Execute/display if single match found
seek <target>         # Search both files and directories
```

#### 6. `activities` - Background Process Monitor
```bash
activities            # Display all background processes with status
```

#### 7. `ping` - Process Signal Sending
```bash
ping <pid> <signal_number>   # Send signal to process
```

#### 8. `fg` - Foreground Process Control
```bash
fg <pid>              # Bring background process to foreground
```

#### 9. `bg` - Background Process Control
```bash
bg <pid>              # Continue stopped process in background
```

#### 10. `neonate` - Recent Process Monitor
```bash
neonate -n <time_arg> # Print most recent process PID every <time_arg> seconds
                      # Press 'x' to terminate
```

#### 11. `iMan` - Online Manual Pages
```bash
iMan <command>        # Fetch and display manual page for command
```

### Advanced Features

#### Piping
```bash
command1 | command2 | command3   # Chain commands with pipes
```

#### Input/Output Redirection
```bash
command < input.txt              # Input redirection
command > output.txt             # Output redirection (overwrite)
command >> output.txt            # Output redirection (append)
command < input.txt > output.txt # Combined redirection
```

#### Background Processes
```bash
command &                        # Run command in background
sleep 100 &                      # Example background process
```

#### Command Chaining
```bash
command1 ; command2 ; command3   # Execute commands sequentially
```

#### Aliases
Create aliases in `.myshrc` file:
```bash
alias reveala = reveal -a
alias home = hop ~
alias homeback = hop ~/.. ~
```

### Signal Handling
- **Ctrl+C (SIGINT)**: Interrupts foreground process (not the shell)
- **Ctrl+Z (SIGTSTP)**: Stops foreground process
- **Ctrl+D**: Exits the shell

### Files and Configuration
- **`.myshrc`**: Configuration file for aliases
- **`.my_shell_log`**: Persistent command history storage

## Implementation Details

### File Structure
- `main.c` - Main shell loop and command processing
- `disp.c` - Prompt display functionality
- `hop.c` - Directory navigation implementation
- `reveal.c` - File listing functionality
- `proclore.c` - Process information display
- `seek.c` - File/directory search implementation
- `activities.c` - Background process monitoring
- `fgbg.c` - Foreground/background process control
- `signal.c` - Signal handling and process signaling
- `neonate.c` - Recent process monitoring
- `iman.c` - Online manual page fetching
- `parse.c` - Command parsing for pipes and redirection
- `executepipesredir.c` - Pipe and redirection execution
- `utilities.c` - Utility functions

### Key Features
- Supports both custom commands and external programs
- Maintains command history across sessions
- Process group management for proper signal handling
- Color-coded output for better readability
- Robust error handling and edge case management

## Examples

```bash
# Basic navigation
hop ~/Documents
reveal -la

# Process management
sleep 100 &
activities
fg 1234
bg 1234

# File operations
seek -f "*.txt" ~/Documents
reveal -l > file_list.txt

# Piping and redirection
ps aux | grep bash | sort > process_list.txt
cat file.txt | grep "pattern" | wc -l

# Command history
log
log execute 3

# Process information
proclore 1234
ping 1234 9
```
