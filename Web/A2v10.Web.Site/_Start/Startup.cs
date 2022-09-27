﻿// Copyright © 2015-2022 Alex Kukhtin. All rights reserved.

using System;
using A2v10.Web.Mvc;


namespace A2v10.Application
{
	public static class AppStartup
	{
		public static Object CreateLicenseManager()
		{
			return LicenseManager.Create();
		}
	}
}