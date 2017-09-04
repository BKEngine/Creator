using System;
using System.Collections.Generic;
using System.IO;
using System.Net;
using System.Windows.Forms;
using System.Xml;
using System.Security.Cryptography;
using System.Collections;
using System.IO.Compression;
using System.Threading;
using System.Text;
using System.Diagnostics;
using System.Net.Security;
using System.Security.Cryptography.X509Certificates;

namespace Updater
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            this.FormClosing += Form1_FormClosing1;
            this.FixUp = false;
#if DEBUG
            this.ForceUseSelf = true;
#endif
            ServicePointManager.ServerCertificateValidationCallback += ValidateRemoteCertificate;
            //ServicePointManager.SecurityProtocol = SecurityProtocolType.Ssl3;
        }

        private bool ValidateRemoteCertificate(object sender, X509Certificate certificate, X509Chain chain, SslPolicyErrors error)
        {
            return true;
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            BeginUpdate();
        }

        private bool _canClose = false;
        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (!_canClose)
                e.Cancel = true;
            else
                Cancel();
        }

        public bool ForceUseSelf { get; set; }

        private static string HTTP_ADDRESS = "https://creator.up.bakery.moe/windows/";
        private static string INFO_FILE = "version";
        private WebClient _client = new MyWebClient();
        private string _version = "";
        private string _remoteVersion = "";
#if DEBUG
        private string _exeDir = Environment.CurrentDirectory + '/';
#else
        private string _exeDir = AppDomain.CurrentDomain.BaseDirectory;
