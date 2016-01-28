using System;
using IO = System.IO;

namespace Tamir.SharpSsh.java.io
{
    class MemoryInputStream : InputStream
    {
        private IO.MemoryStream ms;
        public MemoryInputStream(IO.MemoryStream ms)
        {
            this.ms = ms;
        }
        public MemoryInputStream(byte[] buf)
        {
            this.ms = new IO.MemoryStream(buf);
        }

        public override long Length
        {
            get
            {
                return ms.Length;
            }
        }

        public override int Read(byte[] buffer, int offset, int count)
        {
            return this.ms.Read(buffer, offset, count);
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
