/* Editor Settings: expandtabs and use 4 spaces for indentation
 * ex: set softtabstop=4 tabstop=8 expandtab shiftwidth=4: *
 * -*- mode: c, c-basic-offset: 4 -*- */

/*
 * Copyright Likewise Software    2004-2008
 * All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the license, or (at
 * your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser
 * General Public License for more details.  You should have received a copy
 * of the GNU Lesser General Public License along with this program.  If
 * not, see <http://www.gnu.org/licenses/>.
 *
 * LIKEWISE SOFTWARE MAKES THIS SOFTWARE AVAILABLE UNDER OTHER LICENSING
 * TERMS AS WELL.  IF YOU HAVE ENTERED INTO A SEPARATE LICENSE AGREEMENT
 * WITH LIKEWISE SOFTWARE, THEN YOU MAY ELECT TO USE THE SOFTWARE UNDER THE
 * TERMS OF THAT SOFTWARE LICENSE AGREEMENT INSTEAD OF THE TERMS OF THE GNU
 * LESSER GENERAL PUBLIC LICENSE, NOTWITHSTANDING THE ABOVE NOTICE.  IF YOU
 * HAVE QUESTIONS, OR WISH TO REQUEST A COPY OF THE ALTERNATE LICENSING
 * TERMS OFFERED BY LIKEWISE SOFTWARE, PLEASE CONTACT LIKEWISE SOFTWARE AT
 * license@likewisesoftware.com
 */

using System;
using System.Collections.Generic;
using System.Text;
using Likewise.LMC.Utilities;
using System.Runtime.InteropServices;

namespace Likewise.LMC.Services
{
    public class ServiceManagerInteropWrapper
    {
        public static string[] ApiLwSmEnumerateServices()
        {
            string[] ServicesEnum = null;
            try
            {
                Logger.Log("ServiceManagerInteropWrapper:ApiLwSmEnumerateServices()", Logger.ServiceManagerLoglevel);

                int iRet = ServiceManagerInterop.LwSmEnumerateServices(out ServicesEnum);
                if (iRet != 0)
                {
                    Logger.Log("ServiceManagerInteropWrapper:ApiLwSmEnumerateServices returns" + iRet.ToString(), Logger.ServiceManagerLoglevel);
                    return null;
                }
                //if (ppArry != null && ppArry.Length != 0)
                //{
                //    int idx = 0;
                //    ServicesEnum = new string[ppArry.Length];
                //    foreach (IntPtr pService in ppArry)
                //    {
                //        Console.WriteLine(Marshal.PtrToStringAnsi(pService));
                //        ServicesEnum[idx] = Marshal.PtrToStringAnsi(pService);
                //    }
                //}
                Logger.Log("ServiceManagerInteropWrapper:ApiLwSmEnumerateServices is success", Logger.ServiceManagerLoglevel);
            }
            catch (Exception ex)
            {
                Logger.LogException("ServiceManagerInteropWrapper:ApiLwSmEnumerateServices", ex);
            }

            return ServicesEnum;
        }

        public static IntPtr ApiLwSmAcquireServiceHandle(string sServiceName)
        {
            IntPtr phServiceHandle = IntPtr.Zero;
            try
            {
                Logger.Log(string.Format("ServiceManagerInteropWrapper:ApiLwSmAcquireServiceHandle(Servicename = {0})", sServiceName), Logger.ServiceManagerLoglevel);

                int iRet = ServiceManagerInterop.LwSmAcquireServiceHandle(sServiceName, out phServiceHandle);
                if (iRet != 0)
                {
                    Logger.Log("ServiceManagerInteropWrapper:ApiLwSmAcquireServiceHandle returns" + iRet.ToString(), Logger.ServiceManagerLoglevel);
                    return IntPtr.Zero;
                }
                Logger.Log("ServiceManagerInteropWrapper:ApiLwSmAcquireServiceHandle is success", Logger.ServiceManagerLoglevel);
            }
            catch (Exception ex)
            {
                Logger.LogException("ServiceManagerInteropWrapper:ApiLwSmAcquireServiceHandle", ex);
            }
            return phServiceHandle;
        }

