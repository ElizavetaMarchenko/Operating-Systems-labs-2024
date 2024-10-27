#pragma once
#include <string>
#include <chrono>

using namespace std;

class My_Daemon
{
public:
    static My_Daemon& get_one();
    void terminate();
    void config();
    void run();
    void initialize(const string& config_local);
private:
    My_Daemon() {};
    My_Daemon(const My_Daemon&) = delete;
    My_Daemon& operator=(const My_Daemon&) = delete;
    void start();
    void stop();

    const string key_file = "dont.erase";
    const chrono::seconds defoult_update_time = chrono::seconds(5);
    const string pid_path = "/var/run/lab1_daemon.pid";
    chrono::seconds update_time;
    bool is_Terminate = false;
    string abs_path;
    string dir_path;
};
