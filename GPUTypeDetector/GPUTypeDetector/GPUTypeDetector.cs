/*
 * Standalone GPU type detector application.  Returns exit code to indicate detected GPU type.
 * 
 * In a hybrid configuration (e.g. Optimus architecture with discrete NVIDIA GPU and embedded Intel GPU), the priority order is NVIDIA, AMD, and finally Unknown.
 * 
 * This is used by the CPI to automatically detect your GPU type so the appropriate supplemental applications can be invoked.
 * 
 * See https://github.com/Razer-OSVR/OSVR-TrayApp/ for corresponding source in the TrayApp application.
 */

using System;
using System.Management;

namespace GPUTypeDetector
{
    public class GPUTypeDetector
    {
        public enum GraphicsCardType { UNKNOWN = -2, ERROR = -1, NVIDIA = 1, AMD = 2 };

        static void Main(string[] args)
        {
            try
            {
                GraphicsCardType gpu = Detect();
                Environment.Exit((int)gpu);
            }
            catch
            {
                Environment.Exit((int)GraphicsCardType.ERROR);
            }
        }

        public static GraphicsCardType Detect()
        {
            ManagementObjectSearcher searcher = new ManagementObjectSearcher("SELECT * FROM Win32_VideoController");

            foreach (ManagementObject mo in searcher.Get())
            {
                foreach (PropertyData property in mo.Properties)
                {
                    if (property.Name == "AdapterCompatibility")
                    {
                        switch (property.Value.ToString())
                        {
                            case "NVIDIA":
                                return GraphicsCardType.NVIDIA;

                            case "Advanced Micro Devices, Inc.":
                                return GraphicsCardType.AMD;

                            case "Intel Corporation":
                            default:
                                break;
                        }
                    }
                }
            }

            return GraphicsCardType.UNKNOWN;
        }
    }
}