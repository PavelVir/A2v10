﻿// Copyright © 2015-2020 Alex Kukhtin. All rights reserved.


using System;
using System.Threading.Tasks;
using System.Dynamic;
using System.IO;
using System.Text;
using System.Threading;
using System.Web.Mvc;

using Microsoft.AspNet.Identity;

using Newtonsoft.Json;

using Stimulsoft.Report.Mvc;
using Stimulsoft.Report.Web;

using A2v10.Infrastructure;
using A2v10.Request;
using A2v10.Reports;
using A2v10.Web.Identity;
using A2v10.Interop;
using System.Web;
using System.Net.Http.Headers;
using A2v10.Web.Base;

namespace A2v10.Web.Mvc.Controllers
{

	public class EmptyView : IView, IViewDataContainer
	{
		public ViewDataDictionary ViewData { get; set; }
		public void Render(ViewContext viewContext, TextWriter writer)
		{
			// do nothing
		}
	}

	public class DesktopReport
	{
		public String Base;
		public String Report;
		public String Id;
		public String Format;
		public Int64 UserId;
		public Int32 TenantId;
		public Int64 CompanyId;
		public Boolean AddContentDisposition;
	}

	[Authorize]
	[ExecutingFilter]
	[CheckMobileFilter]
	public class ReportController : Controller, IControllerProfiler, IControllerTenant
	{
		A2v10.Request.BaseController _baseController = new BaseController();
		ReportHelper _reportHelper = new ReportHelper();

		public ReportController()
		{
			_baseController.Host.StartApplication(false);
		}

		public Int64 UserId => User.Identity.GetUserId<Int64>();
		public Int32 TenantId => User.Identity.GetUserTenantId();
		public String UserSegment => User.Identity.GetUserSegment();
		public Int64 CompanyId => _baseController.UserStateManager.UserCompanyId(TenantId, UserId);
		public IProfiler Profiler => _baseController.Host.Profiler;

		readonly static String[] _internalActions = new String[] { /*"GetReport", for profile query*/ "ViewerEvent", "PrintReport", "ExportReport", "Interaction" };

		public Boolean SkipRequest(String Url)
		{
			foreach (var a in _internalActions) {
				if (Url.Contains($"/{a}/"))
					return true;
			}
			return false;
		}

		[HttpGet]
		public async Task Show(String Base, String Rep, String id)
		{
			_reportHelper.SetupLicense();
			try
			{
				var url = $"/_report/{Base.RemoveHeadSlash()}/{Rep}/{id}";
				RequestModel rm = await RequestModel.CreateFromBaseUrl(_baseController.Host, false, url);
				var rep = rm.GetReport();

				MvcHtmlString result = null;
				using (var pr = Profiler.CurrentRequest.Start(ProfileAction.Report, $"render: {Rep}"))
				{
					var view = new EmptyView();
					var vc = new ViewContext(ControllerContext, view, ViewData, TempData, Response.Output);
					var hh = new HtmlHelper(vc, view);
					result = hh.Stimulsoft().StiMvcViewer("A2v10StiMvcViewer", ViewerOptions);
				}

				var sb = new StringBuilder(ResourceHelper.StiReportHtml);
				sb.Replace("$(StiReport)", result.ToHtmlString());
				sb.Replace("$(Lang)", _baseController.CurrentLang);
				sb.Replace("$(Title)", _baseController.Localize(rep.name ?? Rep)); 

				Response.Output.Write(sb.ToString());
			}
			catch (Exception ex)
			{
				if (ex.InnerException != null)
					ex = ex.InnerException;
				Response.Write(ex.Message);
			}
		}

		async Task<ReportInfo> GetReportInfo(String url, String id, ExpandoObject prms)
		{
			var rc = new ReportContext()
			{
				UserId = UserId,
				TenantId = TenantId,
			};
			if (_baseController.Host.IsMultiCompany)
				rc.CompanyId = CompanyId;
			return await _reportHelper.GetReportInfo(rc, url, id, prms);
		}

		async Task<ReportInfo> GetReportInfoDesktop(DesktopReport dr, String url, ExpandoObject prms)
		{
			var rc = new ReportContext()
			{
				UserId = dr.UserId,
				TenantId = dr.TenantId,
			};
			if (_baseController.Host.IsMultiCompany)
				rc.CompanyId = dr.CompanyId;
			return await _reportHelper.GetReportInfo(rc, url, dr.Id, prms);
		}

