#include "DebugLog.h"

DebugLog::DebugLog(){
    Init("Debug.txt");
}

DebugLog::~DebugLog(){
    
}

bool DebugLog::Init(const char* fileName)
{
    ofs = std::ofstream(fileName);
    if (!ofs)
    {
        std::cout << "ログシステム初期化に失敗しました。" << std::endl;
        return false;
    }

    std::cout << "ログテキスト生成or上書き成功 = " << fileName << std::endl;
    return true;
}

void DebugLog::LogOutput(const std::string& message, LogType type)
{
    switch (type)
    {
    case DebugLog::INFO_LOG:
    default:
        ofs << message << std::endl;
        std::cout <<  message << std::endl;
        break;

    case DebugLog::WARNING_LOG:
        ofs << "WARNING::" << message << std::endl;
        std::cout << "WARNING::" << message << std::endl;
        break;

    case DebugLog::ERROR_LOG:
        ofs << "ERROR::" << message << std::endl;
        std::cout << "ERROR::" << message << std::endl;
        break;
    }
}

/* シングルトンインスタンス取得 */
DebugLog& DebugLog::GetInstance()
{
    static DebugLog instance;
    return instance;
}