        public static int ApiLwSmReleaseServiceHandle(IntPtr phServiceHandle)
        {
            int iRet = -1;
            try
            {
                Logger.Log("ServiceManagerInteropWrapper:ApiLwSmSrvReleaseHandle()", Logger.ServiceManagerLoglevel);

                iRet = ServiceManagerInterop.LwSmReleaseServiceHandle(phServiceHandle);
                if (iRet != 0)
                {
                    Logger.Log("ServiceManagerInteropWrapper:ApiLwSmSrvReleaseHandle returns" + iRet.ToString(), Logger.ServiceManagerLoglevel);
                    return iRet;
                }
                Logger.Log("ServiceManagerInteropWrapper:ApiLwSmReleaseServiceHandle is success", Logger.ServiceManagerLoglevel);
            }
            catch (Exception ex)
            {
                Logger.LogException("ServiceManagerInteropWrapper:ApiLwSmReleaseServiceHandle", ex);
            }
            return iRet;
        }

        public static int ApiLwSmStartService(IntPtr phServiceHandle)
        {
            int iRet = -1;
            try
            {
                Logger.Log("ServiceManagerInteropWrapper:LwSmStartService()", Logger.ServiceManagerLoglevel);

                iRet = ServiceManagerInterop.LwSmStartService(phServiceHandle);
                if (iRet != 0)
                {
                    Logger.Log("ServiceManagerInteropWrapper:LwSmStartService returns" + iRet.ToString(), Logger.ServiceManagerLoglevel);
                    return iRet;
                }
                Logger.Log("ServiceManagerInteropWrapper:LwSmStartService is success", Logger.ServiceManagerLoglevel);
            }
            catch (Exception ex)
            {
                Logger.LogException("ServiceManagerInteropWrapper:ApiLwSmStartService", ex);
            }
            return iRet;
        }

        public static int ApiLwSmStopService(IntPtr phServiceHandle)
        {
            int iRet = -1;
            try
            {
                Logger.Log("ServiceManagerInteropWrapper:ApiLwSmStopService()", Logger.ServiceManagerLoglevel);

                iRet = ServiceManagerInterop.LwSmStopService(phServiceHandle);
                if (iRet != 0)
                {
                    Logger.Log("ServiceManagerInteropWrapper:ApiLwSmStopService returns" + iRet.ToString(), Logger.ServiceManagerLoglevel);
                    return iRet;
                }
                Logger.Log("ServiceManagerInteropWrapper:ApiLwSmStopService is success", Logger.ServiceManagerLoglevel);
            }
            catch (Exception ex)
            {
                Logger.LogException("ServiceManagerInteropWrapper:ApiLwSmStopService", ex);
            }
            return iRet;
        }

        public static int ApiLwSmRefreshService(IntPtr phServiceHandle)
        {
            int iRet = -1;
            try
            {
                Logger.Log("ServiceManagerInteropWrapper:ApiLwSmRefreshService()", Logger.ServiceManagerLoglevel);

                iRet = ServiceManagerInterop.LwSmRefreshService(phServiceHandle);
                if (iRet != 0)
                {
                    Logger.Log("ServiceManagerInteropWrapper:ApiLwSmRefreshService returns" + iRet.ToString(), Logger.ServiceManagerLoglevel);
                    return iRet;
                }
                Logger.Log("ServiceManagerInteropWrapper:ApiLwSmRefreshService is success", Logger.ServiceManagerLoglevel);
            }
            catch (Exception ex)
            {
                Logger.LogException("ServiceManagerInteropWrapper:ApiLwSmRefreshService", ex);
            }
            return iRet;
        }

        public static ServiceManagerApi.LW_SERVICE_STATUS ApiLwSmQueryServiceStatus(IntPtr phServiceHandle)
        {
            ServiceManagerApi.LW_SERVICE_STATUS serviceStatus = new ServiceManagerApi.LW_SERVICE_STATUS();

            try
            {
                Logger.Log("ServiceManagerInteropWrapper:ApiLwSmQueryServiceStatus()", Logger.ServiceManagerLoglevel);

                int iRet = ServiceManagerInterop.LwSmQueryServiceStatus(phServiceHandle, out serviceStatus);
                if (iRet != 0)
                {
                    Logger.Log("ServiceManagerInteropWrapper:ApiLwSmQueryServiceStatus returns" + iRet.ToString(), Logger.ServiceManagerLoglevel);
                    return serviceStatus;
                }
                Logger.Log("ServiceManagerInteropWrapper:ApiLwSmQueryServiceStatus is success", Logger.ServiceManagerLoglevel);
            }
            catch (Exception ex)
            {
                Logger.LogException("ServiceManagerInteropWrapper:ApiLwSmQueryServiceInfo", ex);
            }

            return serviceStatus;
        }

