#include <AUILib.h>

#include "ProcessList.h"

namespace cg {
  ProcessDescr::ProcessDescr(UNUSED std::string pidStr, UNUSED std::string path) {
    D3("===New descriptor");
    D3("%s %s", path.c_str(), pidStr.c_str());
    mPath = path;
    mPidStr = pidStr;
    try {
        mPid = std::stoll(mPidStr);
        D4("Value: %ld", mPid);
    } catch (const std::out_of_range& e) {
        E("Number too large for 64-bit int!")
    } catch (const std::invalid_argument& e) {
        E("Invalid input string!")
    }
  }

  INT64 ProcessDescr::Pid() {
    return mPid;
  }

  std::string ProcessDescr::PidStr() {
    return mPidStr;
  }

  ProcessDescr::~ProcessDescr() {
    D3()
    mPath.clear();
    mPidStr.clear();
  }

  void ProcessList::Update() {
    D1();
    struct dirent* de;
    DIR* dp = opendir("/proc");
    std::string dirName;
    std::string cmdlinePath;
    std::string processName;
    std::string processName2;
    std::ifstream cmdlineFile;
    UNUSED size_t nullPos = 0;
    UNUSED ProcessDescr *pd = nullptr;
    if(dp == nullptr) {
      E("Error opening /proc dir: %d", errno)
      return;
    }
    while((de = readdir(dp)) != nullptr) {
      if (std::string(de->d_name) == "." || std::string(de->d_name) == "..") {
        continue;
      }
      if(de->d_type == DT_DIR) {
        dirName = de->d_name;
        if (std::isdigit(de->d_name[0])) {
          cmdlinePath = "/proc/" + dirName + "/cmdline";
          cmdlineFile.open(cmdlinePath.c_str());
          std::getline(cmdlineFile, processName);
          if((UINT64)processName.size()){
            nullPos = processName.find('\0');
            if(nullPos != std::string::npos) {
              processName2 = processName.substr(0, nullPos);
            }
          }
          pd = new ProcessDescr(dirName, processName2);
          mProcs[pd->Pid()] = pd;
          cmdlineFile.close();
        }
      }
      else {
        D4("U %s", de->d_name)
      }
    }
    closedir(dp);
  }

  void ProcessList::Clear() {
    D1()
    for (auto& pair : mProcs) {
        delete pair.second;
    }
    mProcs.clear();
  }

  ProcessList::ProcessList() {
    D1()
    Update();
  }

  ProcessList::~ProcessList() {
    D1()
    Clear();
  }
}

