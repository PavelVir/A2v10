﻿// Copyright © 2015-2019 Oleksandr Kukhtin. All rights reserved.

// 20190309-7488
// components/panel.js

Vue.component('a2-panel', {
	template:
`<div :class="cssClass" :test-id="testId">
	<div class="panel-header" @click.prevent="toggle" v-if="!noHeader">
		<slot name='header'></slot>
		<span v-if="collapsible" class="ico panel-collapse-handle"></span>
	</div>
	<slot v-if="expanded"></slot>
</div>
`,
	props: {
		initialCollapsed: Boolean,
		collapsible: Boolean,
		panelStyle: String,
		noHeader: Boolean,
		testId: String
	},
	data() {
		return {
			collapsed: this.initialCollapsed
		};
	},
	computed: {
		cssClass() {
			let cls = "panel";
			if (this.collapsed) cls += ' collapsed'; else cls += ' expanded';
			if (this.panelStyle) {
				let ps = this.panelStyle.toLowerCase(); 
				switch (ps) {
					case "red":
					case "danger":
					case "error":
						cls += ' panel-red';
						break;
					case "info":
					case "cyan":
						cls += ' panel-cyan';
						break;
					case "green":
					case "success":
						cls += ' panel-green';
						break;
					case "warning":
					case "yellow":
						cls += ' panel-yellow';
						break;
					default:
						cls += ' panel-' + ps;
						break;
				}
			}
			return cls;
		},
		expanded() {
			return !this.collapsed;
		}
	},
	methods: {
		toggle() {
			if (!this.collapsible)
				return;
			this.collapsed = !this.collapsed;
		}
	}
});