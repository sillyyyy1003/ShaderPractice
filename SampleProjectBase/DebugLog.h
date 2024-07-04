#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <format>


/// <summary>
/// �ȒP�f�o�b�O���O�o�̓N���X
/// ���O���o�������t�@�C���Ɉȉ����C���N���[�h
/// #include <DebugLog.h> 
/// ���O�͈ȉ��̂悤�ɏo��
/// int count = 10;
/// DebugLog::Log("count = {}", count);
/// �o�͌��ʂ͈ȉ�
/// count = 10
/// �f�o�b�O�e�L�X�g�̓f���o���ꏊ�f�t�H���g��.sln�Ɠ����K�w
/// C++20�ȏ�Ńr���h���Ȃ��ƃG���[���o��
/// C++20��艺�Ńr���h����ꍇ��std::vformat()����������̃t�H�[�}�b�g�֐��ɂ���Γ���
/// </summary>

class DebugLog
{
    DebugLog();
    ~DebugLog();
public:

    enum LogType {
        INFO_LOG    = 0,
        WARNING_LOG = 1,
        ERROR_LOG   = 2,
    };

public:
    bool Init(const char* fileName);

    template<class... Args>
    static void Log(std::string_view rt_fmt_str, Args && ... args)
    {
        std::string str = std::vformat(rt_fmt_str, std::make_format_args(args...));
        DebugLog::GetInstance().LogOutput(str, INFO_LOG);
    }

    template<class... Args>
    static void LogWarning(std::string_view rt_fmt_str, Args && ... args)
    {
        std::string str = std::vformat(rt_fmt_str, std::make_format_args(args...));
        DebugLog::GetInstance().LogOutput(str, WARNING_LOG);
    }

    template<class... Args>
    static void LogError(std::string_view rt_fmt_str, Args && ... args)
    {
        std::string str = std::vformat(rt_fmt_str, std::make_format_args(args...));
        DebugLog::GetInstance().LogOutput(str, ERROR_LOG);
    }

private:
    /* �V���O���g���C���X�^���X�擾 */
    static DebugLog& GetInstance();
    void LogOutput(const std::string& message, LogType type);

private:
    std::ofstream ofs;

};