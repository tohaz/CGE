#ifndef PROCESSLIST_H_
#define PROCESSLIST_H_

namespace cg {
    class ProcessDescr {
      private:
        INT64 mPid;
        std::string mPidStr;
        std::string mPath;
      protected:
      public:
        ProcessDescr(std::string pidStr, std::string path);
        INT64 Pid();
        std::string PidStr();
        std::string Path();
        ~ProcessDescr();
    };

  class ProcessList {
    private:
      std::map<INT64, ProcessDescr*> mProcs;
    public:
      void Update();
      void Clear();
      ProcessList();
      std::size_t Size();
      virtual ~ProcessList();
  };

}




#endif