#endif

        private Uri UriForFile(string path)
        {
            Uri uri = new Uri(HTTP_ADDRESS + path.Replace('\\','/'));
            return uri;
        }

        void Invoke(MethodInvoker a)
        {
            this.Invoke((Delegate)a);
        }

        public bool FixUp { get; set; }

        private void ReadVersionFile()
        {
            if(FixUp)
            {
                _version = "";
            }
            else
            {
                try
                {
                    _version = File.ReadAllText(Path.Combine(_exeDir, "version"));
                }
                catch (Exception)
                {
                    _version = "";
                }
            }
        }

        private void BeginUpdate()
        {
            ReadVersionFile();
            if (_version != null && _version != "")
            {
                textBox1.Text += "本地版本：" + _version + Environment.NewLine;
            }
            textBox1.Text += "获取更新信息……";
            ReadRemoteInfoFile();
        }

        private bool Output = false;
        public bool CanExit { get; set; }

        public void OutputIsNeedUpdate()
        {
            Output = true;
            ReadVersionFile();
            ReadRemoteInfoFile();
        }

        private void ReadRemoteInfoFile()
        {
            _client.DownloadStringCompleted += _client_DownloadStringCompleted;
            _client.DownloadStringAsync(UriForFile(INFO_FILE));
        }

        public void Cancel()
        {
            _client.CancelAsync();
        }

        private new void Close()
        {
            this._canClose = true;
            base.Close();
        }

        private void timer1_Tick(object sender, EventArgs e)
        {
            this.Close();
        }

        private void CloseAfter3Seconds()
        {
            System.Windows.Forms.Timer timer1 = new System.Windows.Forms.Timer();
            textBox1.Text += "更新程序将在3秒后退出。" + Environment.NewLine;
            timer1.Interval = 3000;
            timer1.Tick += timer1_Tick;
            timer1.Enabled = true;
        }

        private bool NeedVCRuntime = false;

        private void _client_DownloadStringCompleted(object sender, DownloadStringCompletedEventArgs e)
        {
            _client.DownloadStringCompleted -= _client_DownloadStringCompleted;
            if (e.Cancelled)
                return;
            if(Output)
            {
                CanExit = true;
                if(e.Error != null)
                {
                    Console.Out.WriteLine("Error:" + e.Error.Message);
                    return;
                }
                ParseInfoFile(e.Result);
                Console.Out.WriteLine(_remoteVersion != _version);
                return;
            }
            if(e.Error != null)
            {
                textBox1.Text += "失败。" + Environment.NewLine + e.Error.Message;
                CloseAfter3Seconds();
                return;
            }
            textBox1.Text += "成功。" + Environment.NewLine;
            if(!ParseInfoFile(e.Result))
            {
                textBox1.Text += "从服务器下载了一个错误的文件，请检查运营商是否被劫持，或者向我们报告。" + Environment.NewLine + Environment.NewLine;
                textBox1.Text += "以下是文件内容：" + Environment.NewLine + e.Result;
                return;
            }
            textBox1.Text += "服务器端版本：" + _remoteVersion + Environment.NewLine;
            if (!ForceUseSelf && _remoteVersion.Equals(_version))
            {
                textBox1.Text += "应用程序是最新版本，无需更新。将启动BKE_Creator。" + Environment.NewLine;
                Process.Start("BKE_Creator.exe");
                CloseAfter3Seconds();
                return;
            }
            JudgeNeedVCRuntime();
            ScanLocalFile();
        }

        private void JudgeNeedVCRuntime()
        {
            try
            {
                DllLoader loader = new DllLoader("msvcp120.dll");
                NeedVCRuntime = !loader.IsLoaded;
            }
            catch
            {
                NeedVCRuntime = true;
            }
        }

        private Dictionary<string, string> _remoteFiles = new Dictionary<string, string>();

        private bool ParseInfoFile(string content)
        {
            try
            {
                XmlDocument document = new XmlDocument();
                document.LoadXml(content);
                XmlElement root = document.DocumentElement;
                XmlElement ele = root.FirstChild as XmlElement;
                _remoteVersion = ele.InnerText;
                while ((ele = ele.NextSibling as XmlElement) != null)
                {
                    string filename = ele.InnerText;
                    ele = ele.NextSibling as XmlElement;
                    string md5 = ele.InnerText;
                    _remoteFiles.Add(filename, md5);
                }
            }
            catch
            {
                return false;
            }
            return true;
        }

        static private List<string> GetDirectoryFileList(string strDirectory)
        {
            string[] filenames = Directory.GetFileSystemEntries(strDirectory);

            List<string> copy = new List<string>();

            foreach (string file in filenames)// 遍历所有的文件和目录
            {
                if (Directory.Exists(file))// 先当作目录处理如果存在这个目录就递归Copy该目录下面的文件
                {
                    copy.AddRange(GetDirectoryFileList(file));
                }
                else
                {
                    copy.Add(file);
                }
            }

            return copy;
        }


        private object _localFilesLocker = new object();
        private Dictionary<string, string> _localFiles = new Dictionary<string, string>();

        private void ScanLocalFile()
        {
            textBox1.Text += "读取本地文件中……" + Environment.NewLine;
            List<string> files = GetDirectoryFileList(_exeDir);
            int num = 0;
            int total = 0;
            object totalLocker = new object();
            lock(totalLocker)
            {
                foreach (string file in files)
                {
                    string ext = Path.GetExtension(file).ToLower();
                    string filename = Path.GetFileName(file);
                    string relativepath = file.Substring(_exeDir.Length).Replace('\\', '/');
                    if (ext == ".ini" || ext == ".bat" || ext == ".zip" || ext == ".rar" || ext == ".config" || ext == ".tmp")
                        continue;
                    if (filename == "files.txt" || filename == "projects.txt")
                        continue;
                    if (filename == "UpdateReleaser.exe")
                        continue;
                    if (filename == INFO_FILE)
                        continue;
                    if (relativepath.StartsWith("Backup/"))
                        continue;
                    total++;
                    BeginTask<object>(() =>
                    {
                        MD5 md5 = new MD5CryptoServiceProvider();
                        try
                        {
                            using (FileStream fs = File.OpenRead(file))
                            {
                                byte[] output = md5.ComputeHash(fs);
                                string result = BitConverter.ToString(output).Replace("-", "");
                                lock (_localFilesLocker)
                                {
                                    _localFiles.Add(relativepath, result);
                                }
                            }
                        }
                        catch
                        {
                            Invoke(() =>
                            {
                                textBox1.Text += "文件：" + file + "打开或读取失败。" + Environment.NewLine;
                            });
                        }
                        lock (totalLocker)
                        {
                            if(++num == total)
                            {
                                Invoke(GenerateWorkList);
                            }
                        }
                        return null;
                    });

                }
            }
        }

        private List<string> _filesToDelete = new List<string>();
        private List<string> _filesToDownload = new List<string>();

        private bool ShouldUpdateSelf = false;

        private void GenerateWorkList()
        {
            foreach(var e in _localFiles)
            {
                if(!_remoteFiles.ContainsKey(e.Key))
                {
                    string ext = Path.GetExtension(e.Key).ToLower();
                    if(ext == ".exe" || ext == ".dll" || ext == ".api" || ext == ".txt")
                        _filesToDelete.Add(e.Key);
                }
            }
            foreach(var e in _remoteFiles)
            {
                if(!_localFiles.ContainsKey(e.Key))
                {
                    _filesToDownload.Add(e.Key);
                }
                else if(_localFiles[e.Key] != e.Value)
                {
                    _filesToDownload.Add(e.Key);
                }
            }
            if(_filesToDownload.Contains("update.exe"))
            {
                if(!ForceUseSelf)
                {
                    ShouldUpdateSelf = true;
                    _client.DownloadProgressChanged += _client_DownloadProgressChanged;
                    _client.DownloadDataCompleted += _client_DownloadDataCompleted;
                    DownloadFile("update.exe");
                    return;
                }
                _filesToDownload.Remove("update.exe");
            }
            if(NeedVCRuntime)
            {
                _filesToDownload.Add("vcredist_x86.exe");
            }
            MakeBackup();
        }

        private void MakeBackup()
        {
            if(_version == "")
            {
                UpdateCore();
                return;
            }
            string backupPath = _exeDir + "Backup/" + _version + "/";
            Directory.CreateDirectory(backupPath);
            foreach (var e in _filesToDelete)
            {
                Directory.CreateDirectory(Path.GetDirectoryName(backupPath + e));
                File.Copy(_exeDir + e, backupPath + e, true);
            }
            foreach (var e in _filesToDownload)
            {
                if (File.Exists(_exeDir + e))
                {
                    Directory.CreateDirectory(Path.GetDirectoryName(backupPath + e));
                    File.Copy(_exeDir + e, backupPath + e, true);
                }
            }
            if (File.Exists(_exeDir + INFO_FILE))
                    File.Copy(_exeDir + INFO_FILE, backupPath + INFO_FILE, true);
            DeleteOldBackup();
        }

        private void DeleteOldBackup()
        {
            string backupPath = _exeDir + "Backup/";
            string[] backups = Directory.GetDirectories(backupPath);
            if(backups.Length > 3)
            {
                string min = backups[0];
                foreach(var str in backups)
                {
                    if(min.CompareTo(str)>0)
                    {
                        min = str;
                    }
                }
                textBox1.Text += "删除过期备份：" + min + Environment.NewLine;
                Directory.Delete(min, true);
            }
            UpdateCore();
        }

        private void DeleteFiles()
        {
            foreach (var e in _filesToDelete)
            {
                textBox1.Text += "尝试删除文件：" + e + "……";
                try
                {
                    File.Delete(_exeDir + e);
                    textBox1.Text += "成功" + Environment.NewLine;
                }
                catch (Exception ex)
                {
                    textBox1.Text += "失败，" + ex.Message + Environment.NewLine;
                }
            }
        }

        private int _index = -1;

        private void UpdateCore()
        {
            _client.DownloadProgressChanged += _client_DownloadProgressChanged;
            _client.DownloadDataCompleted += _client_DownloadDataCompleted;
            if(_filesToDownload.Count == 0)
            {
                textBox1.Text += "没有文件可以下载。" + Environment.NewLine;
                WriteVersionFile();
                CloseAfter3Seconds();
                return;
            }
            System.Windows.Forms.Timer timer1 = new System.Windows.Forms.Timer();
            timer1.Interval = 1000;
            timer1.Tick += CalculateSpeed;
            timer1.Enabled = true;
            progressBar2.Value = 0;
            progressBar2.Maximum = _filesToDownload.Count;
            NextFile();
        }

        private long downloadedBytes = 0;
        private long lastBytes = 0;

        private void CalculateSpeed(object sender, EventArgs e)
        {
            long mb = downloadedBytes / 1024 / 1024;
            long kb = downloadedBytes % 1024;
            string tmp;
            if(mb == 0)
            {
                tmp = kb.ToString() + " KB/S";
            }
            else
            {
                double mb2 = mb;
                double kb2 = kb;
                kb2 /= 1024;
                mb2 += kb2;
                tmp = mb2.ToString("f2") + " MB/S";
            }
            label1.Text = tmp;
            downloadedBytes = 0;
        }

        private void _client_DownloadDataCompleted(object sender, DownloadDataCompletedEventArgs e)
        {
            if (e.Cancelled)
                return;
            lastBytes = 0;
            if(e.Error != null)
            {
                textBox1.Text += "错误：" + Environment.NewLine + e.Error.Message + Environment.NewLine;
                textBox1.Text += "3秒后重试……" + Environment.NewLine;
                System.Windows.Forms.Timer t = new System.Windows.Forms.Timer();
                t.Interval = 3000;
                t.Tick += (_, __) =>
                {
                    t.Enabled = false;
                    t.Dispose();
                    if (ShouldUpdateSelf)
                    {
                        DownloadFile("update.exe");
                        return;
                    }
                    DownloadThisFile();
                };
                t.Enabled = true;
                return;
            }
            textBox1.Text += "成功。" + Environment.NewLine;
            byte[] data = e.Result;
            if (ShouldUpdateSelf)
            {
                DecompressToFile("update.exe", data);
                UpdateSelf();
                return;
            }
            DecompressThisToFile(data);
            progressBar2.Value = _index + 1;
            label2.Text = progressBar2.Value.ToString() + " / " + progressBar2.Maximum.ToString();
            NextFile();
        }

        private void UpdateSelf()
        {
            foreach (var s in _tasks)
            {
                s();
            }
            Process p = new Process();
            p.StartInfo.FileName = @"cmd.exe";
            p.StartInfo.UseShellExecute = false;
            p.StartInfo.WorkingDirectory = _exeDir;
            string arg = "/C ping /n 2 127.1>nul&del /f /a /q update.exe&ping /n 1 127.1>nul&mv update.exe.tmp update.exe&update.exe";
            if(FixUp)
            {
                arg += " -fixup";
            }
            arg += "&exit";
            p.StartInfo.Arguments = arg;
            p.StartInfo.CreateNoWindow = true;
            p.Start();
            Close();
        }

        private void _client_DownloadProgressChanged(object sender, DownloadProgressChangedEventArgs e)
        {
            downloadedBytes += e.BytesReceived - lastBytes;
            lastBytes = e.BytesReceived;
            progressBar1.Value = e.ProgressPercentage;
        }

        private void DownloadFile(string file)
        {
            progressBar1.Value = 0;
            string tmpFile = _exeDir + file +".tmp";
            textBox1.Text += "正在下载" + file + "……";
            if (File.Exists(tmpFile))
            {
                FileInfo fi = new FileInfo(tmpFile);
                long localFileLength = fi.Length;
                try
                {
                    HttpWebRequest request = WebRequest.Create(UriForFile(file)) as HttpWebRequest;
                    long remoteFileLength = request.GetResponse().ContentLength;
                    if (localFileLength == remoteFileLength)
                    {
                        NextFile();
                        return;
                    }
                    else
                    {
                        File.Delete(tmpFile);
                    }
                }
                catch (Exception)
                {
                    textBox1.Text += "获取服务器文件大小失败：" + file + Environment.NewLine;
                    CloseAfter3Seconds();
                    return;
                }
            }
            _client.DownloadDataAsync(UriForFile(file));
        }

        private void DownloadThisFile()
        {
            string file = _filesToDownload[_index];
            DownloadFile(file);
        }

        public delegate R AsyncTask<R>();

        public static AsyncTask<R> BeginTask<R>(AsyncTask<R> function)
        {
            R retv = default(R);
            bool completed = false;

            object sync = new object();

            IAsyncResult asyncResult = function.BeginInvoke(
                    iAsyncResult =>
                    {
                        lock (sync)
                        {
                            completed = true;
                            retv = function.EndInvoke(iAsyncResult);
                            Monitor.Pulse(sync);
                        }
                    }, null);

            return delegate
            {
                lock (sync)
                {
                    if (!completed)
                    {
                        Monitor.Wait(sync);
                    }
                    return retv;
                }
            };
        }

        private List<AsyncTask<string>> _tasks =  new List<AsyncTask<string>>();

        private void DecompressToFile(string file, byte[] data)
        {
            string tmpFile = Path.Combine(_exeDir, file + ".tmp");
            Directory.CreateDirectory(Path.GetDirectoryName(tmpFile));
            var s = BeginTask<string>(() =>
            {
                using (MemoryStream ms = new MemoryStream(data))
                {
                    byte[] buf = new byte[1024 * 1024];
                    using (GZipStream gz = new GZipStream(ms, CompressionMode.Decompress, true))
                    {
                        using (FileStream fs = File.Create(tmpFile))
                        {
                            while (true)
                            {
                                int len = gz.Read(buf, 0, buf.Length);
                                if (len == 0)
                                {
                                    break;
                                }
                                fs.Write(buf, 0, len);
                            }
                            fs.Close();
                        }
                    }
                }
                return null;
            });
            _tasks.Add(s);
        }

        private void DecompressThisToFile(byte[] data)
        {
            DecompressToFile(_filesToDownload[_index], data);
        }

        private void Form1_FormClosing1(object sender, FormClosingEventArgs e)
        {
            foreach(var s in _tasks)
            {
                s();
            }
        }

        private void NextFile()
        {
            _index++;
            if (_index >= _filesToDownload.Count)
            {
                ApplyTemps();
                return;
            }
            DownloadThisFile();
        }

        private void ApplyTemps()
        {
            foreach (var s in _tasks)
            {
                s();
            }
            textBox1.Text += "正在应用更新项……" + Environment.NewLine;
            DeleteFiles();
            foreach (var file in _filesToDownload)
            {
                try
                {
                    var dstFile = Path.Combine(_exeDir, file);
                    var tmpFile = dstFile + ".tmp";
                    File.Delete(dstFile);
                    File.Move(tmpFile, dstFile);
                }
                catch(Exception e)
                {
                    textBox1.Text += "文件" + file + "应用失败：" + e.Message + "请手动应用它们。" + Environment.NewLine;
                }
            }
            if (NeedVCRuntime)
                SetupVCRuntime();
            else
                Finish();
        }

        private void SetupVCRuntime()
        {
            textBox1.Text += "开始安装Visual C++ Redistributable Package 2015……";
            Process process = new Process();
            process.StartInfo.UseShellExecute = false;
            process.StartInfo.FileName = Path.Combine(_exeDir, "vc_redist.x86.exe");
            process.StartInfo.Arguments = "/install /passive /norestart";
            process.EnableRaisingEvents = true;
            process.Exited += (_, e) =>
            {
                this.Invoke(new MethodInvoker(() =>
                {
                    textBox1.Text += "成功。" + Environment.NewLine;
                    File.Delete(Path.Combine(_exeDir, "vc_redist.x86.exe"));
                    Finish();
                }));
            };
            process.Start();
        }

        private void Finish()
        {
            WriteVersionFile();
            textBox1.Text += "更新完成，将启动BKE_Creator。" + Environment.NewLine;
            Process.Start(Path.Combine(_exeDir, "BKE_Creator.exe"));
            CloseAfter3Seconds();
        }

        private void WriteVersionFile()
        {
            try
            {
                File.WriteAllText(Path.Combine(_exeDir, "version"), _remoteVersion, Encoding.UTF8);
            }
            catch (Exception)
            {

            }
        }

        private void textBox1_TextChanged(object sender, EventArgs e)
        {
            textBox1.Select(textBox1.Text.Length, 0);
            textBox1.ScrollToCaret();
        }
    }
}
