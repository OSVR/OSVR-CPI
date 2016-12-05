// Copyright 2016 Razer, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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