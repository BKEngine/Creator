using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Net;
using System.Windows.Forms;
using System.Xml;
using System.Configuration;
using System.Security.Cryptography;
using System.IO.Compression;
using Tamir.SharpSsh.jsch;
using System.Threading;
using System.Text;
using System.Diagnostics;

namespace UpdateReleaser
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
        }

        private static string HTTP_ADDRESS = "http://creator.up.bakery.moe/windows/";
        private static string INFO_FILE = "version";
        private WebClient _client = new WebClient();
        private string _exeDir = AppDomain.CurrentDomain.BaseDirectory;
        private string PrivateKeyPath
        {
            get { return textBox3.Text; }
            set { textBox3.Text = value; }
        }
        private string ThemidaPath
        {
            get { return textBox4.Text; }
            set { textBox4.Text = value; }
        }
        private string Version
        {
            get { return textBox1.Text; }
            set { textBox1.Text = value; }
        }

        private string RemoteVersion { get; set; }

        private void Form1_Load(object sender, EventArgs e)
        {
            if (!_exeDir.EndsWith("\\") && _exeDir.EndsWith("/"))
                _exeDir += "\\";
            PrivateKeyPath = AppSettings.GetValue("PrivateKeyPath");
            ThemidaPath = AppSettings.GetValue("ThemidaPath");
            ReadRemoteVersionFile();
        }
        private Uri UriForFile(string path)
        {
            Uri uri = new Uri(HTTP_ADDRESS + path.Replace('\\', '/'));
            return uri;
        }

        public void ReadRemoteVersionFile()
        {
            button1.Enabled = false;
            textBox2.Text = "获取服务器文件信息……";
            _client.DownloadStringCompleted += _client_DownloadStringCompleted;
            _client.DownloadStringAsync(UriForFile(INFO_FILE));
        }

        private void _client_DownloadStringCompleted(object sender, DownloadStringCompletedEventArgs e)
        {
            _client.DownloadStringCompleted -= _client_DownloadStringCompleted;
            if (e.Cancelled)
                return;
            if (e.Error != null)
            {
                textBox2.Text += "失败。" + Environment.NewLine + e.Error.Message;
                return;
            }
            ParseInfoFile(e.Result);
            textBox2.Text += "成功。" + Environment.NewLine;
            textBox2.Text += "服务器版本：" + RemoteVersion + Environment.NewLine;
        }

        private Dictionary<string, string> _remoteFiles = new Dictionary<string, string>();

        private void ParseInfoFile(string content)
        {
            XmlDocument document = new XmlDocument();
            document.LoadXml(content);
            XmlElement root = document.DocumentElement;
            XmlElement ele = root.FirstChild as XmlElement;
            RemoteVersion = Version = ele.InnerText;
            while ((ele = ele.NextSibling as XmlElement) != null)
            {
                string filename = ele.InnerText;
                ele = ele.NextSibling as XmlElement;
                string md5 = ele.InnerText;
                _remoteFiles.Add(filename, md5);
            }
            button1.Enabled = true;
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

        private SFTPHelper helper;

        private void button1_Click(object sender, EventArgs e)
        {
            if(textBox1.Text == "")
            {
                MessageBox.Show("版本不能为空！", "警告");
            }
            else if(Version.CompareTo(RemoteVersion)<=0)
            {
                MessageBox.Show("版本不比服务器版本旧！", "警告");
            }
            else
            {
                button1.Enabled = false;
                AddPacker(ScanLocalFile);
            }
        }


        private void AddPacker(Action action)
        {
            if (string.IsNullOrEmpty(ThemidaPath))
            {
                action();
            }
            else
            {
                textBox2.Text += "正在加壳……" + Environment.NewLine;
                string path = Path.Combine(_exeDir, "tool");
                string tmdbkefile = Path.Combine(ThemidaPath, "bke.tmd");
                List<string> bkefiles = GetDirectoryFileList(path);
                int size = 0;
                int finish = 0;
                foreach (string file in bkefiles)
                {
                    string ext = Path.GetExtension(file).ToLower();
                    string filename = Path.GetFileName(file);
                    if (filename.StartsWith("BK", StringComparison.CurrentCultureIgnoreCase))
                    {
                        size++;
                        Process p = new Process();
                        p.StartInfo.FileName = Path.Combine(ThemidaPath, "Themida.exe");
                        p.StartInfo.UseShellExecute = false;
                        p.StartInfo.WorkingDirectory = path;
                        p.StartInfo.Arguments = "/protect \"" + tmdbkefile + "\" /inputfile \"" + file + "\"";
                        p.StartInfo.CreateNoWindow = true;
                        p.EnableRaisingEvents = true;
                        p.Exited += (_, e) =>
                        {
                            Invoke(new MethodInvoker(() =>
                            {
                                if(p.ExitCode != 0)
                                {
                                    if (p.ExitCode == 3)
                                        textBox2.Text += "文件：" + filename + "已经加壳。" + Environment.NewLine;
                                    else
                                        textBox2.Text += "文件：" + filename + "加壳失败。" + Environment.NewLine;
                                }
                                else
                                {
                                    textBox2.Text += "文件：" + filename + "加壳成功。" + Environment.NewLine;
                                    File.Delete(Path.Combine(Path.GetDirectoryName(file), Path.GetFileNameWithoutExtension(file) + ".bak"));
                                    File.Delete(file + ".log");
                                }
                                finish++;
                                if(finish == size)
                                {
                                    action();
                                }
                            }));
                        };
                        p.Start();
                    }
                }
            }
        }

        private Dictionary<string, string> _localFiles = new Dictionary<string, string>();

        private void ScanLocalFile()
        {
            List<string> files = GetDirectoryFileList(_exeDir);
            MD5 md5 = new MD5CryptoServiceProvider();
            foreach (string file in files)
            {
                string ext = Path.GetExtension(file).ToLower();
                string filename = Path.GetFileName(file);
                string relativepath = file.Substring(_exeDir.Length).Replace('\\', '/');
                if (ext == ".ini" || ext == ".tmd" || ext == ".bat" || ext == ".zip" || ext == ".rar" || ext == ".config" || ext == ".tmp")
                    continue;
                if (filename == "files.txt" || filename == "projects.txt")
                    continue;
                if (filename == "UpdateReleaser.exe")
                    continue;
                if (filename == INFO_FILE)
                    continue;
                if (relativepath.StartsWith("Backup/"))
                    continue;
                try
                {
                    using (FileStream fs = File.OpenRead(file))
                    {
                        byte[] output = md5.ComputeHash(fs);
                        string result = BitConverter.ToString(output).Replace("-", "");
                        _localFiles.Add(relativepath, result);
                    }
                }
                catch
                {
                    textBox2.Text += "文件：" + file + "打开或读取失败。" + Environment.NewLine;
                    continue;
                }
            }
            GenerateWorkList();
        }

        private List<string> _filesToDelete = new List<string>();
        private List<string> _filesToUpload = new List<string>();

        private void GenerateWorkList()
        {
            foreach (var e in _remoteFiles)
            {
                if (!_localFiles.ContainsKey(e.Key))
                {
                    _filesToDelete.Add(e.Key);
                }
            }
            foreach (var e in _localFiles)
            {
                if (!_remoteFiles.ContainsKey(e.Key))
                {
                    _filesToUpload.Add(e.Key);
                }
                else if (_remoteFiles[e.Key] != e.Value)
                {
                    _filesToUpload.Add(e.Key);
                }
            }
            ReleaseCore();
        }

        private Dictionary<string, byte[]> _list = new Dictionary<string, byte[]>();

        private void GenerateGzipFile()
        {
            foreach(var str in _filesToUpload)
            {
                try
                {

                    MemoryStream ms = new MemoryStream();
                    using (FileStream fs = File.OpenRead(_exeDir + str))
                    {
                        GZipStream gz = new GZipStream(ms, CompressionMode.Compress);
                        byte[] bytes = new byte[fs.Length];
                        fs.Read(bytes, 0, (int)fs.Length);
                        gz.Write(bytes, 0, (int)fs.Length);
                        gz.Close();
                    }
                    _list.Add(str, ms.ToArray());
                }
                catch (Exception)
                {
                    textBox2.Text += "文件" + str + "打开或读取失败。" + Environment.NewLine;
                }
            }
        }

        private void ReleaseCore()
        {
            GenerateGzipFile();
            if (checkBox1.Checked)
                UploadToServer();
            else
                SaveToLocal();
        }

        private void SaveToLocal()
        {
            var outputDir = _exeDir + "../UpdateData/";

            foreach(var file in _filesToDelete)
            {
                File.Delete(outputDir + file);
            }

            foreach(var file in _list)
            {
                Directory.CreateDirectory(Path.GetDirectoryName(outputDir + file.Key));
                using (FileStream fs = File.Create(outputDir + file.Key))
                {
                    byte[] a = file.Value;
                    fs.Write(a, 0, a.Length);
                }
            }
            _list.Clear();
            GC.Collect();
        }

        private Thread _uploadThread;

        private void UploadToServer()
        {
            _uploadThread = new Thread(new ThreadStart(ConnectServer));
            _uploadThread.IsBackground = true;
            _uploadThread.Start();
            System.Windows.Forms.Timer timer1 = new System.Windows.Forms.Timer();
            timer1.Interval = 1000;
            timer1.Tick += CalculateSpeed;
            timer1.Enabled = true;
        }

        private void ConnectServer()
        {
            SshConnectionInfo info = new SshConnectionInfo
            {
                User = "update",
                Pass = null,
                Host = "up.bakery.moe",
                IdentityFile = PrivateKeyPath,
            };
            helper = new SFTPHelper(info, OnStart, OnProgress, null);
            helper.Connect();
            ChannelSftp c = helper.Sftp.SftpChannel;
            c.cd("www/creator/windows");

            byte[] b = EncodeInfoFile();
            foreach(var file in _list)
            {
                helper.Upload(new MemoryStream(file.Value), file.Key);
                helper.Sftp.Cancel();
            }
            helper.Upload(new MemoryStream(b), INFO_FILE);
            helper.Close();
            helper = null;

            Invoke(new MethodInvoker(() =>
            {
                textBox2.Text += "上传完成。" + Environment.NewLine;
                WriteVersionFile();
                Clear();
                button1.Enabled = true;
            }));
        }

        private byte[] EncodeInfoFile()
        {
            XmlDocument document = new XmlDocument();
            XmlDeclaration dec = document.CreateXmlDeclaration("1.0", "UTF-8", null);
            document.AppendChild(dec);
            XmlElement root = document.CreateElement("info");
            XmlElement version = document.CreateElement("version");
            version.InnerText = Version;
            root.AppendChild(version);
            foreach(var file in _localFiles)
            {
                XmlElement name = document.CreateElement("name");
                name.InnerText = file.Key;
                XmlElement md5 = document.CreateElement("md5");
                md5.InnerText = file.Value;
                root.AppendChild(name);
                root.AppendChild(md5);
            }
            document.AppendChild(root);
            return System.Text.Encoding.UTF8.GetBytes(document.OuterXml);
        }

        private long downloadedBytes = 0;
        private long lastBytes = 0;

        private string GetSizeString(long size)
        {
            long mb = size / 1024 / 1024;
            long kb = size / 1024 % 1024;
            string tmp;
            if (mb == 0)
            {
                tmp = kb.ToString() + " KB";
            }
            else
            {
                double mb2 = mb;
                double kb2 = kb;
                kb2 /= 1024;
                mb2 += kb2;
                tmp = mb2.ToString("f2") + " MB";
            }
            return tmp;
        }

        private void CalculateSpeed(object sender, EventArgs e)
        {
            label3.Text = GetSizeString(downloadedBytes) + "/S";
            downloadedBytes = 0;
        }

        private void OnStart(string src, string dst, int transferredBytes, int totalBytes, string message)
        {
            textBox2.Invoke(new MethodInvoker(() => 
            {
                textBox2.Text += "正在上传" + Path.GetFileName(dst) + "，大小" + GetSizeString(totalBytes) + "中……" + Environment.NewLine;
                lastBytes = 0;
            }));
        }

        private void OnProgress(string src, string dst, int transferredBytes, int totalBytes, string message)
        {
            progressBar1.Invoke(new MethodInvoker(() =>
            {
                downloadedBytes += transferredBytes - lastBytes;
                lastBytes = transferredBytes;
                long b = transferredBytes;
                progressBar1.Value = (int)(b * 100 / totalBytes);
            }));
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            AppSettings.SetValue("PrivateKeyPath", PrivateKeyPath);
            AppSettings.SetValue("ThemidaPath", ThemidaPath);
            if(helper != null)
            {
                helper.Cancel();
                helper.Close();
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            OpenFileDialog dialog = new OpenFileDialog();
            dialog.Filter = "所有文件|*";
            if (dialog.ShowDialog() == DialogResult.OK)
            {
                PrivateKeyPath = dialog.FileName;
            }
        }
        
        private void button3_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog dialog = new FolderBrowserDialog();
            if (dialog.ShowDialog() == DialogResult.OK)
            {
                ThemidaPath = dialog.SelectedPath;
            }
        }

        private void WriteVersionFile()
        {
            try
            {
                File.WriteAllText(_exeDir + "version", Version, Encoding.UTF8);
            }
            catch (Exception)
            {

            }
        }

        private void Clear()
        {
            _client = new WebClient();
            _filesToDelete.Clear();
            _filesToUpload.Clear();
            _list.Clear();
            _localFiles.Clear();
            _remoteFiles.Clear();
            _uploadThread = null;
        }

        private void textBox2_TextChanged(object sender, EventArgs e)
        {
            textBox2.Select(textBox2.Text.Length, 0);
            textBox2.ScrollToCaret();
        }

    }
}
