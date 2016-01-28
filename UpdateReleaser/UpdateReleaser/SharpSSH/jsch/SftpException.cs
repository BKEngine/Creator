using System;

namespace Tamir.SharpSsh.jsch
{
	public class SftpException : java.Exception
	{
		public int id;
		public String message;
		public SftpException (int id, String message):base(message) 
		{
			this.id=id;
			this.message=message;
		}
		public override String toString()
		{
			return message;
		}
	}
}