        public static ServiceManagerApi.LW_SERVICE_INFO ApiLwSmQueryServiceInfo(IntPtr phServiceHandle)
        {
            ServiceManagerApi.LW_SERVICE_INFO serviceInfo = new ServiceManagerApi.LW_SERVICE_INFO();
            IntPtr serviceInfoPtr = IntPtr.Zero;
            try
            {
                Logger.Log("ServiceManagerInteropWrapper:ApiLwSmQueryServiceInfo()", Logger.ServiceManagerLoglevel);

                int iRet = ServiceManagerInterop.LwSmQueryServiceInfo(phServiceHandle, out serviceInfoPtr);
                if (iRet != 0)
                {
                    Logger.Log("ServiceManagerInteropWrapper:ApiLwSmQueryServiceInfo returns" + iRet.ToString(), Logger.ServiceManagerLoglevel);
                    return serviceInfo;
                }
                Logger.Log("ServiceManagerInteropWrapper:ApiLwSmQueryServiceInfo is success", Logger.ServiceManagerLoglevel);

                serviceInfo = ServiceManagerInterop.UnmarshalServiceInfo(serviceInfoPtr);
            }
            catch (Exception ex)
            {
                Logger.LogException("ServiceManagerInteropWrapper:ApiLwSmQueryServiceInfo", ex);
            }

            return serviceInfo;
        }

        public static int ApiLwSmQueryServiceDependencyClosure(IntPtr phServiceHandle, out string[] serviceDependencies)
        {
            serviceDependencies = null;
            int iRet = 0;
            try
            {
                Logger.Log("ServiceManagerInteropWrapper:ApiLwSmQueryServiceDependencyClosure()", Logger.ServiceManagerLoglevel);

                iRet = ServiceManagerInterop.LwSmQueryServiceDependencyClosure(phServiceHandle, out serviceDependencies);
                if (iRet != 0)
                {
                    serviceDependencies = null;
                    Logger.Log("ServiceManagerInteropWrapper:ApiLwSmQueryServiceDependencyClosure returns" + iRet.ToString(), Logger.ServiceManagerLoglevel);
                }
                Logger.Log("ServiceManagerInteropWrapper:ApiLwSmQueryServiceDependencyClosure is success", Logger.ServiceManagerLoglevel);
            }
            catch (Exception ex)
            {
                Logger.LogException("ServiceManagerInteropWrapper:ApiLwSmQueryServiceDependencyClosure", ex);
            }

            return iRet;
        }

		 public static int ApiLwSmQueryServiceReverseDependencyClosure(IntPtr phServiceHandle, out string[] serviceDependencies)
        {
            serviceDependencies = null;
            int iRet = 0;
            try
            {
                Logger.Log("ServiceManagerInteropWrapper:ApiLwSmQueryServiceReverseDependencyClosure()", Logger.ServiceManagerLoglevel);

                iRet = ServiceManagerInterop.LwSmQueryServiceReverseDependencyClosure(phServiceHandle, out serviceDependencies);
                if (iRet != 0)
                {
                    serviceDependencies = null;
                    Logger.Log("ServiceManagerInteropWrapper:ApiLwSmQueryServiceReverseDependencyClosure returns" + iRet.ToString(), Logger.ServiceManagerLoglevel);
                }
                Logger.Log("ServiceManagerInteropWrapper:ApiLwSmQueryServiceReverseDependencyClosure is success", Logger.ServiceManagerLoglevel);
            }
            catch (Exception ex)
            {
                Logger.LogException("ServiceManagerInteropWrapper:ApiLwSmQueryServiceReverseDependencyClosure", ex);
            }

            return iRet;
        }


