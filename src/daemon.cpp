
/*
 * daemon.cpp
 *
 *
 * version 1.2
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

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>


#include "daemon.hpp"
/*
 *  How can I execute a new process from GNOME Terminal
 *  so that the child process's parent PID becomes 1 and not
 *  the PID of the ubuntu session init process?
 *
 *  This is intentionally hard.
 *  Service managers want to keep track of orphaned child processes.
 *  They want not to lose them to process #1.
 *  Stop trying to do that!!!
 *
 *
 *  If you are asking solely because you think that your process (daemon)
 *  ought to have a parent process ID of 1, then wean yourself off this idea.
 *
 *
 *  Therefore, modern Linux system does not correctly determine the fact of
 *  daemonized through the function:
 *
 *  if( getppid() == 1 )
 *      return 1;        //already a daemon
 *
 *  We uses our flag daemonized in daemon_info_t
 */


// Daemon Info Access Functions
/*******************************************************************************
 * Access function to set the "terminated" variable within DaemonInfo
 *
 * @param new_val The new value of "terminated" which is to be saved
 * @return true if new value was successfully set
 ******************************************************************************/
bool DaemonInfo::set_terminated(bool new_val)
{
    terminated = new_val;
    return true;
}


/*******************************************************************************
 * Access function to set the "daemonized" variable within DaemonInfo
 *
 * @param new_val The new value of "daemonized" which is to be saved
 * @return true if new value was successfully set
 ******************************************************************************/
bool DaemonInfo::set_daemonized(bool new_val)
{
    daemonized = new_val;
    return true;
}


/*******************************************************************************
 * Access function to set the "no_chdir" variable within DaemonInfo
 *
 * @param new_val The new value of "no_chdir" which is to be saved
 * @return true if new value was successfully set
 ******************************************************************************/
bool DaemonInfo::set_no_chdir(bool new_val)
{
    no_chdir = new_val;
    return true;
}


/*******************************************************************************
 * Access function to set the "no_fork" variable within DaemonInfo
 *
 * @param new_val The new value of "no_fork" which is to be saved
 * @return true if new value was successfully set
 ******************************************************************************/
bool DaemonInfo::set_no_fork(bool new_val)
{
    no_fork = new_val;
    return true;
}


/*******************************************************************************
 * Access function to set the "no_close_stdio" variable within DaemonInfo
 *
 * @param new_val The new value of "no_close_stdio" which is to be saved
 * @return true if new value was successfully set
 ******************************************************************************/
bool DaemonInfo::set_no_close_stdio(bool new_val)
{
    no_close_stdio = new_val;
    return true;
}


/*******************************************************************************
 * Access function to set the "pidFile" variable within DaemonInfo
 *
 * @param new_val The new value of "pidFile" which is to be saved
 * @return true if new value was successfully set, false if new_val is empty
 ******************************************************************************/
bool DaemonInfo::set_pidFile(std::string new_val)
{
    if(new_val.empty())
    {
        str_err = "pidFile is empty";
        return false;
    }

    pidFile = new_val;
    return true;    
}


/*******************************************************************************
 * Access function to set the "logFile" variable within DaemonInfo
 *
 * @param new_val The new value of "logFile" which is to be saved
 * @return true if new value was successfully set, false if new_val is empty
 ******************************************************************************/
bool DaemonInfo::set_logFile(std::string new_val)
{
    if(new_val.empty())
    {
        str_err = "pidFile is empty";
        return false;
    }

    logFile = new_val;
    return true;    
}


/*******************************************************************************
 * Access function to set the "logLevel" variable within DaemonInfo
 *
 * @param new_val The new value of "logLevel" which is to be saved
 * @return true if new value was successfully set, false if new_val is empty
 ******************************************************************************/
bool DaemonInfo::set_logLevel(std::string new_val)
{
    if(new_val.empty())
    {
        str_err = "logLevel is empty";
        return false;
    }

    logLevel = new_val;
    return true;    
}


/*******************************************************************************
 * Access function to set the "cmdPipe" variable within DaemonInfo
 *
 * @param new_val The new value of "cmdPipe" which is to be saved
 * @return true if new value was successfully set, false if new_val is empty
 ******************************************************************************/