		ExpandoObject CreateParamsFromQueryString()
		{
			var eo = new ExpandoObject();
			if (Request == null)
				return eo;
			if (Request.QueryString.Count == 0)
				return eo;
			eo.Append(_baseController.CheckPeriod(Request.QueryString), toPascalCase: true);
			eo.RemoveKeys("rep,Rep,base,Base,Format,format");
			return eo;
		}

		public async Task ExportDesktop(DesktopReport rep, HttpResponseBase response)
		{
			// TODO: query string ???
			_reportHelper.SetupLicense();
			try
			{
				using (var rr = Profiler.CurrentRequest.Start(ProfileAction.Report, $"export: {rep.Report}"))
				{
					var url = $"/_report/{rep.Base.RemoveHeadSlash()}/{rep.Report}/{rep.Id}";
					ReportInfo ri = await GetReportInfoDesktop(rep, url, CreateParamsFromQueryString());
					ExportReportResult err = null;
					switch (ri.Type)
					{
						case RequestReportType.stimulsoft:
							err = _reportHelper.ExportStiReportStream(ri, rep.Format, response.OutputStream);
							break;
						case RequestReportType.xml:
							throw new NotImplementedException("ExportDesktop. RequestReportType.xml");
						case RequestReportType.json:
							throw new NotImplementedException("ExportDesktop. RequestReportType.json");
					}
					if (err != null)
					{
						response.ContentType = err.ContentType;
						if (rep.AddContentDisposition)
						{
							var cdh = new ContentDispositionHeaderValue("attachment")
							{
								FileNameStar = $"{_baseController.Localize(ri.Name)}.{err.Extension}"
							};
							response.Headers.Add("Content-Disposition", cdh.ToString());
						}
					}

				}
			}
			catch (Exception ex)
			{
				response.ContentType = "text/html";
				response.ContentEncoding = Encoding.UTF8;
				if (ex.InnerException != null)
					ex = ex.InnerException;
				response.Write(ex.Message);
			}
		}

		[HttpGet]
		[OutputCache(Duration = 0)]
		public async Task<ActionResult> Export(String Base, String Rep, String id, String Format)
		{
			_reportHelper.SetupLicense();
			try
			{
				using (var rr = Profiler.CurrentRequest.Start(ProfileAction.Report, $"export: {Rep}"))
				{
					var url = $"/_report/{Base.RemoveHeadSlash()}/{Rep}/{id}";
					ReportInfo ri = await GetReportInfo(url, id, CreateParamsFromQueryString());

					switch (ri.Type)
					{
						case RequestReportType.stimulsoft:
							return _reportHelper.ExportStiReport(ri, Format, saveFile: true);
						case RequestReportType.xml:
							return ExportXmlReport(ri);
						case RequestReportType.json:
							return ExportJsonReport(ri);
					}
				}
			}
			catch (Exception ex)
			{
				Response.ContentType = "text/html";
				Response.ContentEncoding = Encoding.UTF8;
				if (ex.InnerException != null)
					ex = ex.InnerException;
				Response.Write(ex.Message);
			}
			return new EmptyResult();
		}


		[HttpGet]
		[OutputCache(Duration = 0)]
		public async Task<ActionResult> Print(String Base, String Rep, String id, String Format)
		{
			_reportHelper.SetupLicense();
			try
			{
				var url = $"/_report/{Base.RemoveHeadSlash()}/{Rep}/{id}";
				ReportInfo ri = await GetReportInfo(url, id, CreateParamsFromQueryString());

				switch (ri.Type)
				{
					case RequestReportType.stimulsoft:
						return _reportHelper.ExportStiReport(ri, Format, saveFile: false);
					default:
						throw new NotImplementedException("ReportController.Print. ri.Type");
				}
			}
			catch (Exception ex)
			{
				Response.ContentType = "text/html";
				Response.ContentEncoding = Encoding.UTF8;
				if (ex.InnerException != null)
					ex = ex.InnerException;
				Response.Write(ex.Message);
			}
			return new EmptyResult();
		}

		ActionResult ExportJsonReport(ReportInfo ri)
		{
			String json = JsonConvert.SerializeObject(ri.DataModel.Root, Formatting.Indented);
			return Content(json, "application/json", Encoding.UTF8);
		}

