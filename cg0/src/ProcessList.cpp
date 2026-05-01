#include "ProcessList.h"

namespace aui {
  ProcessDescr::ProcessDescr(std::string pidStr, std::string path) {
    mPath = path;
    mPidStr = pidStr;
    try {
        mPid = std::stol(pidStr);
    } catch (const std::invalid_argument& e) {
      E("invalid argument")
    } catch (const std::out_of_range& e) {
      E("out of range")
    }
    D3("new pd %lu(%s) %s", mPid, mPidStr.c_str(), mPath.c_str())
  }

  std::string ProcessDescr::PidStr() {
    return mPidStr;
  }

  std::string ProcessDescr::Path() {
    return mPath;
  }

  UINT64 ProcessDescr::Pid() {
    return mPid;
  }

  ProcessDescr::~ProcessDescr() {
    D3()
    mPath.clear();
    mPidStr.clear();
  }

  ProcessList::ProcessList() {
    RefreshProcesses();
  }

  void ProcessList::Add(ProcessDescr *pd) {
    if(!mProcs.contains(pd->Pid())) {
      mProcs[pd->Pid()] = pd;
    }
    else {
      E("value already exists")
    }
  }

  void ProcessList::Clear() {
    for(auto const& [key, val] : mProcs) {
      delete val;
    }
    mProcs.clear();
  }


  void ProcessList::RefreshProcesses() {
    D3()
    DIR* dp = opendir("/proc");
    if(dp == nullptr) {
      E("Error opening /proc dir: %d", errno)
      return;
    }
    bool is_pid;
    struct dirent* de;
    std::string cmdline_path;
    std::string process_name;
    size_t null_pos;
    std::string dir_name;
    std::ifstream cmdline_file;
    ProcessDescr* pd = 0;
    while((de = readdir(dp)) != nullptr) {
      if(de->d_type == DT_DIR) {
        dir_name = de->d_name;
        is_pid = true;
        for(char const &c : dir_name) {
          if(!isdigit(c)) {
            is_pid = false;
            break;
          }
        }
        if(is_pid) {
          cmdline_path = "/proc/" + dir_name + "/cmdline";
          cmdline_file.open(cmdline_path.c_str());
          if(cmdline_file) {
            std::getline(cmdline_file, process_name);
            null_pos = process_name.find('\0');
            if(null_pos != std::string::npos) {
              process_name = process_name.substr(0, null_pos);
            }
            if(process_name.empty()) {
              process_name = "[kp]";
            }
          }
          else {
            process_name = "[unknown]";
          }
          D3("PID: %s Name: %s", dir_name.c_str(), process_name.c_str());
          cmdline_file.close();
          pd = new ProcessDescr(dir_name, process_name);
          Add(pd);
        }
      }
    }
    closedir(dp);
  }

  std::vector<std::string*>* ProcessList::GetList() {
    std::vector<std::string*>* v = new std::vector<std::string*>;
    std::string* pstr;
    ProcessDescr* pd;
    for (auto const& [key, val] : mProcs) {
      pd = (ProcessDescr*)val;
      pstr = new std::string;
      *pstr = " (" + pd->PidStr() + ") " + pd->Path();
      v->push_back(pstr);
    }
    std::reverse(v->begin(), v->end());
    return v;
  }

  ProcessList::~ProcessList() {
    D3()
    Clear();
  }

}

