#ifndef PROCESSLIST_H_
#define PROCESSLIST_H_

#include <vector>
#include <AUILib.h>


namespace aui {
  class ProcessDescr {
    private:
      UINT64 mPid;
      std::string mPidStr;
      std::string mPath;
    protected:
    public:
      ProcessDescr(std::string pidStr, std::string path);
      UINT64 Pid();
      std::string PidStr();
      std::string Path();
      ~ProcessDescr();
  };

  class ProcessList {
    private:
      void RefreshProcesses();
      std::map<UINT64, ProcessDescr*> mProcs;
    protected:
    public:
      ProcessList();
      virtual ~ProcessList();
      void Add(ProcessDescr *pd);
      void Clear();
      std::vector<std::string*>* GetList();
  };
}

#endif
