﻿/*20170901-7022*/
/* platform/routex.js */

(function () {

	const eventBus = require('std:eventBus');

	// TODO:

	// 1: save/restore query (localStorage)

	function parseQueryString(str) {
		var obj = {};
		str.replace(/\??([^=&]+)=([^&]*)/g, function (m, key, value) {
			obj[decodeURIComponent(key)] = decodeURIComponent(value);
		});
		return obj;
	}

	function makeQueryString(obj) {
		if (!obj)
			return '';
		let esc = encodeURIComponent;
		let query = Object.keys(obj)
			.filter(k => obj[k])
			.map(k => esc(k) + '=' + esc(obj[k]))
			.join('&');
		return query ? '?' + query : '';
	}

	const titleStore = {};

	function setTitle(to) {
		if (to.title) {
			document.title = to.title;
			titleStore[to.url] = to.title;
		}
	}

	function makeBackUrl(url) {
		let urlArr = url.split('/');
		if (urlArr.length === 5)
			return urlArr.slice(0, 3).join('/');
		else if (url.length === 4)
			return urlArr.slice(0, 2).join('/');
		return url;
	}

	const store = new Vuex.Store({
		state: {
			route: window.location.pathname,
			query: parseQueryString(window.location.search)
		},
		getters: {
			seg0: (state) => state.route.split('/')[1],
			seg1: (state) => state.route.split('/')[2],
			len: (state) => state.route.split('/').length,
			url: (state) => state.route,
			query: (state) => state.query,
			route: (state) => {
				let sr = state.route.split('/');
				return {
					len: sr.length,
					seg0: sr[1],
					seg1: sr[2]
				};
			},
			baseUrl: (state) => {
				return state.route + makeQueryString(state.query);
			},
			search: (state) => {
				return makeQueryString(state.query);
			}		
		},
		mutations: {
			navigate(state, to) { // to: {url, query, title}
				let oldUrl = state.route + makeQueryString(state.query);
				state.route = to.url;
				state.query = Object.assign({}, to.query);
				let newUrl = state.route + makeQueryString(to.query);
				let h = window.history;
				setTitle(to);
				// push/pop state feature. Replace the current state and push new one.
				h.replaceState(oldUrl, null, oldUrl);
				h.pushState(oldUrl, null, newUrl);
			},
			query(state, query) {
				// changes all query
				state.query = Object.assign({}, query);
				let newUrl = state.route + makeQueryString(state.query);
				//console.warn('set query: ' + newUrl);
				window.history.replaceState(null, null, newUrl);
			},
			setquery(state, query) {
				// changes some fields or query
				state.query = Object.assign({}, state.query, query);
				let newUrl = state.route + makeQueryString(state.query);
				// TODO: replaceUrl: boolean
				//console.warn('set setquery: ' + newUrl);
				window.history.replaceState(null, null, newUrl);
				eventBus.$emit('queryChange', makeQueryString(state.query));
			},
			popstate(state) {
				state.route = window.location.pathname;
				state.query = parseQueryString(window.location.search);
				if (state.route in titleStore) {
					document.title = titleStore[state.route];
				}
			},
			setstate(state, to ){ // to: {url, title}
				//console.warn('set setstate: ' + url);
				window.history.replaceState(null, null, to.url);
				state.route = window.location.pathname;
				state.query = parseQueryString(window.location.search);
				setTitle(to);
			},
			close(state) {
				if (window.history.length)
					window.history.back();
				else
					store.commit('navigate', { url: makeBackUrl(state.route) });
			}
		}
	});

	function replaceUrlSearch(url, search) {
		let parts = url.split('?');
		return parts[0] + (search || '');
	}

	function replaceUrlQuery(url, query) {
		return replaceUrlSearch(url, makeQueryString(query));
	}

	store.parseQueryString = parseQueryString;
	store.makeQueryString = makeQueryString;
	store.replaceUrlSearch = replaceUrlSearch;
	store.replaceUrlQuery = replaceUrlQuery;
	store.makeBackUrl = makeBackUrl;

	app.components['std:store'] = store;
})();