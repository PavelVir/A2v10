﻿//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     Runtime Version:4.0.30319.42000
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

namespace A2v10.Runtime.Properties {
    using System;
    
    
    /// <summary>
    ///   A strongly-typed resource class, for looking up localized strings, etc.
    /// </summary>
    // This class was auto-generated by the StronglyTypedResourceBuilder
    // class via a tool like ResGen or Visual Studio.
    // To add or remove a member, edit your .ResX file then rerun ResGen
    // with the /str option, or rebuild your VS project.
    [global::System.CodeDom.Compiler.GeneratedCodeAttribute("System.Resources.Tools.StronglyTypedResourceBuilder", "15.0.0.0")]
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
    [global::System.Runtime.CompilerServices.CompilerGeneratedAttribute()]
    internal class Resources {
        
        private static global::System.Resources.ResourceManager resourceMan;
        
        private static global::System.Globalization.CultureInfo resourceCulture;
        
        [global::System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
        internal Resources() {
        }
        
        /// <summary>
        ///   Returns the cached ResourceManager instance used by this class.
        /// </summary>
        [global::System.ComponentModel.EditorBrowsableAttribute(global::System.ComponentModel.EditorBrowsableState.Advanced)]
        internal static global::System.Resources.ResourceManager ResourceManager {
            get {
                if (object.ReferenceEquals(resourceMan, null)) {
                    global::System.Resources.ResourceManager temp = new global::System.Resources.ResourceManager("A2v10.Runtime.Properties.Resources", typeof(Resources).Assembly);
                    resourceMan = temp;
                }
                return resourceMan;
            }
        }
        
        /// <summary>
        ///   Overrides the current thread's CurrentUICulture property for all
        ///   resource lookups using this strongly typed resource class.
        /// </summary>
        [global::System.ComponentModel.EditorBrowsableAttribute(global::System.ComponentModel.EditorBrowsableState.Advanced)]
        internal static global::System.Globalization.CultureInfo Culture {
            get {
                return resourceCulture;
            }
            set {
                resourceCulture = value;
            }
        }
        
        /// <summary>
        ///   Looks up a localized string similar to /*
        ///global elements for context
        ///*/
        ///
        ///(function () {
        ///    // this = global context
        ///
        ///    let g = this;
        ///    const modules = {};
        ///
        ///    g.require = function (module) {
        ///        if (!(module in modules))
        ///            modules[module] = __require(module, g.__context._dir_);
        ///        return modules[module];
        ///    };
        ///
        ///    g.__context = {
        ///        _file_: null,
        ///        _dir_: null
        ///    };
        ///})();
        ///.
        /// </summary>
        internal static string App_context {
            get {
                return ResourceManager.GetString("App_context", resourceCulture);
            }
        }
        
        /// <summary>
        ///   Looks up a localized string similar to 
        ///// global variables!
        ///
        ///var designer = (function () {
        ///
        ///    function createElement(name, ...args) {
        ///        if (name in this.elements) {
        ///            return new this.elements[name](...args);
        ///        }
        ///        console.error(`__createElement. Element &apos;${name}&apos; not found`);
        ///        return null;
        ///    }
        ///
        ///    let designer = {
        ///        form: {
        ///            elements: {},
        ///            __createElement: createElement,
        ///            __registerElement(ctor) {
        ///                var name = ctor.prototype.type;
        ///    [rest of string was truncated]&quot;;.
        /// </summary>
        internal static string Application {
            get {
                return ResourceManager.GetString("Application", resourceCulture);
            }
        }
        
        /// <summary>
        ///   Looks up a localized string similar to (function () {
        ///
        ///    function Form() {
        ///        // form properties
        ///        this.Width = 150;
        ///        this.Height = 200;
        ///        this.Title = &quot;Form title from JS&quot;;
        ///	}
        ///
        ///	Form.prototype.type = &quot;Form&quot;;
        ///
        ///	Form.prototype._meta_ = {
        ///		properties: {
        ///            Title: {
        ///                category: &quot;Appearance&quot;,
        ///                type: &quot;string&quot;,
        ///                description: &quot;Specifies the text that will be displayed in the form&apos;s title bar&quot;
        ///            },
        ///			Width: {
        ///				category: &quot;Layout&quot;,
        ///				type: &quot; [rest of string was truncated]&quot;;.
        /// </summary>
        internal static string Form_form {
            get {
                return ResourceManager.GetString("Form_form", resourceCulture);
            }
        }
        
        /// <summary>
        ///   Looks up a localized string similar to 
        ///(function () {
        ///
        ///    const cats = {
        ///        general: &quot;Общие&quot;
        ///    };
        ///
        ///    function copyProps(trg, src) {
        ///        for (let p in trg._meta_) {
        ///            let sprop = src[p];
        ///            let mi = trg._meta_[p];
        ///        }
        ///    }
        ///
        ///    function Column(src) {
        ///        copyProps(this, src);
        ///    }
        ///
        ///    Column.prototype._meta_ = {
        ///        Name: {
        ///            category: cats.general,
        ///            type: &quot;string&quot;,
        ///            description: &quot;Наименование колонки таблицы&quot;
        ///        }
        ///    }
        ///
        ///    function [rest of string was truncated]&quot;;.
        /// </summary>
        internal static string Solution {
            get {
                return ResourceManager.GetString("Solution", resourceCulture);
            }
        }
    }
}
