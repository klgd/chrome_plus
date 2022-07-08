std::wstring QuoteSpaceIfNeeded(const std::wstring &str)
{
    if (str.find(L' ') == std::wstring::npos)
        return std::move(str);

    std::wstring escaped(L"\"");
    for (auto c : str)
    {
        if (c == L'"')
            escaped += L'"';
        escaped += c;
    }
    escaped += L'"';
    return std::move(escaped);
}

std::wstring JoinArgsString(std::vector<std::wstring> lines, const std::wstring &delimiter)
{
    std::wstring text;
    bool first = true;
    for (auto &line : lines)
    {
        if (!first)
            text += delimiter;
        else
            first = false;
        text += QuoteSpaceIfNeeded(line);
    }
    return text;
}

bool IsExistsPortable()
{
    std::wstring path = GetAppDir() + L"\\portable";
    if (PathFileExists(path.data()))
    {
        return true;
    }
    return false;
}

bool IsNeedPortable()
{
    return true;
    static bool need_portable = IsExistsPortable();
    return need_portable;
}

std::wstring GetUserDataDir()
{
    std::wstring path = GetAppDir() + L"\\..\\Data";

    TCHAR temp[MAX_PATH];
    ::PathCanonicalize(temp, path.data());

    return temp;
}
std::wstring GetDiskCacheDir()
{
    std::wstring path = GetAppDir() + L"\\..\\Cache";

    TCHAR temp[MAX_PATH];
    ::PathCanonicalize(temp, path.data());

    return temp;
}

std::wstring GetExtensionsDir()
{
    std::wstring path = GetAppDir() + L"\\..\\Extensions";

    TCHAR temp[MAX_PATH];
    ::PathCanonicalize(temp, path.data());

    return temp;
}

std::wstring GetProfileExtensionsDir(std::wstring profile_name)
{
    std::wstring path = GetUserDataDir();
    path.append(L"\\").append(profile_name).append(L"\\Extensions");

    TCHAR temp[MAX_PATH];
    ::PathCanonicalize(temp, path.data());

    return temp;
}


// 构造新命令行
std::wstring GetCommand(LPWSTR param)
{
    std::vector <std::wstring> args;

    int argc;
    LPWSTR* argv = CommandLineToArgvW(param, &argc);

    int insert_pos = 0;
    for (int i = 0; i < argc; i++)
    {
        if (wcscmp(argv[i], L"--") == 0)
        {
            break;
        }
        if (wcscmp(argv[i], L"--single-argument") == 0)
        {
            break;
        }
        insert_pos = i;
    }

    // std::wstring profileName = L"Default";

    for (int i = 0; i < argc; i++)
    {
        // 保留原来参数
        if(i)
            args.push_back(argv[i]);

        // if (wcsstr(argv[i], L"--profile-directory=")) {
        //     wchar_t* arg = argv[i];
        //     wchar_t delim[] = L"="; 
        //     wchar_t* pt;
        //     wchar_t* tmpArg = wcstok(arg, delim, &pt);
        //     tmpArg = wcstok(NULL, delim, &pt);
        //     profileName = tmpArg;
        // }

        // 追加参数
        if (i == insert_pos)
        {
            args.push_back(L"--shuax");
            args.push_back(L"--disable-quic");

            // args.push_back(L"--force-local-ntp");
            // args.push_back(L"--disable-background-networking");

            args.push_back(L"--disable-features=RendererCodeIntegrity,FlashDeprecationWarning,SidePanel");

            //if (IsNeedPortable())
            {
                auto diskcache = GetDiskCacheDir();

                wchar_t temp[MAX_PATH];
                wsprintf(temp, L"--disk-cache-dir=%s", diskcache.c_str());
                args.push_back(temp);
            }
            {
                auto userdata = GetUserDataDir();

                wchar_t temp[MAX_PATH];
                wsprintf(temp, L"--user-data-dir=%s", userdata.c_str());
                args.push_back(temp);
            }

            auto extPath = GetExtensionsDir();
            // auto proExtPath = GetProfileExtensionsDir(profileName);

            std::vector<std::wstring> extList;

            GetExtensionsList(extPath.c_str(), extList);
            // GetExtensionsList(proExtPath.c_str(), extList);

            if (extList.size()) {
                std::wstring text;
                bool first = true;
                for (auto &line : extList)
                {
                    if (!first)
                        text += ',';
                    else
                        first = false;
                    text += line;
                }
                
                // WriteLog(L"extList: %s", text.c_str());

                wchar_t temp[800];
                wsprintf(temp, L"--load-extension=%s", text.c_str());
                // WriteLog(L"extList: %s", temp);
                args.push_back(temp);
            }
        }
    }
    LocalFree(argv);

    return JoinArgsString(args, L" ");;
}


void Portable(LPWSTR param)
{
    wchar_t path[MAX_PATH];
    ::GetModuleFileName(NULL, path, MAX_PATH);

    std::wstring args = GetCommand(param);

    SHELLEXECUTEINFO sei = { 0 };
    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
    sei.lpVerb = L"open";
    sei.lpFile = path;
    sei.nShow = SW_SHOWNORMAL;

    sei.lpParameters = args.c_str();
    if (ShellExecuteEx(&sei))
    {
        ExitProcess(0);
    }
}
