using System.Runtime.InteropServices; // 用 DllImport 需用此 命名空间
using System;

namespace Updater
{
    /// <summary>
    /// 参数传递方式枚举 ,ByValue 表示值传递 ,ByRef 表示址传递
    /// </summary>
    public enum ModePass
    {
        ByValue = 0x0001,
        ByRef = 0x0002
    }

    class DllLoader
    {
        /// <summary>
        /// 原型是 :HMODULE LoadLibrary(LPCTSTR lpFileName);
        /// </summary>
        /// <param name="lpFileName">DLL 文件名 </param>
        /// <returns> 函数库模块的句柄 </returns>
        [DllImport("kernel32.dll")]
        static extern IntPtr LoadLibrary(string lpFileName);
        /// <summary>
        /// 原型是 : FARPROC GetProcAddress(HMODULE hModule, LPCWSTR lpProcName);
        /// </summary>
        /// <param name="hModule"> 包含需调用函数的函数库模块的句柄 </param>
        /// <param name="lpProcName"> 调用函数的名称 </param>
        /// <returns> 函数指针 </returns>
        [DllImport("kernel32.dll")]
        static extern IntPtr GetProcAddress(IntPtr hModule, string lpProcName);
        /// <summary>
        /// 原型是 : BOOL FreeLibrary(HMODULE hModule);
        /// </summary>
        /// <param name="hModule"> 需释放的函数库模块的句柄 </param>
        /// <returns> 是否已释放指定的 Dll</returns>
        [DllImport("kernel32", EntryPoint = "FreeLibrary", SetLastError = true)]
        static extern bool FreeLibrary(IntPtr hModule);
        /// <summary>
        /// Loadlibrary 返回的函数库模块的句柄
        /// </summary>
        private IntPtr hModule = IntPtr.Zero;
        /// <summary>
        /// GetProcAddress 返回的函数指针
        /// </summary>
        private IntPtr farProc = IntPtr.Zero;

        /// <summary>
        /// 装载 Dll
        /// </summary>
        /// <param name="lpFileName">DLL 文件名 </param>
        public void LoadDll(string lpFileName)
        {
            hModule = LoadLibrary(lpFileName);
        }

        public bool IsLoaded
        {
            get
            {
                return hModule != IntPtr.Zero;
            }
        }

        public DllLoader(string lpFileName)
        {
            LoadDll(lpFileName);
        }

        /// <summary>
        /// 获得函数指针
        /// </summary>
        /// <param name="lpProcName"> 调用函数的名称 </param>
        public void LoadFunction(string lpProcName)
        { // 若函数库模块的句柄为空，则抛出异常
            if (hModule == IntPtr.Zero)
                throw (new Exception(" 函数库模块的句柄为空 , 请确保已进行 LoadDll 操作 !"));
            // 取得函数指针
            farProc = GetProcAddress(hModule, lpProcName);
            // 若函数指针，则抛出异常
            if (farProc == IntPtr.Zero)
                throw (new Exception(" 没有找到 :" + lpProcName + " 这个函数的入口点 "));
        }

        /// <summary>
        /// 卸载 Dll
        /// </summary>
        public void UnloadDll()
        {
            FreeLibrary(hModule);
            hModule = IntPtr.Zero;
            farProc = IntPtr.Zero;
        }
    }
}
