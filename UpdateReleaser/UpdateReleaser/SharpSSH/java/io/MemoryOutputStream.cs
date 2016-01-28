using System;
using IO = System.IO;

namespace Tamir.SharpSsh.java.io
{
    class MemoryOutputStream : OutputStream
    {
        private IO.MemoryStream ms;
        public MemoryOutputStream(IO.MemoryStream ms)
        {
            this.ms = ms;
        }
        public MemoryOutputStream(byte[] buf)
        {
            this.ms = new IO.MemoryStream(buf);
        }
        public override void Write(byte[] buffer, int offset, int count)
        {
            ms.Write(buffer, offset, count);
        }

        public override void Flush()
        {
            ms.Flush();
        }

        public override void Close()
        {
            ms.Close();
        }

        public override bool CanSeek
        {
            get
            {
                return ms.CanSeek;
            }
        }

        public override long Seek(long offset, IO.SeekOrigin origin)
        {
            return ms.Seek(offset, origin);
        }
    }
}