bool DaemonInfo::set_cmdPipe(std::string new_val)
{
    if(new_val.empty())
    {
        str_err = "cmdPipe is empty";
        return false;
    }

    cmdPipe = new_val;
    return true;    
}


/*******************************************************************************
 * Access function to set the "logFileSizeMb" variable within DaemonInfo
 *
 * @param new_val The new value of "logFileSizeMb" which is to be saved
 * @return true if new value was successfully set, false if new_val is empty
 ******************************************************************************/
bool DaemonInfo::set_logFileSizeMb(size_t new_val)
{
    logFileSizeMb = new_val;
    return true;
}


/*******************************************************************************
 * Access function to set the "logFileCount" variable within DaemonInfo
 *
 * @param new_val The new value of "logFileCount" which is to be saved
 * @return true if new value was successfully set, false if new_val is empty
 ******************************************************************************/
bool DaemonInfo::set_logFileCount(size_t new_val)
{
    logFileCount = new_val;
    return true;
}


/*******************************************************************************
 * Access function to set the "logAsync" variable within DaemonInfo
 *
 * @param new_val The new value of "logAsync" which is to be saved
 * @return true if new value was successfully set, false if new_val is empty
 ******************************************************************************/
bool DaemonInfo::set_logAsync(bool new_val)
{
    logAsync = new_val;
    return true;
}


/*******************************************************************************
 * Validation function to check the private values stored in DaemonInfo
 *
 * Ideally all the settings stored within DaemonInfo should be checked at this
 * point, however the daemon can still function correctly with some empty
 * strings and it's not easy to check the validity of boolean values.
 * Intergers can be checked using range validation if valid ranges are provided.
 *
 * @return true if pidFile and logLevel are holding valies, false if emptu
 ******************************************************************************/
bool DaemonInfo::is_valid() const
{
    return ( !pidFile.empty()   &&
             !logLevel.empty()  );
}


/*******************************************************************************
 * Clear function to reset the status of all settings within DaemonInfo
 *
 * This function is to be used when the user wishes to reset the daemon 
 * configuration for any particular reason.
 ******************************************************************************/
void DaemonInfo::clear()
{
    pidFile.clear();
    logFile.clear();
    logLevel.clear();
    cmdPipe.clear();
    logFileSizeMb = 0;
    logFileCount = 0;
    
    terminated = false;
    daemonized = false;
    no_chdir = false;
    no_fork = false;
    no_close_stdio = false;
    logAsync = false;
}


// Daemon Control and Management Functions
/*******************************************************************************
 * Save daemon configuration from DaemonInfo class into Daemon class for used
 *
 * After initial configuration has completed, the settings will be stored within
 * an instance of the Daemon class and used throughout the operation of the 
 * daemon.  If the configuration has been populated correctly it will saved
 *
 * @param config An instance of the DaemonInfo class which has been populated
 * @return true if the configuration is valid, and false if invalid.
 ******************************************************************************/
bool Daemon::SaveConfig(DaemonInfo& config)
{
    if( !config.is_valid() )
    {
        str_err = "daemon configuration has unset parameters";
        return false;
    }

    daemonConfig = config;
    return true;
}


/*******************************************************************************
 * Exit the application if it was unable to daemonise
 *
 * @param exit_status The exit code to tell the OS why we have exited.
 ******************************************************************************/
void Daemon::exit_if_not_daemonized(int exit_status)
{
    if( !daemonConfig.get_daemonized() )
        _exit(exit_status);
}


/*******************************************************************************
 * Exit the application and provide a reason why
 *
 * @param format A string(s)explainig why the application was exited.
 ******************************************************************************/
void Daemon::daemon_error_exit(const char *format, ...)
{
    va_list ap;

    if( format &&  *format )
    {
        va_start(ap, format);
        fprintf(stderr, "%s: ", "onvif_srvd");
        vfprintf(stderr, format, ap);
        va_end(ap);
    }

    _exit(EXIT_FAILURE);
}


/*******************************************************************************
 * Set the signal handler
 *
 * The daemon needs to know what to do if a signal is sent from the OS to the
 * daemon, this function configures that functionality.
 *
 * @param signum The signal number which has been sent @see signum-generic.h
 * @param handler A function call to deal with the signal based on the severity
 ******************************************************************************/
