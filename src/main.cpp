#include "ChunkFile.h"
#include "Exceptions.h"
#include <list>
#include <vector>
using namespace std;

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#include <shlwapi.h>
#include <shellapi.h>
#include "resource.h"

static wstring GetWindowStr(HWND hWnd)
{
    int len = GetWindowTextLength(hWnd);
    TCHAR* buf = new TCHAR[len + 1];
    GetWindowText(hWnd, buf, len + 1);
    wstring text = buf;
    delete[] buf;
    return text;
}

static wstring LoadResourceString(UINT uID)
{
    LPCTSTR str = TEXT("");
    int n = LoadString(NULL, uID, (LPTSTR)&str, 0);
    return wstring(str, n);
}

class ChunkFile
{
    class Chunk
    {
        typedef list<pair<ChunkType, Chunk*> > ChunkList;

        ChunkList m_children;
        char*     m_data;
        size_t    m_size;

    public:
        void write(ChunkWriter& writer) const
        {
            if (m_data != NULL)
            {
                writer.write(m_data, m_size);
            }
            else
            {
                for (ChunkList::const_iterator p = m_children.begin(); p != m_children.end(); ++p)
                {
                    writer.beginChunk(p->first);
                    p->second->write(writer);
                    writer.endChunk();
                }
            }
        }

        bool writeAndDeletePreview(const wstring& path)
        {
            for (ChunkList::iterator p = m_children.begin(); p != m_children.end(); ++p)
            {
                if (p->first == 0x13)
                {
                    if (p->second->m_data != NULL)
                    {
                        PhysicalFile file(path.c_str(), FILEMODE_WRITE);
                        file.write(p->second->m_data, p->second->m_size);
                        delete p->second;
                        m_children.erase(p);
                        return true;
                    }
                    break;
                }
            }
            return false;
        }

        Chunk(ChunkReader& reader)
            : m_data(NULL)
        {
            if (reader.group())
            {
                ChunkType type;
                while ((type = reader.next()) != -1)
                {
                    m_children.push_back(make_pair(type, (Chunk*)NULL));
                    m_children.back().second = new Chunk(reader);
                }
            }
            else
            {
                m_data = new char[m_size = reader.size()];
                reader.read(m_data, m_size);
            }
        }
        
        ~Chunk()
        {
            for (ChunkList::const_iterator p = m_children.begin(); p != m_children.end(); ++p)
            {
                delete p->second;
            }
            delete[] m_data;
        }
    };

    Chunk m_root;

public:
    bool writeAndDeletePreview(const wstring& path)
    {
        // Find the preview
        return m_root.writeAndDeletePreview(path);
    }

    void write(ChunkWriter& writer) const
    {
        m_root.write(writer);
    }

    ChunkFile(ChunkReader& reader)
        : m_root(reader)
    {
    }
};

static void ExtractPreview(const wstring& dest, const wstring& src)
{
    ChunkFile* file = NULL;

    try
    {
        {
            // Read the file
            PhysicalFile fSrc(src, FILEMODE_READ);
            ChunkReader reader(fSrc);
            file = new ChunkFile(reader);
        }

        if (!file->writeAndDeletePreview(dest))
        {
            wstring text = LoadResourceString(IDS_NO_PREVIEW);
            MessageBox(NULL, text.c_str(), NULL, MB_OK | MB_ICONHAND);
        }
        else
        {
            // Write the new file
            PhysicalFile fSrc(src, FILEMODE_WRITE);
            ChunkWriter writer(fSrc);
            file->write(writer);
        }
    }
#ifdef NDEBUG
    catch (exception& e)
    {
		MessageBoxA(NULL, e.what(), NULL, MB_OK );
    }
    catch (wexception& e)
    {
		MessageBoxW(NULL, e.what(), NULL, MB_OK );
    }
#endif
    catch (...)
    {
        delete file;
        throw;
    }
    delete file;
}

static wstring QueryOpenFilename(HWND hWndParent)
{
    TCHAR filebuf[MAX_PATH];
    filebuf[0] = '\0';

    wstring filter = LoadResourceString(IDS_FILES_MAP) + wstring(L" (*.ted)\0*.TED\0", 15)
                   + LoadResourceString(IDS_FILES_ALL) + wstring(L" (*.*)\0*.*\0", 11);

    OPENFILENAME ofn = {0};
    ofn.lStructSize  = sizeof(OPENFILENAME);
    ofn.hwndOwner    = hWndParent;
    ofn.lpstrFilter  = filter.c_str();
    ofn.nFilterIndex = 0;
    ofn.lpstrFile    = filebuf;
    ofn.nMaxFile     = MAX_PATH;
    ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	if (GetOpenFileName( &ofn ) != 0)
	{
        return filebuf;
	}
    return TEXT("");
}

