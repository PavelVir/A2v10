﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using A2v10.Infrastructure;
using System.Xaml;

namespace A2v10.Xaml
{
    public class XamlRenderer : IRenderer
    {
        public void Render(RenderInfo info)
        {
            if (String.IsNullOrEmpty(info.FileName))
                throw new XamlException("No source for render");

            String fileName = String.Empty;
            // XamlServices.Load sets IUriContext
            UIElement uiElem = XamlServices.Load(info.FileName) as UIElement;
            if (uiElem == null)
                throw new XamlException("Xaml. Root is not UIElement");

            RenderContext ctx = new RenderContext(info.Writer);
            ctx.RootId = info.RootId; 
            uiElem.RenderElement(ctx);
        }
    }
}