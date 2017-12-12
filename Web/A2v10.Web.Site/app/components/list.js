﻿// Copyright © 2015-2017 Alex Kukhtin. All rights reserved.

// 20171212-7078
// components/list.js

(function() {

    const utils = require('std:utils');

    Vue.component("a2-list", {
        template:
`<ul class="a2-list" v-lazy="itemsSource">
    <template v-if="itemsSource">
	    <li class="a2-list-item" tabindex="1" :class="cssClass(listItem)" v-for="(listItem, listItemIndex) in itemsSource" :key="listItemIndex" @click.prevent="select(listItem)" @keydown="keyDown">
            <slot name="items" :item="listItem" />
	    </li>
    </template>
    <template v-else>
        <slot />
    </template>
</ul>
`,
        props: {
            itemsSource: Array,
            autoSelect: String,
            mark: String
        },
        computed: {
            isSelectFirstItem() {
                return this.autoSelect === 'first-item';
            },
            selectedSource() {
                // method! not cached
                let src = this.itemsSource;
                if (!src) return null;
                if (src.$origin)
                    src = src.$origin;
                return src.$selected;
            }
        },
        methods: {
            cssClass(item) {
                let cls = item.$selected ? 'active' : '';
                if (this.mark) {
                    let clsmrk = utils.eval(item, this.mark);
                    if (clsmrk) cls += ' ' + clsmrk;
                }
                return cls;
            },
            select(item) {
                if (item.$select) item.$select();
            },
            selectStatic() {
                alert('yet not implemented');
                console.dir(this);
            },
            selectFirstItem() {
                if (!this.isSelectFirstItem)
                    return;
                // from source (not $origin!)
                let src = this.itemsSource;
                if (!src.length)
                    return;
                let fe = src[0];
                this.select(fe);
            },
            keyDown(e) {
                const next = (delta) => {
                    let index;
                    index = this.itemsSource.indexOf(this.selectedSource);
                    if (index == -1)
                        return;
                    index += delta;
                    if (index == -1)
                        return;
                    if (index < this.itemsSource.length)
                        this.select(this.itemsSource[index]);
                };
                switch (e.which) {
                    case 38: // up
                        next(-1);
                        break;
                    case 40: // down
                        next(1);
                        break;
                    case 36: // home
                        //this.selected = this.itemsSource[0];
                        break;
                    case 35: // end
                        //this.selected = this.itemsSource[this.itemsSource.length - 1];
                        break;
                    case 33: // pgUp
                        break;
                    case 34: // pgDn
                        break;
                    default:
                        return;
                }
                e.preventDefault();
                e.stopPropagation();
            }
        },
        created() {
            this.selectFirstItem();
        },
        updated() {
            if (!this.selectedSource && this.isSelectFirstItem) {
                this.selectFirstItem();
            }
        }
    });
})();
