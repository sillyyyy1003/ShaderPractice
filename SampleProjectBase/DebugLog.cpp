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
        std::cout << "���O�V�X�e���������Ɏ��s���܂����B" << std::endl;
        return false;
    }

    std::cout << "���O�e�L�X�g����or�㏑������ = " << fileName << std::endl;
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

/* �V���O���g���C���X�^���X�擾 */
DebugLog& DebugLog::GetInstance()
{
    static DebugLog instance;
    return instance;
}