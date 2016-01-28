using System.Collections;
using System.IO;
using Tamir.SharpSsh;
using Tamir.SharpSsh.java.io;

namespace UpdateReleaser
{

    public class SshConnectionInfo
    {
        public string IdentityFilePassword { get; set; }
        public string IdentityFile { get; set; }
        public string Pass { get; set; }
        public string Host { get; set; }
        public string User { get; set; }
    }

    public class SFTPHelper
    {
        private Sftp m_sshCp;
        private SFTPHelper()
        {

        }
        public SFTPHelper(SshConnectionInfo connectionInfo)
        {
            m_sshCp = new Sftp(connectionInfo.Host, connectionInfo.User);

            if (connectionInfo.Pass != null)
            {
                m_sshCp.Password = connectionInfo.Pass;
            }

            if (connectionInfo.IdentityFile != null)
            {
                m_sshCp.AddIdentityFile(connectionInfo.IdentityFile, connectionInfo.IdentityFilePassword);
            }
        }

        public SFTPHelper(SshConnectionInfo connectionInfo, FileTransferEvent onStart, FileTransferEvent onProgress, FileTransferEvent onEnd)
            : this(connectionInfo)
        {
            m_sshCp.OnTransferStart += onStart;
            m_sshCp.OnTransferProgress += onProgress;
            m_sshCp.OnTransferEnd += onEnd;
        }

        public bool Connected
        {
            get
            {
                return m_sshCp.Connected;
            }
        }
        public void Connect()
        {
            if (!m_sshCp.Connected)
            {
                m_sshCp.Connect();
            }
        }
        public void Close()
        {
            if (m_sshCp.Connected)
            {
                m_sshCp.Close();
            }
        }
        public bool Upload(string localPath, string remotePath)
        {
            try
            {
                if (!m_sshCp.Connected)
                {
                    m_sshCp.Connect();
                }
                m_sshCp.Put(localPath, remotePath);

                return true;
            }
            catch
            {
                return false;
            }
        }

        public void Upload(MemoryStream stream, string remotePath)
        {
            if (!m_sshCp.Connected)
            {
                m_sshCp.Connect();
            }
            m_sshCp.Put(new MemoryInputStream(stream), remotePath);
        }

        public bool Download(string remotePath, string localPath)
        {
            try
            {
                if (!m_sshCp.Connected)
                {
                    m_sshCp.Connect();
                }

                m_sshCp.Get(remotePath, localPath);

                return true;
            }
            catch
            {
                return false;
            }
        }
        public bool Delete(string remotePath)
        {
            try
            {
                if (!m_sshCp.Connected)
                {
                    m_sshCp.Connect();
                }
                m_sshCp.Delete(remotePath);//刚刚新增的Delete方法

                return true;
            }
            catch
            {
                return false;
            }
        }

        public ArrayList GetFileList(string path)
        {
            try
            {
                if (!m_sshCp.Connected)
                {
                    m_sshCp.Connect();
                }
                return ((Sftp)m_sshCp).GetFileList(path);
            }
            catch
            {
                return null;
            }
        }

        public void Cancel()
        {
            m_sshCp.Cancel();
        }

        public Sftp Sftp { get { return m_sshCp; } }
    }
}