static wstring QuerySaveFilename(HWND hWndParent, const wstring& defname)
{
    TCHAR filebuf[MAX_PATH];
    wcscpy_s(filebuf, MAX_PATH, defname.c_str());

    wstring filter = LoadResourceString(IDS_FILES_TGA) + wstring(L" (*.tga)\0*.TGA\0", 15)
                   + LoadResourceString(IDS_FILES_ALL) + wstring(L" (*.*)\0*.*\0", 11);

    OPENFILENAME ofn = {0};
    ofn.lStructSize  = sizeof(OPENFILENAME);
    ofn.hwndOwner    = hWndParent;
    ofn.lpstrFilter  = filter.c_str();
    ofn.lpstrDefExt  = L"ted";
    ofn.nFilterIndex = 0;
    ofn.lpstrFile    = filebuf;
    ofn.nMaxFile     = MAX_PATH;
    ofn.Flags        = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	if (GetSaveFileName( &ofn ) != 0)
	{
        return filebuf;
	}
    return TEXT("");
}

static INT_PTR CALLBACK MainDialogFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
            SetWindowText(GetDlgItem(hWnd, IDC_EDIT1), (LPCTSTR)lParam);
            if (lParam != NULL)
            {
                EnableWindow(GetDlgItem(hWnd, IDC_BUTTON2), TRUE);
            }
            break;

        case WM_COMMAND:
        {
            UINT code     = HIWORD(wParam);
            UINT idCtrl   = LOWORD(wParam);
            HWND hControl = (HWND)lParam;
            switch (code)
            {
                case EN_CHANGE:
                {
                    wstring path = GetWindowStr(hControl);
                    BOOL enable = PathFileExists(path.c_str()) && !PathIsDirectory(path.c_str());
                    EnableWindow(GetDlgItem(hWnd, IDC_BUTTON2), enable);
                    break;
                }

                case BN_CLICKED:
                    switch (idCtrl)
                    {
                        case IDC_BUTTON1: // Browse
                        {
                            wstring path = QueryOpenFilename(hWnd);
                            if (!path.empty())
                            {
                                SetWindowText(GetDlgItem(hWnd, IDC_EDIT1), path.c_str());

                                BOOL enable = PathFileExists(path.c_str()) && !PathIsDirectory(path.c_str());
                                EnableWindow(GetDlgItem(hWnd, IDC_BUTTON2), enable);
                            }
                            break;
                        }

                        case IDC_BUTTON2: // Extract Preview
                        {
                            wstring path = GetWindowStr(GetDlgItem(hWnd, IDC_EDIT1));
                            LPCTSTR filename  = PathFindFileName(path.c_str());
                            LPCTSTR extension = PathFindExtension(filename);

                            wstring dest = QuerySaveFilename(hWnd, wstring(filename, extension - filename));
                            if (!dest.empty())
                            {
                                ExtractPreview(dest, path);
                            }
                            break;
                        }

                        case IDOK:
                        case IDCANCEL:
                            EndDialog(hWnd, 0);
                            break;
                    }
                    break;
            }
            break;
        }
        
        case WM_DROPFILES:
        {
            HDROP hDrop = (HDROP)wParam;
            if (DragQueryFile(hDrop, -1, NULL, 0) > 0)
            {
                TCHAR path[MAX_PATH + 1];
                if (DragQueryFile(hDrop, 0, path, MAX_PATH) != 0)
                {
                    SetWindowText(GetDlgItem(hWnd, IDC_EDIT1), path);
                }
            }
            DragFinish(hDrop);
            break;
        }
    }
    return FALSE;
}

// Parse the command line into a argv-style vector
static vector<wstring> ParseCommandLine()
{
	int nArgs;
	vector<wstring> argv;

	LPTSTR *args = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (args != NULL)
	{
		for (int i = 0; i < nArgs; i++)
		{
			argv.push_back(args[i]);
		}
		LocalFree(args);
	}
	return argv;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
#ifndef NDEBUG
    // In debug mode we turn on memory checking
    _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

#ifdef NDEBUG
    // Only catch exceptions in release mode.
    // In debug mode, the IDE will jump to the source.
    try
#endif
    {
        // Filename to load
        LPCTSTR filename = NULL;

        vector<wstring> args = ParseCommandLine();
        if (args.size() > 1)
        {
            // Fill the 
            if (PathFileExists(args[1].c_str()))
            {
                filename = args[1].c_str();
            }
        }

        DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_MAINDIALOG), NULL, MainDialogFunc, (LPARAM)filename);
    }
#ifdef NDEBUG
    catch (exception& e)
    {
		MessageBoxA(NULL, e.what(), NULL, MB_OK );
    }
    catch (wexception& e)
    {
		MessageBoxW(NULL, e.what(), NULL, MB_OK );
    }
#endif

    return 0;
}