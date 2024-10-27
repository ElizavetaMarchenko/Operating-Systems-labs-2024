#include "daemon.hpp"
#include <fstream>
#include <filesystem>
#include <unistd.h>
#include <thread>
#include <syslog.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <csignal>

using namespace std;

static void ReceiveSignal(int signal)
{
    switch (signal)
    {
    case SIGHUP:
    {
        syslog(LOG_INFO, "Reload config");
        My_Daemon::get_one().config();
        break;
    }
    case SIGTERM:
    {
        syslog(LOG_INFO, "Terminate");
        My_Daemon::get_one().terminate();
        break;
    }
    default:
        break;
    }
}

void Deleter(const string& directory, const string& key_file)
{
    if (filesystem::exists(directory + '/' + key_file))
    {
        return;
    }
    for (auto& file : filesystem::directory_iterator(directory))
    {
        if (!filesystem::remove(file))
        {
            syslog(LOG_WARNING, "Removed was failed: \"%s\"", file.path().c_str());
        }
    }
}

void My_Daemon::start()
{
    syslog(LOG_INFO, "Create daemon");

    pid_t pid = fork();
    if (pid < 0)
    {
        syslog(LOG_ERR, "fork error");
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    umask(0);
    if (setsid() < 0)
    {
        syslog(LOG_ERR, "Error: Group setting");
        exit(EXIT_FAILURE);
    }
    if (chdir("/") < 0)
    {
        syslog(LOG_ERR, "Error: Switching to root dir");
        exit(EXIT_FAILURE);
    }

    for (int tmp = sysconf(_SC_OPEN_MAX); tmp >= 0; --tmp)
    {
        close(tmp);
    }
    int devNull = open("/dev/null", O_RDWR);
    dup2(devNull, STDIN_FILENO);
    dup2(devNull, STDOUT_FILENO);
    dup2(devNull, STDERR_FILENO);
    syslog(LOG_INFO, "end of daemon create");
}

void My_Daemon::stop()
{
    unsigned int living_daemon_pid = 0;
    syslog(LOG_INFO, "search living daemon");
    ifstream pid_file(pid_path);

    if (pid_file.is_open() &&
        pid_file >> living_daemon_pid && kill(living_daemon_pid, 0) == 0)
    {
        syslog(LOG_WARNING, "found living daemon. PID: %i", living_daemon_pid);
        kill(living_daemon_pid, SIGTERM);
    }
}

static My_Daemon& My_Daemon::get_one()
{
    static My_Daemon daemon;
    return daemon;
}

void My_Daemon::terminate()
{
    is_Terminate = true;
    closelog();
}

void My_Daemon::config()
{
    unsigned int update_seconds = 0;
    ifstream config_file(abs_path);
    if (!config_file.is_open())
    {
        syslog(LOG_ERR, "Invalid config");
        exit(EXIT_FAILURE);
    }
    if (!getline(config_file, dir_path))
    {
        syslog(LOG_ERR, "Directory path reading error");
        exit(EXIT_FAILURE);
    }

    if (!(config_file >> update_seconds))
    {
        syslog(LOG_WARNING, "Update time reading error");
        update_time = defoult_update_time;
    }
    else
    {
        update_time = chrono::seconds(update_seconds);
    }
}

void My_Daemon::run()
{
    while (!is_Terminate)
    {
        if (filesystem::is_directory(dir_path))
        {
            Deleter(dir_path, key_file); 
        }
        else
        {
            syslog(LOG_WARNING, "Not found directory");
        }
        this_thread::sleep_for(update_time);
    }
}

void My_Daemon::initialize(const string& config_local)
{
    abs_path = filesystem::absolute(config_local);
    openlog("My_Daemon", LOG_NDELAY | LOG_PID | LOG_PERROR, LOG_USER);
    syslog(LOG_INFO, "Daemon initialization");
    stop();
    start();

    syslog(LOG_INFO, "Writing pid");
    ofstream pid_file(pid_path.c_str());
    if (!pid_file.is_open())
    {
        syslog(LOG_ERR, "write pid error");
        exit(EXIT_FAILURE);
    }
    pid_file << getpid();

    signal(SIGHUP, ReceiveSignal);
    signal(SIGTERM, ReceiveSignal);
    config();
    syslog(LOG_INFO, "Initialize end");
}
