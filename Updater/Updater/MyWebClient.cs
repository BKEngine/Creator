using System;
using System.Net;

namespace Updater
{
    internal class MyWebClient : WebClient
    {
        internal MyWebClient()
        {
            this.Proxy = null;
            ServicePointManager.DefaultConnectionLimit = 500;
        }

        protected override WebRequest GetWebRequest(Uri address)
        {
            HttpWebRequest request = (HttpWebRequest)base.GetWebRequest(address);
            request.Timeout = 3000;
            request.ReadWriteTimeout = 3000;
            return request;
        }
    }
}