void Daemon::set_sig_handler(int signum, signal_handler_t handler)
{
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    if( sigaction(signum, &sa, NULL) != 0 )
        daemon_error_exit("Can't set handler for signal: %d %m\n", signum);
}


/*******************************************************************************
 * Redirect STDOUT to /dev/null
 *
 * When a daemon is running in the background, it is usually not a good idea to
 * print anything to the terminal as it can get messy, and if it is being ran
 * by systemctl then there isn't really a terminal to print to.  
 *
 * This setting is handy for those times you are debugging and really want to
 * be able to quickly print to the terminal.
 ******************************************************************************/
int Daemon::redirect_stdio_to_devnull(void)
{
    int fd;

    fd = open("/dev/null", O_RDWR);
    if(fd == -1)
        return -1; //error can't open file

    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);

    if(fd > 2)
        close(fd);

    return 0; //good job
}


/*******************************************************************************
 * Create a pid file to store the PID number while the daemon runs
 *
 * This function will create a file in a specified location that holds the PID
 * number of the daemon, it will be used by systemctl to terminate the process
 * if it needs to.
 *
 * @param pid_file_name A string containing a path to the file
 ******************************************************************************/
int Daemon::create_pid_file(std::string pid_file_name)
{
    int fd;
    const int BUF_SIZE = 32;
    char buf[BUF_SIZE];

    if( pid_file_name.empty() )
    {
        errno = EINVAL;
        return -1;
    }

    fd = open(pid_file_name.c_str(), O_RDWR | O_CREAT, 0644);
    if(fd == -1)
        return -1; // Could not create on PID file

    if( lockf(fd, F_TLOCK, 0) == -1 )
    {
        close(fd);
        return -1; // Could not get lock on PID file
    }

    if( ftruncate(fd, 0) != 0 )
    {
        close(fd);
        return -1; // Could not truncate on PID file
    }

    snprintf(buf, BUF_SIZE, "%ld\n", (long)getpid());
    if( write(fd, buf, strlen(buf)) != (int)strlen(buf) )
    {
        close(fd);
        return -1; // Could not write PID to PID file
    }

    return fd; //good job
}


/*******************************************************************************
 * Tell the daemon to run as a background process
 *
 * This function will attempt to fork the daemon, if unsuccessful the daemon
 * will exit.
 ******************************************************************************/
void Daemon::do_fork()
{
    switch( fork() )                                     // Become background process
    {
        case -1:  daemon_error_exit("Can't fork: %m\n");
        case  0:  break;                                 // child process (go next)
        default:  _exit(EXIT_SUCCESS);                   // We can exit the parent process
    }

    // ---- At this point we are executing as the child process ----
}


/*******************************************************************************
 * Main function of the Daemon class
 *
 * This function will handle the daemonise attempt
 *
 * @parm (*optional_init)(void *) Pointer to function to daemonise
 * @param data Any extra data the daemon will need to run.
 ******************************************************************************/
void Daemon::daemonize2(void (*optional_init)(void *), void *data)
{
    if( !daemonConfig.get_no_fork() )
        do_fork();

    // Reset the file mode mask
    umask(0);

    // Create a new process group(session) (SID) for the child process
    // call setsid() only if fork is done
    if( !daemonConfig.get_no_fork() && (setsid() == -1) )
        daemon_error_exit("Can't setsid: %m\n");

    // Change the current working directory to "/"
    // This prevents the current directory from locked
    // The demon must always change the directory to "/"
    if( !daemonConfig.get_no_chdir() && (chdir("/") != 0) )
        daemon_error_exit("Can't chdir: %m\n");

    if( daemonConfig.get_pidFile().empty() && (create_pid_file(daemonConfig.get_pidFile()) == -1) )
        daemon_error_exit("Can't create pid file: %s: %m\n", daemonConfig.get_pidFile().c_str());

    // call user functions for the optional initialization
    // before closing the standardIO (STDIN, STDOUT, STDERR)
    if( optional_init )
        optional_init(data);

    if( !daemonConfig.get_no_close_stdio() && (redirect_stdio_to_devnull() != 0) )
        daemon_error_exit("Can't redirect stdio to /dev/null: %m\n");

    daemonConfig.set_daemonized(true); //good job
}