		ActionResult ExportXmlReport(ReportInfo ri)
		{
			if (ri.XmlSchemaPathes == null)
				throw new RequestModelException("The xml-schemes are not specified");
			foreach (var path in ri.XmlSchemaPathes)
			{
				if (!System.IO.File.Exists(path))
					throw new RequestModelException($"File not found '{path}'");
			}
			if (String.IsNullOrEmpty(ri.Encoding))
				throw new RequestModelException("The xml encoding is not specified");
			var xmlCreator = new XmlCreator(ri.XmlSchemaPathes, ri.DataModel, ri.Encoding)
			{
				Validate = ri.Validate
			};
			var bytes = xmlCreator.CreateXml();
			if (xmlCreator.HasErrors)
				throw new Exception(xmlCreator.ErrorMessage);
			return File(bytes, "text/xml", $"{ri.Name}.xml");
		}

		private String LocaleKey => Thread.CurrentThread.CurrentUICulture.TwoLetterISOLanguageName;

		private StiMvcViewerOptions ViewerOptions {
			get {
				String localeFile = $"~/Localization/{LocaleKey}.xml";
				return new StiMvcViewerOptions()
				{
					Theme = StiViewerTheme.Office2013LightGrayBlue,
					Localization = localeFile,
					Server = new StiMvcViewerOptions.ServerOptions()
					{
						Controller = "StiReport",
						RequestTimeout = 300,
						UseRelativeUrls = true
					},
					Actions = new StiMvcViewerOptions.ActionOptions()
					{
						GetReport = "GetReport",
						ViewerEvent = "ViewerEvent",
						PrintReport = "PrintReport",
						ExportReport = "ExportReport",
						Interaction = "Interaction",
					},
					Appearance = new StiMvcViewerOptions.AppearanceOptions()
					{
						BackgroundColor = System.Drawing.Color.FromArgb(0x00e3e3e3),
						ShowTooltips = false,
						ScrollbarsMode = true,
						FullScreenMode = true,
					},
					Toolbar = new StiMvcViewerOptions.ToolbarOptions()
					{
						MenuAnimation = false,
						ShowFullScreenButton = false,
						ShowMenuMode = StiShowMenuMode.Click,
						//FontFamily = "system-ui, 'Segoe UI', Tahoma, Verdana, sans-serif",
						//FontColor = System.Drawing.Color.FromArgb(0x00333333),
						ShowBookmarksButton = false,
						ShowParametersButton = true,
						ShowSendEmailButton = false,
					},
					Exports = new StiMvcViewerOptions.ExportOptions()
					{
						DefaultSettings = StiReportExtensions.GetExportSettings()
					}
				};
			}
		}

		public async Task<ActionResult> GetReport()
		{
			try
			{
				var rp = StiMvcViewer.GetRequestParams();
				var Rep = rp.HttpContext.Request.Params["Rep"];
				var Base = rp.HttpContext.Request.Params["Base"];
				var id = rp.Routes["Id"];
				var url = $"/_report/{Base.RemoveHeadSlash()}/{Rep}/{id}";

				//TODO: profile var token = Profiler.BeginReport("create");
				var prms = new ExpandoObject();
				prms.Append(_baseController.CheckPeriod(rp.HttpContext.Request.QueryString), toPascalCase: true);
				prms.RemoveKeys("Rep,rep,Base,base,Format,format");

				ReportInfo ri = await GetReportInfo(url, id, prms);
				//TODO: image settings var rm = rm.ImageInfo;
				if (ri == null)
					throw new InvalidProgramException("invalid data");
				var path = ri.ReportPath;
				using (var stream = ri.GetStream(_baseController.Host.ApplicationReader))
				{
					var r = StiReportExtensions.CreateReport(stream, ri.Name);
					r.AddDataModel(ri.DataModel);
					var vars = ri.Variables;
					if (vars != null)
						r.AddVariables(vars);
					return StiMvcViewer.GetReportResult(r);
				}
			}
			catch (Exception ex)
			{
				String msg = ex.Message;
				Int32 x = msg.IndexOf(": error");
				if (x != -1)
					msg = msg.Substring(x + 7).Trim();
				return new HttpStatusCodeResult(500, msg);
			}
		}

		public ActionResult ViewerEvent()
		{
			return StiMvcViewer.ViewerEventResult();
		}

		public ActionResult PrintReport()
		{
			return StiMvcViewer.PrintReportResult();
		}

		public ActionResult ExportReport()
		{
			return StiMvcViewer.ExportReportResult();
		}

		public ActionResult Interaction()
		{
			return StiMvcViewer.InteractionResult();
		}

		#region IControllerTenant
		public void StartTenant()
		{
			var host = ServiceLocator.Current.GetService<IApplicationHost>();
			host.TenantId = TenantId;
			host.UserId = UserId;
			host.UserSegment = UserSegment;
		}
		#endregion

	}
}
