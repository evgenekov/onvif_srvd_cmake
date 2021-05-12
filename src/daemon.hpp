
/*
 * daemon.h
 *
 *
 * version 1.1
 *
 *
 *
 * BSD 3-Clause License
 *
 * Copyright (c) 2015, Koynov Stas - skojnov@yandex.ru
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef DAEMON_HEADER
#define DAEMON_HEADER


#include <stddef.h>  //for NULL
#include <signal.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string>
#include <errno.h>




#define DAEMON_DEF_TO_STR_(text) #text
#define DAEMON_DEF_TO_STR(arg) DAEMON_DEF_TO_STR_(arg)


#define DAEMON_MAJOR_VERSION_STR  DAEMON_DEF_TO_STR(DAEMON_MAJOR_VERSION)
#define DAEMON_MINOR_VERSION_STR  DAEMON_DEF_TO_STR(DAEMON_MINOR_VERSION)
#define DAEMON_PATCH_VERSION_STR  DAEMON_DEF_TO_STR(DAEMON_PATCH_VERSION)

#define DAEMON_VERSION_STR  DAEMON_MAJOR_VERSION_STR "." \
                            DAEMON_MINOR_VERSION_STR "." \
                            DAEMON_PATCH_VERSION_STR



class DaemonInfo
{
public:

    bool  get_terminated           (void) const { return terminated;     }
    bool  get_daemonized           (void) const { return daemonized;     }
    bool  get_no_chdir             (void) const { return no_chdir;       }
    bool  get_no_fork              (void) const { return no_fork;        }
    bool  get_no_close_stdio       (void) const { return no_close_stdio; }
    std::string  get_pidFile       (void) const { return pidFile;        }
    std::string  get_logFile       (void) const { return logFile;        }
    std::string  get_logLevel      (void) const { return logLevel;       }
    std::string  get_cmdPipe       (void) const { return cmdPipe;        }
    size_t  get_logFileSizeMb         (void) const { return logFileSizeMb;  }
    size_t  get_logFileCount          (void) const { return logFileCount;   }
    bool get_logAsync              (void) const { return logAsync;       }
    
    //methods for parsing opt from cmd
    bool set_terminated            (bool        new_val);
    bool set_daemonized            (bool        new_val);
    bool set_no_chdir              (bool        new_val);
    bool set_no_fork               (bool        new_val);
    bool set_no_close_stdio        (bool        new_val);
    bool set_pidFile               (std::string new_val);
    bool set_logFile               (std::string new_val);
    bool set_logLevel              (std::string new_val);
    bool set_cmdPipe               (std::string new_val);
    bool set_logFileSizeMb         (size_t      new_val);
    bool set_logFileCount          (size_t      new_val);
    bool set_logAsync              (bool        new_val);

    std::string get_str_err()  const { return str_err;         }
    const char* get_cstr_err() const { return str_err.c_str(); }
    
    void clear(void);
    bool is_valid(void) const;


private:

    bool terminated{false};
    bool daemonized{false};
    bool no_chdir{false};
    bool no_fork{false};
    bool no_close_stdio{true};

    std::string pidFile{""};
    std::string logFile{""};
    std::string logLevel{""};
    std::string cmdPipe{""};

    size_t logFileSizeMb{0};
    size_t logFileCount{0};
    bool logAsync{false};    
    
    std::string  str_err;
};
                            

class Daemon
{
public:

    int redirect_stdio_to_devnull(void);
    int create_pid_file(std::string pid_file_name);
    void daemon_error_exit(const char *format, ...);
    void exit_if_not_daemonized(int exit_status);
    void do_fork();

    typedef void (*signal_handler_t) (int);

    void set_sig_handler(int signum, signal_handler_t handler);
    void daemonize2(void (*optional_init)(void *), void *data);
    bool SaveConfig(DaemonInfo& config);
    DaemonInfo GetDaemonInfo() { return daemonConfig; }
    
    std::string get_str_err() const { return str_err;         }
    const char* get_cstr_err()const { return str_err.c_str(); }    

private:
    DaemonInfo daemonConfig;
    std::string  str_err;
    
};







#endif //DAEMON_HEADER
