#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <format>


/// <summary>
/// 簡単デバッグログ出力クラス
/// ログを出したいファイルに以下をインクルード
/// #include <DebugLog.h> 
/// ログは以下のように出す
/// int count = 10;
/// DebugLog::Log("count = {}", count);
/// 出力結果は以下
/// count = 10
/// デバッグテキストの吐き出し場所デフォルトで.slnと同じ階層
/// C++20以上でビルドしないとエラーが出る
/// C++20より下でビルドする場合はstd::vformat()部分を自作のフォーマット関数にすれば動く
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
    /* シングルトンインスタンス取得 */
    static DebugLog& GetInstance();
    void LogOutput(const std::string& message, LogType type);

private:
    std::ofstream ofs;

};