﻿
using System;
using System.Reflection;
using System.Windows.Markup;
using System.Xaml;

namespace A2v10.Xaml
{
	public class Source : MarkupExtension
	{
		public String Path { get; set; }
		public Source()
		{

		}
		public Source(String path)
		{
			Path = path;
		}

		public override object ProvideValue(IServiceProvider serviceProvider)
		{
			try
			{
				IProvideValueTarget iTarget = serviceProvider.GetService(typeof(IProvideValueTarget)) as IProvideValueTarget;
				if (iTarget == null)
					return null;
				var targetProp = iTarget.TargetProperty as PropertyInfo;
				if (targetProp == null)
					return null;
				if (targetProp.PropertyType != typeof(Object) && targetProp.PropertyType != typeof(UIElement))
				{
					throw new XamlException("The 'Source' markup extension can only be used for properties that are of type 'System.Object' or 'A2v10.Xaml.UIElement'");
				}

				var root = serviceProvider.GetService(typeof(IUriContext)) as IUriContext;
				return Load(Path, root.BaseUri);
			}
			catch (Exception ex)
			{
				return null; //TODO: new Span() { CssClass = "xaml-alert alert alert-danger", Content = ex.Message };
			}

			Object Load(String path, Uri baseUri)
			{
				String basePath = System.IO.Path.GetDirectoryName(baseUri.PathAndQuery);
				String targetDir = System.IO.Path.Combine(basePath, path);
				if (System.IO.Path.GetExtension(targetDir).ToLowerInvariant() == ".js")
				{
					if (System.IO.File.Exists(targetDir))
						return System.IO.File.ReadAllText(targetDir);
					else
						throw new XamlException($"File not found {path}");
				}
				else
				{
					String trgPath = System.IO.Path.ChangeExtension(targetDir, "xaml");
					if (System.IO.File.Exists(trgPath))
						return XamlServices.Load(trgPath);
					else
						throw new XamlException($"File not found {path}");
				}
			}

		}
	}
}