using System;
using System.Collections.Generic;
using System.Text;
using System.Configuration;

namespace UpdateReleaser
{
    /// <summary>
    /// 对exe.Config文件中的appSettings段进行读写配置操作
    /// 注意：调试时，写操作将写在vhost.exe.config文件中
    /// </summary>
    public class AppSettings
    {
        /// <summary>
        /// 写入值
        /// </summary>
        /// <param name="key"></param>
        /// <param name="value"></param>
        public static void SetValue(string key, string value)
        {
            //增加的内容写在appSettings段下 <add key="RegCode" value="0"/>
            System.Configuration.Configuration config = ConfigurationManager.OpenExeConfiguration(ConfigurationUserLevel.None);
            if (config.AppSettings.Settings[key] == null)
            {
                config.AppSettings.Settings.Add(key, value);
            }
            else
            {
                config.AppSettings.Settings[key].Value = value;
            }
            config.Save(ConfigurationSaveMode.Modified);
            //ConfigurationManager.RefreshSection("appSettings");//重新加载新的配置文件 
        }

        /// <summary>
        /// 读取指定key的值
        /// </summary>
        /// <param name="key"></param>
        /// <returns></returns>
        public static string GetValue(string key)
        {
            System.Configuration.Configuration config = ConfigurationManager.OpenExeConfiguration(ConfigurationUserLevel.None);
            if (config.AppSettings.Settings[key] == null)
                return "";
            else
                return config.AppSettings.Settings[key].Value;
        }

    }
}