		public static int StartAllServiceDependencies(IntPtr pHandle, ref int iReturn)
        {
			try
			{
	            if (pHandle != IntPtr.Zero)
	            {
	                ServiceManagerApi.LW_SERVICE_INFO serviceInfo = ServiceManagerInteropWrapper.ApiLwSmQueryServiceInfo(pHandle);
					ServiceManagerApi.LW_SERVICE_STATUS serviceStatus = ServiceManagerInteropWrapper.ApiLwSmQueryServiceStatus(pHandle);

					if(serviceStatus.state == ServiceManagerApi.LW_SERVICE_STATE.LW_SERVICE_STATE_RUNNING)
					{
						Logger.Log("Dependency servcie: "+ serviceInfo.pwszName + " running");
						return iReturn;
					}

	                if (serviceInfo.ppwszDependencies != null && serviceInfo.ppwszDependencies.Length != 0) {
	                    foreach (string service in serviceInfo.ppwszDependencies)
	                    {
							Logger.Log("Dependency servcie: "+ service);

	                        IntPtr pDHandle = ServiceManagerInteropWrapper.ApiLwSmAcquireServiceHandle(service);
	                        if (pDHandle != IntPtr.Zero)
	                        {
	                            ServiceManagerApi.LW_SERVICE_INFO InnerServiceInfo = ServiceManagerInteropWrapper.ApiLwSmQueryServiceInfo(pDHandle);
								ServiceManagerApi.LW_SERVICE_STATUS Status = ServiceManagerInteropWrapper.ApiLwSmQueryServiceStatus(pDHandle);

	                            if (InnerServiceInfo.ppwszDependencies == null || InnerServiceInfo.ppwszDependencies.Length == 0)
								{
									if(Status.state != ServiceManagerApi.LW_SERVICE_STATE.LW_SERVICE_STATE_RUNNING)
									{
						iReturn = ServiceManagerInteropWrapper.ApiLwSmStartService(pDHandle);
						if (iReturn != 0)
		                                    return iReturn;
									}
									else
										Logger.Log("Dependency servcie: "+ service + " running");
	                            }
					else
								{
					StartAllServiceDependencies(pDHandle, ref iReturn);
									iReturn = ServiceManagerInteropWrapper.ApiLwSmStartService(pDHandle);
								}
								ServiceManagerInteropWrapper.ApiLwSmReleaseServiceHandle(pDHandle);
	                        }
	                    }
	                    iReturn = ServiceManagerInteropWrapper.ApiLwSmStartService(pHandle);
					}
	                else
					{
						if(serviceStatus.state != ServiceManagerApi.LW_SERVICE_STATE.LW_SERVICE_STATE_RUNNING)
						{
				iReturn = ServiceManagerInteropWrapper.ApiLwSmStartService(pHandle);
						}
	                }
				}
			}
			catch (Exception ex)
            {
                Logger.LogException("ServiceManagerInteropWrapper:StartAllServiceDependencies", ex);
            }

            return iReturn;
        }

        public static int StopServiceRecursive(IntPtr pHandle)
        {
            int iRet = 0;
            bool bIsDependenciesStopped = false;
            try
            {
                iRet = ServiceManagerInteropWrapper.ApiLwSmStopService(pHandle);
                if (iRet == (int)41205)
                {
                    string[] servicelist = null;
                    iRet = ApiLwSmQueryServiceReverseDependencyClosure(pHandle, out servicelist);
                    if (servicelist != null && servicelist.Length != 0)
                    {
                        foreach (string service in servicelist)
                        {
                            IntPtr pDHandle = ServiceManagerInteropWrapper.ApiLwSmAcquireServiceHandle(service);
                            StopServiceRecursive(pDHandle);
                        }
                        bIsDependenciesStopped = true;
                    }
                }

                if (bIsDependenciesStopped)
                {
                    iRet = ServiceManagerInteropWrapper.ApiLwSmStopService(pHandle);
                }
            }
            catch (Exception ex)
            {
                Logger.LogException("ServiceManagerInteropWrapper:StartAllServiceDependencies", ex);
            }
            return iRet;
        }
    }
}
