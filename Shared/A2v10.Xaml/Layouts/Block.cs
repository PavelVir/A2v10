﻿
using System;
using System.Windows.Markup;

namespace A2v10.Xaml
{
    [ContentProperty("Children")]
    public class Block : UIElementBase
    {

        public UIElementCollection Children { get; set; } = new UIElementCollection();

        internal virtual void RenderChildren(RenderContext context)
        {
            foreach (var c in Children)
            {
                c.RenderElement(context);
            }
        }

        internal override void RenderElement(RenderContext context, Action<TagBuilder> onRender = null)
        {
            var div = new TagBuilder("div");
            if (onRender != null)
                onRender(div);
            div.RenderStart(context);
            RenderChildren(context);
            div.RenderEnd(context);
        }

        protected override void OnEndInit()
        {
            base.OnEndInit();
            foreach (var c in Children)
                c.SetParent(this);
        }

    }
}