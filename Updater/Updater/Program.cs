using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace Updater
{
    static class Program
    {
        /// <summary>
        /// 应用程序的主入口点。
        /// </summary>
        [STAThread]

        [DllImport("kernel32.dll")]
        static extern bool AttachConsole(int dwProcessId);
        private const int ATTACH_PARENT_PROCESS = -1;
        static void Main(string[] args)
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            var form = new Form1();
            if (args.Length >= 1) 
            {
                if(args[0] == "-output")
                {
                    AttachConsole(ATTACH_PARENT_PROCESS);
                    form.OutputIsNeedUpdate();
                    while (true)
                    {
                        Application.DoEvents();
                        if (form.CanExit)
                            return;
                    }
                }
                else if(args[0] == "-force")
                {
                    form.ForceUseSelf = true;
                }
                else if(args[0] == "-fixup")
                {
                    form.FixUp = true;
                }
            }
            
            Application.Run(form);
        }
    }
}
