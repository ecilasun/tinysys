(function(){/*

 Copyright The Closure Library Authors.
 SPDX-License-Identifier: Apache-2.0
*/
'use strict';var q;function ca(a){var b=0;return function(){return b<a.length?{done:!1,value:a[b++]}:{done:!0}}}
var da="function"==typeof Object.defineProperties?Object.defineProperty:function(a,b,c){if(a==Array.prototype||a==Object.prototype)return a;a[b]=c.value;return a};
function ea(a){a=["object"==typeof globalThis&&globalThis,a,"object"==typeof window&&window,"object"==typeof self&&self,"object"==typeof global&&global];for(var b=0;b<a.length;++b){var c=a[b];if(c&&c.Math==Math)return c}throw Error("Cannot find global object");}
var fa=ea(this);function t(a,b){if(b)a:{var c=fa;a=a.split(".");for(var d=0;d<a.length-1;d++){var e=a[d];if(!(e in c))break a;c=c[e]}a=a[a.length-1];d=c[a];b=b(d);b!=d&&null!=b&&da(c,a,{configurable:!0,writable:!0,value:b})}}
t("Symbol",function(a){function b(f){if(this instanceof b)throw new TypeError("Symbol is not a constructor");return new c(d+(f||"")+"_"+e++,f)}
function c(f,g){this.h=f;da(this,"description",{configurable:!0,writable:!0,value:g})}
if(a)return a;c.prototype.toString=function(){return this.h};
var d="jscomp_symbol_"+(1E9*Math.random()>>>0)+"_",e=0;return b});
t("Symbol.iterator",function(a){if(a)return a;a=Symbol("Symbol.iterator");for(var b="Array Int8Array Uint8Array Uint8ClampedArray Int16Array Uint16Array Int32Array Uint32Array Float32Array Float64Array".split(" "),c=0;c<b.length;c++){var d=fa[b[c]];"function"===typeof d&&"function"!=typeof d.prototype[a]&&da(d.prototype,a,{configurable:!0,writable:!0,value:function(){return ha(ca(this))}})}return a});
function ha(a){a={next:a};a[Symbol.iterator]=function(){return this};
return a}
function u(a){var b="undefined"!=typeof Symbol&&Symbol.iterator&&a[Symbol.iterator];if(b)return b.call(a);if("number"==typeof a.length)return{next:ca(a)};throw Error(String(a)+" is not an iterable or ArrayLike");}
function ka(a){if(!(a instanceof Array)){a=u(a);for(var b,c=[];!(b=a.next()).done;)c.push(b.value);a=c}return a}
function la(a,b){return Object.prototype.hasOwnProperty.call(a,b)}
var ma="function"==typeof Object.assign?Object.assign:function(a,b){for(var c=1;c<arguments.length;c++){var d=arguments[c];if(d)for(var e in d)la(d,e)&&(a[e]=d[e])}return a};
t("Object.assign",function(a){return a||ma});
var na="function"==typeof Object.create?Object.create:function(a){function b(){}
b.prototype=a;return new b},oa;
if("function"==typeof Object.setPrototypeOf)oa=Object.setPrototypeOf;else{var pa;a:{var qa={a:!0},ra={};try{ra.__proto__=qa;pa=ra.a;break a}catch(a){}pa=!1}oa=pa?function(a,b){a.__proto__=b;if(a.__proto__!==b)throw new TypeError(a+" is not extensible");return a}:null}var sa=oa;
function v(a,b){a.prototype=na(b.prototype);a.prototype.constructor=a;if(sa)sa(a,b);else for(var c in b)if("prototype"!=c)if(Object.defineProperties){var d=Object.getOwnPropertyDescriptor(b,c);d&&Object.defineProperty(a,c,d)}else a[c]=b[c];a.pa=b.prototype}
function ta(){this.H=!1;this.m=null;this.i=void 0;this.h=1;this.v=this.s=0;this.X=this.j=null}
function ua(a){if(a.H)throw new TypeError("Generator is already running");a.H=!0}
ta.prototype.N=function(a){this.i=a};
function va(a,b){a.j={ec:b,pc:!0};a.h=a.s||a.v}
ta.prototype.return=function(a){this.j={return:a};this.h=this.v};
function w(a,b,c){a.h=c;return{value:b}}
ta.prototype.D=function(a){this.h=a};
function ya(a,b,c){a.s=b;void 0!=c&&(a.v=c)}
function Aa(a,b){a.h=b;a.s=0}
function Ba(a){a.s=0;var b=a.j.ec;a.j=null;return b}
function Ca(a){a.X=[a.j];a.s=0;a.v=0}
function Da(a){var b=a.X.splice(0)[0];(b=a.j=a.j||b)?b.pc?a.h=a.s||a.v:void 0!=b.D&&a.v<b.D?(a.h=b.D,a.j=null):a.h=a.v:a.h=0}
function Ea(a){this.h=new ta;this.i=a}
function Fa(a,b){ua(a.h);var c=a.h.m;if(c)return Ga(a,"return"in c?c["return"]:function(d){return{value:d,done:!0}},b,a.h.return);
a.h.return(b);return Ia(a)}
function Ga(a,b,c,d){try{var e=b.call(a.h.m,c);if(!(e instanceof Object))throw new TypeError("Iterator result "+e+" is not an object");if(!e.done)return a.h.H=!1,e;var f=e.value}catch(g){return a.h.m=null,va(a.h,g),Ia(a)}a.h.m=null;d.call(a.h,f);return Ia(a)}
function Ia(a){for(;a.h.h;)try{var b=a.i(a.h);if(b)return a.h.H=!1,{value:b.value,done:!1}}catch(c){a.h.i=void 0,va(a.h,c)}a.h.H=!1;if(a.h.j){b=a.h.j;a.h.j=null;if(b.pc)throw b.ec;return{value:b.return,done:!0}}return{value:void 0,done:!0}}
function Ja(a){this.next=function(b){ua(a.h);a.h.m?b=Ga(a,a.h.m.next,b,a.h.N):(a.h.N(b),b=Ia(a));return b};
this.throw=function(b){ua(a.h);a.h.m?b=Ga(a,a.h.m["throw"],b,a.h.N):(va(a.h,b),b=Ia(a));return b};
this.return=function(b){return Fa(a,b)};
this[Symbol.iterator]=function(){return this}}
function Ka(a){function b(d){return a.next(d)}
function c(d){return a.throw(d)}
return new Promise(function(d,e){function f(g){g.done?d(g.value):Promise.resolve(g.value).then(b,c).then(f,e)}
f(a.next())})}
function x(a){return Ka(new Ja(new Ea(a)))}
function La(){for(var a=Number(this),b=[],c=a;c<arguments.length;c++)b[c-a]=arguments[c];return b}
t("Reflect.setPrototypeOf",function(a){return a?a:sa?function(b,c){try{return sa(b,c),!0}catch(d){return!1}}:null});
t("Promise",function(a){function b(g){this.h=0;this.j=void 0;this.i=[];this.H=!1;var h=this.m();try{g(h.resolve,h.reject)}catch(k){h.reject(k)}}
function c(){this.h=null}
function d(g){return g instanceof b?g:new b(function(h){h(g)})}
if(a)return a;c.prototype.i=function(g){if(null==this.h){this.h=[];var h=this;this.j(function(){h.v()})}this.h.push(g)};
var e=fa.setTimeout;c.prototype.j=function(g){e(g,0)};
c.prototype.v=function(){for(;this.h&&this.h.length;){var g=this.h;this.h=[];for(var h=0;h<g.length;++h){var k=g[h];g[h]=null;try{k()}catch(l){this.m(l)}}}this.h=null};
c.prototype.m=function(g){this.j(function(){throw g;})};
b.prototype.m=function(){function g(l){return function(m){k||(k=!0,l.call(h,m))}}
var h=this,k=!1;return{resolve:g(this.Gb),reject:g(this.v)}};
b.prototype.Gb=function(g){if(g===this)this.v(new TypeError("A Promise cannot resolve to itself"));else if(g instanceof b)this.Gc(g);else{a:switch(typeof g){case "object":var h=null!=g;break a;case "function":h=!0;break a;default:h=!1}h?this.Fb(g):this.s(g)}};
b.prototype.Fb=function(g){var h=void 0;try{h=g.then}catch(k){this.v(k);return}"function"==typeof h?this.Hc(h,g):this.s(g)};
b.prototype.v=function(g){this.N(2,g)};
b.prototype.s=function(g){this.N(1,g)};
b.prototype.N=function(g,h){if(0!=this.h)throw Error("Cannot settle("+g+", "+h+"): Promise already settled in state"+this.h);this.h=g;this.j=h;2===this.h&&this.Hb();this.X()};
b.prototype.Hb=function(){var g=this;e(function(){if(g.Ga()){var h=fa.console;"undefined"!==typeof h&&h.error(g.j)}},1)};
b.prototype.Ga=function(){if(this.H)return!1;var g=fa.CustomEvent,h=fa.Event,k=fa.dispatchEvent;if("undefined"===typeof k)return!0;"function"===typeof g?g=new g("unhandledrejection",{cancelable:!0}):"function"===typeof h?g=new h("unhandledrejection",{cancelable:!0}):(g=fa.document.createEvent("CustomEvent"),g.initCustomEvent("unhandledrejection",!1,!0,g));g.promise=this;g.reason=this.j;return k(g)};
b.prototype.X=function(){if(null!=this.i){for(var g=0;g<this.i.length;++g)f.i(this.i[g]);this.i=null}};
var f=new c;b.prototype.Gc=function(g){var h=this.m();g.tb(h.resolve,h.reject)};
b.prototype.Hc=function(g,h){var k=this.m();try{g.call(h,k.resolve,k.reject)}catch(l){k.reject(l)}};
b.prototype.then=function(g,h){function k(r,p){return"function"==typeof r?function(y){try{l(r(y))}catch(A){m(A)}}:p}
var l,m,n=new b(function(r,p){l=r;m=p});
this.tb(k(g,l),k(h,m));return n};
b.prototype.catch=function(g){return this.then(void 0,g)};
b.prototype.tb=function(g,h){function k(){switch(l.h){case 1:g(l.j);break;case 2:h(l.j);break;default:throw Error("Unexpected state: "+l.h);}}
var l=this;null==this.i?f.i(k):this.i.push(k);this.H=!0};
b.resolve=d;b.reject=function(g){return new b(function(h,k){k(g)})};
b.race=function(g){return new b(function(h,k){for(var l=u(g),m=l.next();!m.done;m=l.next())d(m.value).tb(h,k)})};
b.all=function(g){var h=u(g),k=h.next();return k.done?d([]):new b(function(l,m){function n(y){return function(A){r[y]=A;p--;0==p&&l(r)}}
var r=[],p=0;do r.push(void 0),p++,d(k.value).tb(n(r.length-1),m),k=h.next();while(!k.done)})};
return b});
t("WeakMap",function(a){function b(k){this.h=(h+=Math.random()+1).toString();if(k){k=u(k);for(var l;!(l=k.next()).done;)l=l.value,this.set(l[0],l[1])}}
function c(){}
function d(k){var l=typeof k;return"object"===l&&null!==k||"function"===l}
function e(k){if(!la(k,g)){var l=new c;da(k,g,{value:l})}}
function f(k){var l=Object[k];l&&(Object[k]=function(m){if(m instanceof c)return m;Object.isExtensible(m)&&e(m);return l(m)})}
if(function(){if(!a||!Object.seal)return!1;try{var k=Object.seal({}),l=Object.seal({}),m=new a([[k,2],[l,3]]);if(2!=m.get(k)||3!=m.get(l))return!1;m.delete(k);m.set(l,4);return!m.has(k)&&4==m.get(l)}catch(n){return!1}}())return a;
var g="$jscomp_hidden_"+Math.random();f("freeze");f("preventExtensions");f("seal");var h=0;b.prototype.set=function(k,l){if(!d(k))throw Error("Invalid WeakMap key");e(k);if(!la(k,g))throw Error("WeakMap key fail: "+k);k[g][this.h]=l;return this};
b.prototype.get=function(k){return d(k)&&la(k,g)?k[g][this.h]:void 0};
b.prototype.has=function(k){return d(k)&&la(k,g)&&la(k[g],this.h)};
b.prototype.delete=function(k){return d(k)&&la(k,g)&&la(k[g],this.h)?delete k[g][this.h]:!1};
return b});
t("Map",function(a){function b(){var h={};return h.previous=h.next=h.head=h}
function c(h,k){var l=h.h;return ha(function(){if(l){for(;l.head!=h.h;)l=l.previous;for(;l.next!=l.head;)return l=l.next,{done:!1,value:k(l)};l=null}return{done:!0,value:void 0}})}
function d(h,k){var l=k&&typeof k;"object"==l||"function"==l?f.has(k)?l=f.get(k):(l=""+ ++g,f.set(k,l)):l="p_"+k;var m=h.data_[l];if(m&&la(h.data_,l))for(h=0;h<m.length;h++){var n=m[h];if(k!==k&&n.key!==n.key||k===n.key)return{id:l,list:m,index:h,entry:n}}return{id:l,list:m,index:-1,entry:void 0}}
function e(h){this.data_={};this.h=b();this.size=0;if(h){h=u(h);for(var k;!(k=h.next()).done;)k=k.value,this.set(k[0],k[1])}}
if(function(){if(!a||"function"!=typeof a||!a.prototype.entries||"function"!=typeof Object.seal)return!1;try{var h=Object.seal({x:4}),k=new a(u([[h,"s"]]));if("s"!=k.get(h)||1!=k.size||k.get({x:4})||k.set({x:4},"t")!=k||2!=k.size)return!1;var l=k.entries(),m=l.next();if(m.done||m.value[0]!=h||"s"!=m.value[1])return!1;m=l.next();return m.done||4!=m.value[0].x||"t"!=m.value[1]||!l.next().done?!1:!0}catch(n){return!1}}())return a;
var f=new WeakMap;e.prototype.set=function(h,k){h=0===h?0:h;var l=d(this,h);l.list||(l.list=this.data_[l.id]=[]);l.entry?l.entry.value=k:(l.entry={next:this.h,previous:this.h.previous,head:this.h,key:h,value:k},l.list.push(l.entry),this.h.previous.next=l.entry,this.h.previous=l.entry,this.size++);return this};
e.prototype.delete=function(h){h=d(this,h);return h.entry&&h.list?(h.list.splice(h.index,1),h.list.length||delete this.data_[h.id],h.entry.previous.next=h.entry.next,h.entry.next.previous=h.entry.previous,h.entry.head=null,this.size--,!0):!1};
e.prototype.clear=function(){this.data_={};this.h=this.h.previous=b();this.size=0};
e.prototype.has=function(h){return!!d(this,h).entry};
e.prototype.get=function(h){return(h=d(this,h).entry)&&h.value};
e.prototype.entries=function(){return c(this,function(h){return[h.key,h.value]})};
e.prototype.keys=function(){return c(this,function(h){return h.key})};
e.prototype.values=function(){return c(this,function(h){return h.value})};
e.prototype.forEach=function(h,k){for(var l=this.entries(),m;!(m=l.next()).done;)m=m.value,h.call(k,m[1],m[0],this)};
e.prototype[Symbol.iterator]=e.prototype.entries;var g=0;return e});
function Ma(a,b,c){if(null==a)throw new TypeError("The 'this' value for String.prototype."+c+" must not be null or undefined");if(b instanceof RegExp)throw new TypeError("First argument to String.prototype."+c+" must not be a regular expression");return a+""}
t("String.prototype.endsWith",function(a){return a?a:function(b,c){var d=Ma(this,b,"endsWith");b+="";void 0===c&&(c=d.length);c=Math.max(0,Math.min(c|0,d.length));for(var e=b.length;0<e&&0<c;)if(d[--c]!=b[--e])return!1;return 0>=e}});
t("Array.prototype.find",function(a){return a?a:function(b,c){a:{var d=this;d instanceof String&&(d=String(d));for(var e=d.length,f=0;f<e;f++){var g=d[f];if(b.call(c,g,f,d)){b=g;break a}}b=void 0}return b}});
t("String.prototype.startsWith",function(a){return a?a:function(b,c){var d=Ma(this,b,"startsWith");b+="";var e=d.length,f=b.length;c=Math.max(0,Math.min(c|0,d.length));for(var g=0;g<f&&c<e;)if(d[c++]!=b[g++])return!1;return g>=f}});
t("Number.isFinite",function(a){return a?a:function(b){return"number"!==typeof b?!1:!isNaN(b)&&Infinity!==b&&-Infinity!==b}});
t("Number.isInteger",function(a){return a?a:function(b){return Number.isFinite(b)?b===Math.floor(b):!1}});
t("Number.MAX_SAFE_INTEGER",function(){return 9007199254740991});
t("Number.isNaN",function(a){return a?a:function(b){return"number"===typeof b&&isNaN(b)}});
function Na(a,b){a instanceof String&&(a+="");var c=0,d=!1,e={next:function(){if(!d&&c<a.length){var f=c++;return{value:b(f,a[f]),done:!1}}d=!0;return{done:!0,value:void 0}}};
e[Symbol.iterator]=function(){return e};
return e}
t("Array.prototype.entries",function(a){return a?a:function(){return Na(this,function(b,c){return[b,c]})}});
t("Array.from",function(a){return a?a:function(b,c,d){c=null!=c?c:function(h){return h};
var e=[],f="undefined"!=typeof Symbol&&Symbol.iterator&&b[Symbol.iterator];if("function"==typeof f){b=f.call(b);for(var g=0;!(f=b.next()).done;)e.push(c.call(d,f.value,g++))}else for(f=b.length,g=0;g<f;g++)e.push(c.call(d,b[g],g));return e}});
t("Array.prototype.keys",function(a){return a?a:function(){return Na(this,function(b){return b})}});
t("Array.prototype.values",function(a){return a?a:function(){return Na(this,function(b,c){return c})}});
t("Array.prototype.fill",function(a){return a?a:function(b,c,d){var e=this.length||0;0>c&&(c=Math.max(0,e+c));if(null==d||d>e)d=e;d=Number(d);0>d&&(d=Math.max(0,e+d));for(c=Number(c||0);c<d;c++)this[c]=b;return this}});
function Oa(a){return a?a:Array.prototype.fill}
t("Int8Array.prototype.fill",Oa);t("Uint8Array.prototype.fill",Oa);t("Uint8ClampedArray.prototype.fill",Oa);t("Int16Array.prototype.fill",Oa);t("Uint16Array.prototype.fill",Oa);t("Int32Array.prototype.fill",Oa);t("Uint32Array.prototype.fill",Oa);t("Float32Array.prototype.fill",Oa);t("Float64Array.prototype.fill",Oa);t("Object.setPrototypeOf",function(a){return a||sa});
t("Set",function(a){function b(c){this.h=new Map;if(c){c=u(c);for(var d;!(d=c.next()).done;)this.add(d.value)}this.size=this.h.size}
if(function(){if(!a||"function"!=typeof a||!a.prototype.entries||"function"!=typeof Object.seal)return!1;try{var c=Object.seal({x:4}),d=new a(u([c]));if(!d.has(c)||1!=d.size||d.add(c)!=d||1!=d.size||d.add({x:4})!=d||2!=d.size)return!1;var e=d.entries(),f=e.next();if(f.done||f.value[0]!=c||f.value[1]!=c)return!1;f=e.next();return f.done||f.value[0]==c||4!=f.value[0].x||f.value[1]!=f.value[0]?!1:e.next().done}catch(g){return!1}}())return a;
b.prototype.add=function(c){c=0===c?0:c;this.h.set(c,c);this.size=this.h.size;return this};
b.prototype.delete=function(c){c=this.h.delete(c);this.size=this.h.size;return c};
b.prototype.clear=function(){this.h.clear();this.size=0};
b.prototype.has=function(c){return this.h.has(c)};
b.prototype.entries=function(){return this.h.entries()};
b.prototype.values=function(){return this.h.values()};
b.prototype.keys=b.prototype.values;b.prototype[Symbol.iterator]=b.prototype.values;b.prototype.forEach=function(c,d){var e=this;this.h.forEach(function(f){return c.call(d,f,f,e)})};
return b});
t("Object.entries",function(a){return a?a:function(b){var c=[],d;for(d in b)la(b,d)&&c.push([d,b[d]]);return c}});
t("Object.is",function(a){return a?a:function(b,c){return b===c?0!==b||1/b===1/c:b!==b&&c!==c}});
t("Array.prototype.includes",function(a){return a?a:function(b,c){var d=this;d instanceof String&&(d=String(d));var e=d.length;c=c||0;for(0>c&&(c=Math.max(c+e,0));c<e;c++){var f=d[c];if(f===b||Object.is(f,b))return!0}return!1}});
t("String.prototype.includes",function(a){return a?a:function(b,c){return-1!==Ma(this,b,"includes").indexOf(b,c||0)}});
var z=this||self;function B(a,b,c){a=a.split(".");c=c||z;a[0]in c||"undefined"==typeof c.execScript||c.execScript("var "+a[0]);for(var d;a.length&&(d=a.shift());)a.length||void 0===b?c[d]&&c[d]!==Object.prototype[d]?c=c[d]:c=c[d]={}:c[d]=b}
function C(a,b){a=a.split(".");b=b||z;for(var c=0;c<a.length;c++)if(b=b[a[c]],null==b)return null;return b}
function Pa(a){var b=typeof a;return"object"!=b?b:a?Array.isArray(a)?"array":b:"null"}
function Qa(a){var b=Pa(a);return"array"==b||"object"==b&&"number"==typeof a.length}
function Ra(a){var b=typeof a;return"object"==b&&null!=a||"function"==b}
function Sa(a){return Object.prototype.hasOwnProperty.call(a,Ta)&&a[Ta]||(a[Ta]=++Ua)}
var Ta="closure_uid_"+(1E9*Math.random()>>>0),Ua=0;function Va(a,b,c){return a.call.apply(a.bind,arguments)}
function Ya(a,b,c){if(!a)throw Error();if(2<arguments.length){var d=Array.prototype.slice.call(arguments,2);return function(){var e=Array.prototype.slice.call(arguments);Array.prototype.unshift.apply(e,d);return a.apply(b,e)}}return function(){return a.apply(b,arguments)}}
function Za(a,b,c){Function.prototype.bind&&-1!=Function.prototype.bind.toString().indexOf("native code")?Za=Va:Za=Ya;return Za.apply(null,arguments)}
function D(a,b){function c(){}
c.prototype=b.prototype;a.pa=b.prototype;a.prototype=new c;a.prototype.constructor=a;a.Ld=function(d,e,f){for(var g=Array(arguments.length-2),h=2;h<arguments.length;h++)g[h-2]=arguments[h];return b.prototype[e].apply(d,g)}}
function $a(a){return a}
;function ab(a,b){if(Error.captureStackTrace)Error.captureStackTrace(this,ab);else{var c=Error().stack;c&&(this.stack=c)}a&&(this.message=String(a));void 0!==b&&(this.cause=b)}
D(ab,Error);ab.prototype.name="CustomError";function bb(a){a=a.url;var b=/[?&]dsh=1(&|$)/.test(a);this.j=!b&&/[?&]ae=1(&|$)/.test(a);this.m=!b&&/[?&]ae=2(&|$)/.test(a);if((this.h=/[?&]adurl=([^&]*)/.exec(a))&&this.h[1]){try{var c=decodeURIComponent(this.h[1])}catch(d){c=null}this.i=c}}
;function cb(){}
function db(a){var b=!1,c;return function(){b||(c=a(),b=!0);return c}}
;var eb=Array.prototype.indexOf?function(a,b){return Array.prototype.indexOf.call(a,b,void 0)}:function(a,b){if("string"===typeof a)return"string"!==typeof b||1!=b.length?-1:a.indexOf(b,0);
for(var c=0;c<a.length;c++)if(c in a&&a[c]===b)return c;return-1},E=Array.prototype.forEach?function(a,b,c){Array.prototype.forEach.call(a,b,c)}:function(a,b,c){for(var d=a.length,e="string"===typeof a?a.split(""):a,f=0;f<d;f++)f in e&&b.call(c,e[f],f,a)},fb=Array.prototype.map?function(a,b){return Array.prototype.map.call(a,b,void 0)}:function(a,b){for(var c=a.length,d=Array(c),e="string"===typeof a?a.split(""):a,f=0;f<c;f++)f in e&&(d[f]=b.call(void 0,e[f],f,a));
return d},gb=Array.prototype.reduce?function(a,b,c){return Array.prototype.reduce.call(a,b,c)}:function(a,b,c){var d=c;
E(a,function(e,f){d=b.call(void 0,d,e,f,a)});
return d};
function hb(a,b){b=eb(a,b);var c;(c=0<=b)&&Array.prototype.splice.call(a,b,1);return c}
function ib(a){return Array.prototype.concat.apply([],arguments)}
function jb(a){var b=a.length;if(0<b){for(var c=Array(b),d=0;d<b;d++)c[d]=a[d];return c}return[]}
function kb(a,b){for(var c=1;c<arguments.length;c++){var d=arguments[c];if(Qa(d)){var e=a.length||0,f=d.length||0;a.length=e+f;for(var g=0;g<f;g++)a[e+g]=d[g]}else a.push(d)}}
;function mb(a,b){for(var c in a)b.call(void 0,a[c],c,a)}
function nb(a){var b=ob,c;for(c in b)if(a.call(void 0,b[c],c,b))return c}
function pb(a,b){for(var c in a)if(!(c in b)||a[c]!==b[c])return!1;for(var d in b)if(!(d in a))return!1;return!0}
function qb(a){if(!a||"object"!==typeof a)return a;if("function"===typeof a.clone)return a.clone();if("undefined"!==typeof Map&&a instanceof Map)return new Map(a);if("undefined"!==typeof Set&&a instanceof Set)return new Set(a);if(a instanceof Date)return new Date(a.getTime());var b=Array.isArray(a)?[]:"function"!==typeof ArrayBuffer||"function"!==typeof ArrayBuffer.isView||!ArrayBuffer.isView(a)||a instanceof DataView?{}:new a.constructor(a.length),c;for(c in a)b[c]=qb(a[c]);return b}
var rb="constructor hasOwnProperty isPrototypeOf propertyIsEnumerable toLocaleString toString valueOf".split(" ");function sb(a,b){for(var c,d,e=1;e<arguments.length;e++){d=arguments[e];for(c in d)a[c]=d[c];for(var f=0;f<rb.length;f++)c=rb[f],Object.prototype.hasOwnProperty.call(d,c)&&(a[c]=d[c])}}
;var tb;function ub(){}
function vb(a){return new ub(wb,a)}
var wb={};vb("");var xb=String.prototype.trim?function(a){return a.trim()}:function(a){return/^[\s\xa0]*([\s\S]*?)[\s\xa0]*$/.exec(a)[1]},yb=/&/g,zb=/</g,Ab=/>/g,Bb=/"/g,Cb=/'/g,Db=/\x00/g,Jb=/[\x00&<>"']/;function Kb(a,b){this.h=b===Lb?a:""}
Kb.prototype.toString=function(){return this.h.toString()};
var Lb={},Mb=new Kb("about:invalid#zClosurez",Lb);function Nb(){var a=z.navigator;return a&&(a=a.userAgent)?a:""}
function F(a){return-1!=Nb().indexOf(a)}
;function Ob(){return(F("Chrome")||F("CriOS"))&&!F("Edge")||F("Silk")}
;var Pb={};function Qb(a){this.h=Pb===Pb?a:""}
Qb.prototype.toString=function(){return this.h.toString()};function Rb(a){Jb.test(a)&&(-1!=a.indexOf("&")&&(a=a.replace(yb,"&amp;")),-1!=a.indexOf("<")&&(a=a.replace(zb,"&lt;")),-1!=a.indexOf(">")&&(a=a.replace(Ab,"&gt;")),-1!=a.indexOf('"')&&(a=a.replace(Bb,"&quot;")),-1!=a.indexOf("'")&&(a=a.replace(Cb,"&#39;")),-1!=a.indexOf("\x00")&&(a=a.replace(Db,"&#0;")));return a}
;var Sb=RegExp("^(?:([^:/?#.]+):)?(?://(?:([^\\\\/?#]*)@)?([^\\\\/?#]*?)(?::([0-9]+))?(?=[\\\\/?#]|$))?([^?#]+)?(?:\\?([^#]*))?(?:#([\\s\\S]*))?$");function Tb(a){return a?decodeURI(a):a}
function Ub(a){return Tb(a.match(Sb)[3]||null)}
function Vb(a){var b=a.match(Sb);a=b[1];var c=b[2],d=b[3];b=b[4];var e="";a&&(e+=a+":");d&&(e+="//",c&&(e+=c+"@"),e+=d,b&&(e+=":"+b));return e}
function Wb(a,b,c){if(Array.isArray(b))for(var d=0;d<b.length;d++)Wb(a,String(b[d]),c);else null!=b&&c.push(a+(""===b?"":"="+encodeURIComponent(String(b))))}
function Xb(a){var b=[],c;for(c in a)Wb(c,a[c],b);return b.join("&")}
var Yb=/#|$/;function Zb(a,b){var c=a.search(Yb);a:{var d=0;for(var e=b.length;0<=(d=a.indexOf(b,d))&&d<c;){var f=a.charCodeAt(d-1);if(38==f||63==f)if(f=a.charCodeAt(d+e),!f||61==f||38==f||35==f)break a;d+=e+1}d=-1}if(0>d)return null;e=a.indexOf("&",d);if(0>e||e>c)e=c;d+=b.length+1;return decodeURIComponent(a.slice(d,-1!==e?e:0).replace(/\+/g," "))}
;function $b(a){z.setTimeout(function(){throw a;},0)}
;function ac(){return F("iPhone")&&!F("iPod")&&!F("iPad")}
;function bc(a){bc[" "](a);return a}
bc[" "]=function(){};var cc=F("Opera"),dc=F("Trident")||F("MSIE"),ec=F("Edge"),fc=F("Gecko")&&!(-1!=Nb().toLowerCase().indexOf("webkit")&&!F("Edge"))&&!(F("Trident")||F("MSIE"))&&!F("Edge"),gc=-1!=Nb().toLowerCase().indexOf("webkit")&&!F("Edge");function hc(){var a=z.document;return a?a.documentMode:void 0}
var ic;a:{var nc="",oc=function(){var a=Nb();if(fc)return/rv:([^\);]+)(\)|;)/.exec(a);if(ec)return/Edge\/([\d\.]+)/.exec(a);if(dc)return/\b(?:MSIE|rv)[: ]([^\);]+)(\)|;)/.exec(a);if(gc)return/WebKit\/(\S+)/.exec(a);if(cc)return/(?:Version)[ \/]?(\S+)/.exec(a)}();
oc&&(nc=oc?oc[1]:"");if(dc){var pc=hc();if(null!=pc&&pc>parseFloat(nc)){ic=String(pc);break a}}ic=nc}var qc=ic,rc;if(z.document&&dc){var sc=hc();rc=sc?sc:parseInt(qc,10)||void 0}else rc=void 0;var tc=rc;var uc=ac()||F("iPod"),vc=F("iPad");!F("Android")||Ob();Ob();var wc=F("Safari")&&!(Ob()||F("Coast")||F("Opera")||F("Edge")||F("Edg/")||F("OPR")||F("Firefox")||F("FxiOS")||F("Silk")||F("Android"))&&!(ac()||F("iPad")||F("iPod"));var xc={},yc=null;
function zc(a,b){Qa(a);void 0===b&&(b=0);if(!yc){yc={};for(var c="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789".split(""),d=["+/=","+/","-_=","-_.","-_"],e=0;5>e;e++){var f=c.concat(d[e].split(""));xc[e]=f;for(var g=0;g<f.length;g++){var h=f[g];void 0===yc[h]&&(yc[h]=g)}}}b=xc[b];c=Array(Math.floor(a.length/3));d=b[64]||"";for(e=f=0;f<a.length-2;f+=3){var k=a[f],l=a[f+1];h=a[f+2];g=b[k>>2];k=b[(k&3)<<4|l>>4];l=b[(l&15)<<2|h>>6];h=b[h&63];c[e++]=""+g+k+l+h}g=0;h=d;switch(a.length-
f){case 2:g=a[f+1],h=b[(g&15)<<2]||d;case 1:a=a[f],c[e]=""+b[a>>2]+b[(a&3)<<4|g>>4]+h+d}return c.join("")}
;var Ac="undefined"!==typeof Uint8Array,Bc=!dc&&"function"===typeof z.btoa;var Cc="function"===typeof Symbol&&"symbol"===typeof Symbol()?Symbol():void 0;function Dc(a,b){if(Cc)return a[Cc]|=b;if(void 0!==a.ma)return a.ma|=b;Object.defineProperties(a,{ma:{value:b,configurable:!0,writable:!0,enumerable:!1}});return b}
function Ec(a,b){Cc?a[Cc]&&(a[Cc]&=~b):void 0!==a.ma&&(a.ma&=~b)}
function G(a){var b;Cc?b=a[Cc]:b=a.ma;return null==b?0:b}
function Fc(a,b){Cc?a[Cc]=b:void 0!==a.ma?a.ma=b:Object.defineProperties(a,{ma:{value:b,configurable:!0,writable:!0,enumerable:!1}})}
function Gc(a){Dc(a,1);return a}
function Hc(a,b){Fc(b,(a|0)&-51)}
function Ic(a,b){Fc(b,(a|18)&-41)}
;var Jc={};function Kc(a){return null!==a&&"object"===typeof a&&!Array.isArray(a)&&a.constructor===Object}
var Lc,Nc,Oc=[];Fc(Oc,23);Nc=Object.freeze(Oc);function Pc(a){if(G(a.M)&2)throw Error();}
function Qc(a){var b=a.length;(b=b?a[b-1]:void 0)&&Kc(b)?b.g=1:(b={},a.push((b.g=1,b)))}
;function Rc(a){return a.displayName||a.name||"unknown type name"}
function Sc(a,b){if(!(a instanceof b))throw Error("Expected instanceof "+Rc(b)+" but got "+(a&&Rc(a.constructor)));return a}
function Tc(a,b){var c=G(a),d=c;0===d&&(d|=b&16);d|=b&2;d!==c&&Fc(a,d)}
;function Uc(a){var b=a.h+a.Ia;return a.ha||(a.ha=a.M[b]={})}
function Vc(a,b,c){return-1===b?null:b>=a.h?a.ha?a.ha[b]:void 0:c&&a.ha&&(c=a.ha[b],null!=c)?c:a.M[b+a.Ia]}
function H(a,b,c,d){Pc(a);return Wc(a,b,c,d)}
function Wc(a,b,c,d){a.i&&(a.i=void 0);if(b>=a.h||d)return Uc(a)[b]=c,a;a.M[b+a.Ia]=c;(c=a.ha)&&b in c&&delete c[b];return a}
function Xc(a,b,c,d,e){var f=Vc(a,b,d);Array.isArray(f)||(f=Nc);var g=G(f);g&1||Gc(f);if(e)g&2||Dc(f,18),c&1||Object.freeze(f);else{e=!(c&2);var h=g&2;c&1||!h?e&&g&16&&!h&&Ec(f,16):(f=Gc(Array.prototype.slice.call(f)),Wc(a,b,f,d))}return f}
function Yc(a,b,c,d){Pc(a);(c=Zc(a,c))&&c!==b&&null!=d&&Wc(a,c,void 0,!1);Wc(a,b,d)}
function Zc(a,b){for(var c=0,d=0;d<b.length;d++){var e=b[d];null!=Vc(a,e)&&(0!==c&&Wc(a,c,void 0,!1),c=e)}return c}
function $c(a,b,c){var d=void 0===d?!1:d;var e=Vc(a,c,d);var f=G(a.M),g=!1;null==e||"object"!==typeof e||(g=Array.isArray(e))||e.Qb!==Jc?g?(Tc(e,f),b=new b(e)):b=void 0:b=e;b!==e&&null!=b&&Wc(a,c,b,d);e=b;if(null==e)return e;G(a.M)&2||(b=e,G(b.M)&2&&(f=ad(b,!1),f.i=b,b=f),b!==e&&(e=b,Wc(a,c,e,d)));return e}
function J(a,b,c,d){Pc(a);null!=d?Sc(d,b):d=void 0;return Wc(a,c,d)}
function bd(a,b,c,d,e){Pc(a);null!=e?Sc(e,b):e=void 0;Yc(a,c,d,e)}
function cd(a,b,c,d,e){Pc(a);var f=null==d?Nc:Gc([]);if(null!=d){for(var g=!!d.length,h=0;h<d.length;h++){var k=d[h];Sc(k,b);g=g&&!(G(k.M)&2);f[h]=k.M}b=f;g=(g?8:0)|1;f=G(b);(f&g)!==g&&(Object.isFrozen(b)&&(b=Array.prototype.slice.call(b)),Fc(b,f|g));f=b;a.ca||(a.ca={});a.ca[c]=d}else a.ca&&(a.ca[c]=void 0);return Wc(a,c,f,e)}
function gd(a,b,c,d){var e=G(a.M);if(e&2)throw Error();var f=!!(e&2);a.ca||(a.ca={});var g=a.ca[b],h=Xc(a,b,3,void 0,f);if(g)!f&&Object.isFrozen(g)&&(g=Array.prototype.slice.call(g),a.ca[b]=g),f=g;else{var k=h;f=[];g=!!(e&2);var l=!!(G(k)&2);h=k;!g&&l&&(k=Array.prototype.slice.call(k));e|=l?2:0;for(var m=0;m<k.length;m++){var n=k[m];var r=c;Array.isArray(n)?(Tc(n,e),n=new r(n)):n=void 0;void 0!==n&&(l||(l=!!(2&G(n.M))),f.push(n))}a.ca[b]=f;l=!l;m=G(k);e=m|33;e=l?e|8:e&-9;m!=e&&(Object.isFrozen(k)&&
(k=Array.prototype.slice.call(k)),Fc(k,e));h!==k&&Wc(a,b,k);g&&Dc(f,18)}c=null!=d?Sc(d,c):new c;a=Xc(a,b,2,void 0,!1);f.push(c);a.push(c.M);G(c.M)&2&&Ec(a,8)}
function hd(a,b){a=Vc(a,b);return null==a?"":a}
function id(a,b){var c=Zc(a,jd)===b;return Vc(a,c?b:-1)}
;var kd;function ld(a){switch(typeof a){case "number":return isFinite(a)?a:String(a);case "object":if(a)if(Array.isArray(a)){if(0!==(G(a)&128))return a=Array.prototype.slice.call(a),Qc(a),a}else if(Ac&&null!=a&&a instanceof Uint8Array){if(Bc){for(var b="";10240<a.length;)b+=String.fromCharCode.apply(null,a.subarray(0,10240)),a=a.subarray(10240);b+=String.fromCharCode.apply(null,a);a=btoa(b)}else a=zc(a);return a}}return a}
;function md(a,b,c,d){if(null!=a){if(Array.isArray(a))a=nd(a,b,c,void 0!==d);else if(Kc(a)){var e={},f;for(f in a)e[f]=md(a[f],b,c,d);a=e}else a=b(a,d);return a}}
function nd(a,b,c,d){var e=G(a);d=d?!!(e&16):void 0;a=Array.prototype.slice.call(a);for(var f=0;f<a.length;f++)a[f]=md(a[f],b,c,d);c(e,a);return a}
function od(a){return a.Qb===Jc?a.toJSON():ld(a)}
function pd(a,b){a&128&&Qc(b)}
;function qd(a,b,c){c=void 0===c?Ic:c;if(null!=a){if(Ac&&a instanceof Uint8Array)return b?a:new Uint8Array(a);if(Array.isArray(a)){var d=G(a);if(d&2)return a;if(b&&!(d&32)&&(d&16||0===d))return Fc(a,d|18),a;a=nd(a,qd,d&4?Ic:c,!0);b=G(a);b&4&&b&2&&Object.freeze(a);return a}return a.Qb===Jc?rd(a):a}}
function sd(a,b,c,d,e,f,g){(a=a.ca&&a.ca[c])?(d=0<a.length?a[0].constructor:void 0,f=G(a),f&2||(a=fb(a,rd),Ic(f,a),Object.freeze(a)),cd(b,d,c,a,e)):H(b,c,qd(d,f,g),e)}
function rd(a){if(G(a.M)&2)return a;a=ad(a,!0);Dc(a.M,18);return a}
function ad(a,b){var c=a.M,d=[];Dc(d,16);var e=a.constructor.h;e&&d.push(e);e=a.ha;if(e){d.length=c.length;d.fill(void 0,d.length,c.length);var f={};d[d.length-1]=f}0!==(G(c)&128)&&Qc(d);b=b||G(a.M)&2?Ic:Hc;f=a.constructor;G(d);kd=d;d=new f(d);kd=void 0;a.oc&&(d.oc=a.oc.slice());f=!!(G(c)&16);for(var g=e?c.length-1:c.length,h=0;h<g;h++)sd(a,d,h-a.Ia,c[h],!1,f,b);if(e)for(var k in e)c=e[k],g=+k,Number.isNaN(g),sd(a,d,g,c,!0,f,b);return d}
;function K(a,b,c,d){null==a&&(a=kd);kd=void 0;var e=this.constructor.h;if(null==a){a=e?[e]:[];var f=48;var g=!0;d&&(f|=128);Fc(a,f)}else{if(!Array.isArray(a))throw Error();if(e&&e!==a[0])throw Error();f=Dc(a,0)|32;g=0!==(16&f);if(d){if(!(f&128)&&0<a.length){var h=a[a.length-1];if(Kc(h)&&"g"in h){f|=128;delete h.g;var k=!0,l;for(l in h){k=!1;break}k&&a.pop()}else throw Error();}}else if(128&f)throw Error();Fc(a,f)}this.Ia=e?0:-1;this.ca=void 0;this.M=a;a:{f=this.M.length;e=f-1;if(f&&(f=this.M[e],Kc(f))){this.ha=
f;this.h=e-this.Ia;break a}void 0!==b&&-1<b?(this.h=Math.max(b,e+1-this.Ia),this.ha=void 0):this.h=Number.MAX_VALUE}if(!d&&this.ha&&"g"in this.ha)throw Error('Unexpected "g" flag in sparse object of message that is not a group type.');if(c){b=g&&!0;d=this.h;var m;for(g=0;g<c.length;g++)e=c[g],e<d?(e+=this.Ia,(f=a[e])?td(f,b):a[e]=Nc):(m||(m=Uc(this)),(f=m[e])?td(f,b):m[e]=Nc)}G(this.M)}
K.prototype.toJSON=function(){var a=this.M,b;Lc?b=a:b=nd(a,od,pd);return b};
function ud(a){Lc=!0;try{return JSON.stringify(a.toJSON(),vd)}finally{Lc=!1}}
K.prototype.clone=function(){return ad(this,!1)};
function td(a,b){if(Array.isArray(a)){var c=G(a),d=1;!b||c&2||(d|=16);(c&d)!==d&&Fc(a,c|d)}}
K.prototype.Qb=Jc;K.prototype.toString=function(){return this.M.toString()};
function vd(a,b){return ld(b)}
;var wd=window;vb("csi.gstatic.com");vb("googleads.g.doubleclick.net");vb("partner.googleadservices.com");vb("pubads.g.doubleclick.net");vb("securepubads.g.doubleclick.net");vb("tpc.googlesyndication.com");function xd(a,b){this.x=void 0!==a?a:0;this.y=void 0!==b?b:0}
q=xd.prototype;q.clone=function(){return new xd(this.x,this.y)};
q.equals=function(a){return a instanceof xd&&(this==a?!0:this&&a?this.x==a.x&&this.y==a.y:!1)};
q.ceil=function(){this.x=Math.ceil(this.x);this.y=Math.ceil(this.y);return this};
q.floor=function(){this.x=Math.floor(this.x);this.y=Math.floor(this.y);return this};
q.round=function(){this.x=Math.round(this.x);this.y=Math.round(this.y);return this};function yd(a,b){this.width=a;this.height=b}
q=yd.prototype;q.clone=function(){return new yd(this.width,this.height)};
q.aspectRatio=function(){return this.width/this.height};
q.Lb=function(){return!(this.width*this.height)};
q.ceil=function(){this.width=Math.ceil(this.width);this.height=Math.ceil(this.height);return this};
q.floor=function(){this.width=Math.floor(this.width);this.height=Math.floor(this.height);return this};
q.round=function(){this.width=Math.round(this.width);this.height=Math.round(this.height);return this};function zd(){var a=document;var b="IFRAME";"application/xhtml+xml"===a.contentType&&(b=b.toLowerCase());return a.createElement(b)}
function Ad(a,b){for(var c=0;a;){if(b(a))return a;a=a.parentNode;c++}return null}
;function Bd(a){var b=C("window.location.href");null==a&&(a='Unknown Error of type "null/undefined"');if("string"===typeof a)return{message:a,name:"Unknown error",lineNumber:"Not available",fileName:b,stack:"Not available"};var c=!1;try{var d=a.lineNumber||a.line||"Not available"}catch(g){d="Not available",c=!0}try{var e=a.fileName||a.filename||a.sourceURL||z.$googDebugFname||b}catch(g){e="Not available",c=!0}b=Cd(a);if(!(!c&&a.lineNumber&&a.fileName&&a.stack&&a.message&&a.name)){c=a.message;if(null==
c){if(a.constructor&&a.constructor instanceof Function){if(a.constructor.name)c=a.constructor.name;else if(c=a.constructor,Dd[c])c=Dd[c];else{c=String(c);if(!Dd[c]){var f=/function\s+([^\(]+)/m.exec(c);Dd[c]=f?f[1]:"[Anonymous]"}c=Dd[c]}c='Unknown Error of type "'+c+'"'}else c="Unknown Error of unknown type";"function"===typeof a.toString&&Object.prototype.toString!==a.toString&&(c+=": "+a.toString())}return{message:c,name:a.name||"UnknownError",lineNumber:d,fileName:e,stack:b||"Not available"}}a.stack=
b;return{message:a.message,name:a.name,lineNumber:a.lineNumber,fileName:a.fileName,stack:a.stack}}
function Cd(a,b){b||(b={});b[Ed(a)]=!0;var c=a.stack||"";(a=a.cause)&&!b[Ed(a)]&&(c+="\nCaused by: ",a.stack&&0==a.stack.indexOf(a.toString())||(c+="string"===typeof a?a:a.message+"\n"),c+=Cd(a,b));return c}
function Ed(a){var b="";"function"===typeof a.toString&&(b=""+a);return b+a.stack}
var Dd={};/*

 SPDX-License-Identifier: Apache-2.0
*/
var Fd;try{new URL("s://g"),Fd=!0}catch(a){Fd=!1}var Gd=Fd;function Hd(a,b){a.removeAttribute("srcdoc");if(b instanceof Kb)b instanceof Kb&&b.constructor===Kb?b=b.h:(Pa(b),b="type_error:SafeUrl");else{b:if(Gd){try{var c=new URL(b)}catch(d){c="https:";break b}c=c.protocol}else c:{c=document.createElement("a");try{c.href=b}catch(d){c=void 0;break c}c=c.protocol;c=":"===c||""===c?"https:":c}b="javascript:"!==c?b:void 0}void 0!==b&&(a.src=b);for(b="allow-same-origin allow-scripts allow-forms allow-popups allow-popups-to-escape-sandbox allow-storage-access-by-user-activation".split(" ");0<
a.sandbox.length;)a.sandbox.remove(a.sandbox.item(0));for(c=0;c<b.length;c++)a.sandbox.supports&&!a.sandbox.supports(b[c])||a.sandbox.add(b[c])}
;function Id(a){this.cd=a}
function Jd(a){return new Id(function(b){return b.substr(0,a.length+1).toLowerCase()===a+":"})}
var Kd=[Jd("data"),Jd("http"),Jd("https"),Jd("mailto"),Jd("ftp"),new Id(function(a){return/^[^:]*([/?#]|$)/.test(a)})];
function Ld(a,b){b=void 0===b?Kd:b;for(var c=0;c<b.length;++c){var d=b[c];if(d instanceof Id&&d.cd(a))return new Kb(a,Lb)}}
function Md(a){var b=void 0===b?Kd:b;return Ld(a,b)||Mb}
;function Nd(a){var b=Od;if(b)for(var c in b)Object.prototype.hasOwnProperty.call(b,c)&&a(b[c],c,b)}
function Pd(){var a=[];Nd(function(b){a.push(b)});
return a}
var Od={sd:"allow-forms",td:"allow-modals",ud:"allow-orientation-lock",vd:"allow-pointer-lock",wd:"allow-popups",xd:"allow-popups-to-escape-sandbox",yd:"allow-presentation",zd:"allow-same-origin",Ad:"allow-scripts",Bd:"allow-top-navigation",Cd:"allow-top-navigation-by-user-activation"},Qd=db(function(){return Pd()});
function Rd(){var a=Sd(),b={};E(Qd(),function(c){a.sandbox&&a.sandbox.supports&&a.sandbox.supports(c)&&(b[c]=!0)});
return b}
function Sd(){var a=void 0===a?document:a;return a.createElement("iframe")}
;var Td=(new Date).getTime();function Ud(a){if(!a)return"";if(/^about:(?:blank|srcdoc)$/.test(a))return window.origin||"";a=a.split("#")[0].split("?")[0];a=a.toLowerCase();0==a.indexOf("//")&&(a=window.location.protocol+a);/^[\w\-]*:\/\//.test(a)||(a=window.location.href);var b=a.substring(a.indexOf("://")+3),c=b.indexOf("/");-1!=c&&(b=b.substring(0,c));c=a.substring(0,a.indexOf("://"));if(!c)throw Error("URI is missing protocol: "+a);if("http"!==c&&"https"!==c&&"chrome-extension"!==c&&"moz-extension"!==c&&"file"!==c&&"android-app"!==
c&&"chrome-search"!==c&&"chrome-untrusted"!==c&&"chrome"!==c&&"app"!==c&&"devtools"!==c)throw Error("Invalid URI scheme in origin: "+c);a="";var d=b.indexOf(":");if(-1!=d){var e=b.substring(d+1);b=b.substring(0,d);if("http"===c&&"80"!==e||"https"===c&&"443"!==e)a=":"+e}return c+"://"+b+a}
;var Vd="client_dev_domain client_dev_regex_map client_dev_root_url client_rollout_override expflag forcedCapability jsfeat jsmode mods".split(" ");ka(Vd);function Wd(){function a(){e[0]=1732584193;e[1]=4023233417;e[2]=2562383102;e[3]=271733878;e[4]=3285377520;m=l=0}
function b(n){for(var r=g,p=0;64>p;p+=4)r[p/4]=n[p]<<24|n[p+1]<<16|n[p+2]<<8|n[p+3];for(p=16;80>p;p++)n=r[p-3]^r[p-8]^r[p-14]^r[p-16],r[p]=(n<<1|n>>>31)&4294967295;n=e[0];var y=e[1],A=e[2],I=e[3],N=e[4];for(p=0;80>p;p++){if(40>p)if(20>p){var T=I^y&(A^I);var P=1518500249}else T=y^A^I,P=1859775393;else 60>p?(T=y&A|I&(y|A),P=2400959708):(T=y^A^I,P=3395469782);T=((n<<5|n>>>27)&4294967295)+T+N+P+r[p]&4294967295;N=I;I=A;A=(y<<30|y>>>2)&4294967295;y=n;n=T}e[0]=e[0]+n&4294967295;e[1]=e[1]+y&4294967295;e[2]=
e[2]+A&4294967295;e[3]=e[3]+I&4294967295;e[4]=e[4]+N&4294967295}
function c(n,r){if("string"===typeof n){n=unescape(encodeURIComponent(n));for(var p=[],y=0,A=n.length;y<A;++y)p.push(n.charCodeAt(y));n=p}r||(r=n.length);p=0;if(0==l)for(;p+64<r;)b(n.slice(p,p+64)),p+=64,m+=64;for(;p<r;)if(f[l++]=n[p++],m++,64==l)for(l=0,b(f);p+64<r;)b(n.slice(p,p+64)),p+=64,m+=64}
function d(){var n=[],r=8*m;56>l?c(h,56-l):c(h,64-(l-56));for(var p=63;56<=p;p--)f[p]=r&255,r>>>=8;b(f);for(p=r=0;5>p;p++)for(var y=24;0<=y;y-=8)n[r++]=e[p]>>y&255;return n}
for(var e=[],f=[],g=[],h=[128],k=1;64>k;++k)h[k]=0;var l,m;a();return{reset:a,update:c,digest:d,Mc:function(){for(var n=d(),r="",p=0;p<n.length;p++)r+="0123456789ABCDEF".charAt(Math.floor(n[p]/16))+"0123456789ABCDEF".charAt(n[p]%16);return r}}}
;function Xd(a,b,c){var d=String(z.location.href);return d&&a&&b?[b,Yd(Ud(d),a,c||null)].join(" "):null}
function Yd(a,b,c){var d=[],e=[];if(1==(Array.isArray(c)?2:1))return e=[b,a],E(d,function(h){e.push(h)}),Zd(e.join(" "));
var f=[],g=[];E(c,function(h){g.push(h.key);f.push(h.value)});
c=Math.floor((new Date).getTime()/1E3);e=0==f.length?[c,b,a]:[f.join(":"),c,b,a];E(d,function(h){e.push(h)});
a=Zd(e.join(" "));a=[c,a];0==g.length||a.push(g.join(""));return a.join("_")}
function Zd(a){var b=Wd();b.update(a);return b.Mc().toLowerCase()}
;var $d={};function ae(a){this.h=a||{cookie:""}}
q=ae.prototype;q.isEnabled=function(){if(!z.navigator.cookieEnabled)return!1;if(!this.Lb())return!0;this.set("TESTCOOKIESENABLED","1",{Nb:60});if("1"!==this.get("TESTCOOKIESENABLED"))return!1;this.remove("TESTCOOKIESENABLED");return!0};
q.set=function(a,b,c){var d=!1;if("object"===typeof c){var e=c.Td;d=c.secure||!1;var f=c.domain||void 0;var g=c.path||void 0;var h=c.Nb}if(/[;=\s]/.test(a))throw Error('Invalid cookie name "'+a+'"');if(/[;\r\n]/.test(b))throw Error('Invalid cookie value "'+b+'"');void 0===h&&(h=-1);c=f?";domain="+f:"";g=g?";path="+g:"";d=d?";secure":"";h=0>h?"":0==h?";expires="+(new Date(1970,1,1)).toUTCString():";expires="+(new Date(Date.now()+1E3*h)).toUTCString();this.h.cookie=a+"="+b+c+g+h+d+(null!=e?";samesite="+
e:"")};
q.get=function(a,b){for(var c=a+"=",d=(this.h.cookie||"").split(";"),e=0,f;e<d.length;e++){f=xb(d[e]);if(0==f.lastIndexOf(c,0))return f.slice(c.length);if(f==a)return""}return b};
q.remove=function(a,b,c){var d=void 0!==this.get(a);this.set(a,"",{Nb:0,path:b,domain:c});return d};
q.Lb=function(){return!this.h.cookie};
q.clear=function(){for(var a=(this.h.cookie||"").split(";"),b=[],c=[],d,e,f=0;f<a.length;f++)e=xb(a[f]),d=e.indexOf("="),-1==d?(b.push(""),c.push(e)):(b.push(e.substring(0,d)),c.push(e.substring(d+1)));for(a=b.length-1;0<=a;a--)this.remove(b[a])};
var be=new ae("undefined"==typeof document?null:document);function ce(a){return!!$d.FPA_SAMESITE_PHASE2_MOD||!(void 0===a||!a)}
function de(a,b,c,d){(a=z[a])||(a=(new ae(document)).get(b));return a?Xd(a,c,d):null}
function ee(a){var b=void 0===b?!1:b;var c=Ud(String(z.location.href)),d=[];var e=b;e=void 0===e?!1:e;var f=z.__SAPISID||z.__APISID||z.__3PSAPISID||z.__OVERRIDE_SID;ce(e)&&(f=f||z.__1PSAPISID);if(f)e=!0;else{var g=new ae(document);f=g.get("SAPISID")||g.get("APISID")||g.get("__Secure-3PAPISID")||g.get("SID")||g.get("OSID");ce(e)&&(f=f||g.get("__Secure-1PAPISID"));e=!!f}e&&(e=(c=0==c.indexOf("https:")||0==c.indexOf("chrome-extension:")||0==c.indexOf("moz-extension:"))?z.__SAPISID:z.__APISID,e||(e=new ae(document),
e=e.get(c?"SAPISID":"APISID")||e.get("__Secure-3PAPISID")),(e=e?Xd(e,c?"SAPISIDHASH":"APISIDHASH",a):null)&&d.push(e),c&&ce(b)&&((b=de("__1PSAPISID","__Secure-1PAPISID","SAPISID1PHASH",a))&&d.push(b),(a=de("__3PSAPISID","__Secure-3PAPISID","SAPISID3PHASH",a))&&d.push(a)));return 0==d.length?null:d.join(" ")}
;"undefined"!==typeof TextDecoder&&new TextDecoder;var fe="undefined"!==typeof TextEncoder?new TextEncoder:null,ge=fe?function(a){return fe.encode(a)}:function(a){for(var b=[],c=0,d=0;d<a.length;d++){var e=a.charCodeAt(d);
128>e?b[c++]=e:(2048>e?b[c++]=e>>6|192:(55296==(e&64512)&&d+1<a.length&&56320==(a.charCodeAt(d+1)&64512)?(e=65536+((e&1023)<<10)+(a.charCodeAt(++d)&1023),b[c++]=e>>18|240,b[c++]=e>>12&63|128):b[c++]=e>>12|224,b[c++]=e>>6&63|128),b[c++]=e&63|128)}a=new Uint8Array(b.length);for(c=0;c<a.length;c++)a[c]=b[c];return a};function he(){this.j=this.j;this.v=this.v}
he.prototype.j=!1;he.prototype.dispose=function(){this.j||(this.j=!0,this.Sa())};
he.prototype.Sa=function(){if(this.v)for(;this.v.length;)this.v.shift()()};function ie(a,b){this.type=a;this.h=this.target=b;this.defaultPrevented=this.j=!1}
ie.prototype.stopPropagation=function(){this.j=!0};
ie.prototype.preventDefault=function(){this.defaultPrevented=!0};var je=function(){if(!z.addEventListener||!Object.defineProperty)return!1;var a=!1,b=Object.defineProperty({},"passive",{get:function(){a=!0}});
try{z.addEventListener("test",function(){},b),z.removeEventListener("test",function(){},b)}catch(c){}return a}();function ke(a,b){ie.call(this,a?a.type:"");this.relatedTarget=this.h=this.target=null;this.button=this.screenY=this.screenX=this.clientY=this.clientX=0;this.key="";this.charCode=this.keyCode=0;this.metaKey=this.shiftKey=this.altKey=this.ctrlKey=!1;this.state=null;this.pointerId=0;this.pointerType="";this.i=null;a&&this.init(a,b)}
D(ke,ie);var le={2:"touch",3:"pen",4:"mouse"};
ke.prototype.init=function(a,b){var c=this.type=a.type,d=a.changedTouches&&a.changedTouches.length?a.changedTouches[0]:null;this.target=a.target||a.srcElement;this.h=b;if(b=a.relatedTarget){if(fc){a:{try{bc(b.nodeName);var e=!0;break a}catch(f){}e=!1}e||(b=null)}}else"mouseover"==c?b=a.fromElement:"mouseout"==c&&(b=a.toElement);this.relatedTarget=b;d?(this.clientX=void 0!==d.clientX?d.clientX:d.pageX,this.clientY=void 0!==d.clientY?d.clientY:d.pageY,this.screenX=d.screenX||0,this.screenY=d.screenY||
0):(this.clientX=void 0!==a.clientX?a.clientX:a.pageX,this.clientY=void 0!==a.clientY?a.clientY:a.pageY,this.screenX=a.screenX||0,this.screenY=a.screenY||0);this.button=a.button;this.keyCode=a.keyCode||0;this.key=a.key||"";this.charCode=a.charCode||("keypress"==c?a.keyCode:0);this.ctrlKey=a.ctrlKey;this.altKey=a.altKey;this.shiftKey=a.shiftKey;this.metaKey=a.metaKey;this.pointerId=a.pointerId||0;this.pointerType="string"===typeof a.pointerType?a.pointerType:le[a.pointerType]||"";this.state=a.state;
this.i=a;a.defaultPrevented&&ke.pa.preventDefault.call(this)};
ke.prototype.stopPropagation=function(){ke.pa.stopPropagation.call(this);this.i.stopPropagation?this.i.stopPropagation():this.i.cancelBubble=!0};
ke.prototype.preventDefault=function(){ke.pa.preventDefault.call(this);var a=this.i;a.preventDefault?a.preventDefault():a.returnValue=!1};var me="closure_listenable_"+(1E6*Math.random()|0);var pe=0;function qe(a,b,c,d,e){this.listener=a;this.proxy=null;this.src=b;this.type=c;this.capture=!!d;this.xb=e;this.key=++pe;this.lb=this.sb=!1}
function re(a){a.lb=!0;a.listener=null;a.proxy=null;a.src=null;a.xb=null}
;function se(a){this.src=a;this.listeners={};this.h=0}
se.prototype.add=function(a,b,c,d,e){var f=a.toString();a=this.listeners[f];a||(a=this.listeners[f]=[],this.h++);var g=te(a,b,d,e);-1<g?(b=a[g],c||(b.sb=!1)):(b=new qe(b,this.src,f,!!d,e),b.sb=c,a.push(b));return b};
se.prototype.remove=function(a,b,c,d){a=a.toString();if(!(a in this.listeners))return!1;var e=this.listeners[a];b=te(e,b,c,d);return-1<b?(re(e[b]),Array.prototype.splice.call(e,b,1),0==e.length&&(delete this.listeners[a],this.h--),!0):!1};
function ue(a,b){var c=b.type;c in a.listeners&&hb(a.listeners[c],b)&&(re(b),0==a.listeners[c].length&&(delete a.listeners[c],a.h--))}
function te(a,b,c,d){for(var e=0;e<a.length;++e){var f=a[e];if(!f.lb&&f.listener==b&&f.capture==!!c&&f.xb==d)return e}return-1}
;var ve="closure_lm_"+(1E6*Math.random()|0),we={},xe=0;function ye(a,b,c,d,e){if(d&&d.once)ze(a,b,c,d,e);else if(Array.isArray(b))for(var f=0;f<b.length;f++)ye(a,b[f],c,d,e);else c=Ae(c),a&&a[me]?a.Ka(b,c,Ra(d)?!!d.capture:!!d,e):Be(a,b,c,!1,d,e)}
function Be(a,b,c,d,e,f){if(!b)throw Error("Invalid event type");var g=Ra(e)?!!e.capture:!!e,h=Ce(a);h||(a[ve]=h=new se(a));c=h.add(b,c,d,g,f);if(!c.proxy){d=De();c.proxy=d;d.src=a;d.listener=c;if(a.addEventListener)je||(e=g),void 0===e&&(e=!1),a.addEventListener(b.toString(),d,e);else if(a.attachEvent)a.attachEvent(Ee(b.toString()),d);else if(a.addListener&&a.removeListener)a.addListener(d);else throw Error("addEventListener and attachEvent are unavailable.");xe++}}
function De(){function a(c){return b.call(a.src,a.listener,c)}
var b=Fe;return a}
function ze(a,b,c,d,e){if(Array.isArray(b))for(var f=0;f<b.length;f++)ze(a,b[f],c,d,e);else c=Ae(c),a&&a[me]?a.h.add(String(b),c,!0,Ra(d)?!!d.capture:!!d,e):Be(a,b,c,!0,d,e)}
function Ge(a,b,c,d,e){if(Array.isArray(b))for(var f=0;f<b.length;f++)Ge(a,b[f],c,d,e);else(d=Ra(d)?!!d.capture:!!d,c=Ae(c),a&&a[me])?a.h.remove(String(b),c,d,e):a&&(a=Ce(a))&&(b=a.listeners[b.toString()],a=-1,b&&(a=te(b,c,d,e)),(c=-1<a?b[a]:null)&&He(c))}
function He(a){if("number"!==typeof a&&a&&!a.lb){var b=a.src;if(b&&b[me])ue(b.h,a);else{var c=a.type,d=a.proxy;b.removeEventListener?b.removeEventListener(c,d,a.capture):b.detachEvent?b.detachEvent(Ee(c),d):b.addListener&&b.removeListener&&b.removeListener(d);xe--;(c=Ce(b))?(ue(c,a),0==c.h&&(c.src=null,b[ve]=null)):re(a)}}}
function Ee(a){return a in we?we[a]:we[a]="on"+a}
function Fe(a,b){if(a.lb)a=!0;else{b=new ke(b,this);var c=a.listener,d=a.xb||a.src;a.sb&&He(a);a=c.call(d,b)}return a}
function Ce(a){a=a[ve];return a instanceof se?a:null}
var Ie="__closure_events_fn_"+(1E9*Math.random()>>>0);function Ae(a){if("function"===typeof a)return a;a[Ie]||(a[Ie]=function(b){return a.handleEvent(b)});
return a[Ie]}
;function Je(){he.call(this);this.h=new se(this);this.Ga=this;this.N=null}
D(Je,he);Je.prototype[me]=!0;Je.prototype.addEventListener=function(a,b,c,d){ye(this,a,b,c,d)};
Je.prototype.removeEventListener=function(a,b,c,d){Ge(this,a,b,c,d)};
function Ke(a,b){var c=a.N;if(c){var d=[];for(var e=1;c;c=c.N)d.push(c),++e}a=a.Ga;c=b.type||b;"string"===typeof b?b=new ie(b,a):b instanceof ie?b.target=b.target||a:(e=b,b=new ie(c,a),sb(b,e));e=!0;if(d)for(var f=d.length-1;!b.j&&0<=f;f--){var g=b.h=d[f];e=Le(g,c,!0,b)&&e}b.j||(g=b.h=a,e=Le(g,c,!0,b)&&e,b.j||(e=Le(g,c,!1,b)&&e));if(d)for(f=0;!b.j&&f<d.length;f++)g=b.h=d[f],e=Le(g,c,!1,b)&&e}
Je.prototype.Sa=function(){Je.pa.Sa.call(this);if(this.h){var a=this.h,b=0,c;for(c in a.listeners){for(var d=a.listeners[c],e=0;e<d.length;e++)++b,re(d[e]);delete a.listeners[c];a.h--}}this.N=null};
Je.prototype.Ka=function(a,b,c,d){return this.h.add(String(a),b,!1,c,d)};
function Le(a,b,c,d){b=a.h.listeners[String(b)];if(!b)return!0;b=b.concat();for(var e=!0,f=0;f<b.length;++f){var g=b[f];if(g&&!g.lb&&g.capture==c){var h=g.listener,k=g.xb||g.src;g.sb&&ue(a.h,g);e=!1!==h.call(k,d)&&e}}return e&&!d.defaultPrevented}
;function Me(a){Je.call(this);var b=this;this.X=this.m=0;this.ia=null!=a?a:{ka:function(e,f){return setTimeout(e,f)},
Qa:function(e){clearTimeout(e)}};
var c,d;this.i=null!=(d=null==(c=window.navigator)?void 0:c.onLine)?d:!0;this.s=function(){return x(function(e){return w(e,Ne(b),0)})};
window.addEventListener("offline",this.s);window.addEventListener("online",this.s);this.X||Oe(this)}
v(Me,Je);function Pe(){var a=Qe;Me.h||(Me.h=new Me(a));return Me.h}
Me.prototype.dispose=function(){window.removeEventListener("offline",this.s);window.removeEventListener("online",this.s);this.ia.Qa(this.X);delete Me.h};
Me.prototype.da=function(){return this.i};
function Oe(a){a.X=a.ia.ka(function(){var b;return x(function(c){if(1==c.h)return a.i?(null==(b=window.navigator)?0:b.onLine)?c.D(3):w(c,Ne(a),3):w(c,Ne(a),3);Oe(a);c.h=0})},3E4)}
function Ne(a,b){return a.H?a.H:a.H=new Promise(function(c){var d,e,f,g;return x(function(h){switch(h.h){case 1:return d=window.AbortController?new window.AbortController:void 0,f=null==(e=d)?void 0:e.signal,g=!1,ya(h,2,3),d&&(a.m=a.ia.ka(function(){d.abort()},b||2E4)),w(h,fetch("/generate_204",{method:"HEAD",
signal:f}),5);case 5:g=!0;case 3:Ca(h);a.H=void 0;a.m&&(a.ia.Qa(a.m),a.m=0);g!==a.i&&(a.i=g,a.i?Ke(a,"networkstatus-online"):Ke(a,"networkstatus-offline"));c(g);Da(h);break;case 2:Ba(h),g=!1,h.D(3)}})})}
;function Re(){this.data_=[];this.h=-1}
Re.prototype.set=function(a,b){b=void 0===b?!0:b;0<=a&&52>a&&Number.isInteger(a)&&this.data_[a]!==b&&(this.data_[a]=b,this.h=-1)};
Re.prototype.get=function(a){return!!this.data_[a]};
function Se(a){-1===a.h&&(a.h=gb(a.data_,function(b,c,d){return c?b+Math.pow(2,d):b},0));
return a.h}
;function Te(a){K.call(this,a)}
v(Te,K);function Ue(a){K.call(this,a,-1,Ve)}
v(Ue,K);function We(a,b){return H(a,2,b)}
function Xe(a,b){return H(a,3,b)}
function Ye(a,b){return H(a,4,b)}
function Ze(a,b){return H(a,5,b)}
function $e(a,b){return H(a,9,b)}
function af(a,b){return cd(a,Te,10,b)}
function bf(a,b){return H(a,11,b)}
function cf(a,b){return H(a,1,b)}
function df(a,b){return H(a,7,b)}
var Ve=[10,6];var ef="platform platformVersion architecture model uaFullVersion bitness fullVersionList wow64".split(" ");function ff(a){var b;return null!=(b=a.google_tag_data)?b:a.google_tag_data={}}
function gf(a){var b,c;return"function"===typeof(null==(b=a.navigator)?void 0:null==(c=b.userAgentData)?void 0:c.getHighEntropyValues)}
function hf(){var a=window;if(!gf(a))return null;var b=ff(a);if(b.uach_promise)return b.uach_promise;a=a.navigator.userAgentData.getHighEntropyValues(ef).then(function(c){null!=b.uach||(b.uach=c);return c});
return b.uach_promise=a}
function jf(a){var b;return bf(af(Ze(We(cf(Ye(df($e(Xe(new Ue,a.architecture||""),a.bitness||""),a.mobile||!1),a.model||""),a.platform||""),a.platformVersion||""),a.uaFullVersion||""),(null==(b=a.fullVersionList)?void 0:b.map(function(c){var d=new Te;d=H(d,1,c.brand);return H(d,2,c.version)}))||[]),a.wow64||!1)}
function kf(){var a,b;return null!=(b=null==(a=hf())?void 0:a.then(function(c){return jf(c)}))?b:null}
;function lf(a,b){this.j=a;this.m=b;this.i=0;this.h=null}
lf.prototype.get=function(){if(0<this.i){this.i--;var a=this.h;this.h=a.next;a.next=null}else a=this.j();return a};
function mf(a,b){a.m(b);100>a.i&&(a.i++,b.next=a.h,a.h=b)}
;var nf;function of(){var a=z.MessageChannel;"undefined"===typeof a&&"undefined"!==typeof window&&window.postMessage&&window.addEventListener&&!F("Presto")&&(a=function(){var e=zd();e.style.display="none";document.documentElement.appendChild(e);var f=e.contentWindow;e=f.document;e.open();e.close();var g="callImmediate"+Math.random(),h="file:"==f.location.protocol?"*":f.location.protocol+"//"+f.location.host;e=Za(function(k){if(("*"==h||k.origin==h)&&k.data==g)this.port1.onmessage()},this);
f.addEventListener("message",e,!1);this.port1={};this.port2={postMessage:function(){f.postMessage(g,h)}}});
if("undefined"!==typeof a&&!F("Trident")&&!F("MSIE")){var b=new a,c={},d=c;b.port1.onmessage=function(){if(void 0!==c.next){c=c.next;var e=c.ac;c.ac=null;e()}};
return function(e){d.next={ac:e};d=d.next;b.port2.postMessage(0)}}return function(e){z.setTimeout(e,0)}}
;function pf(){this.i=this.h=null}
pf.prototype.add=function(a,b){var c=qf.get();c.set(a,b);this.i?this.i.next=c:this.h=c;this.i=c};
pf.prototype.remove=function(){var a=null;this.h&&(a=this.h,this.h=this.h.next,this.h||(this.i=null),a.next=null);return a};
var qf=new lf(function(){return new rf},function(a){return a.reset()});
function rf(){this.next=this.scope=this.h=null}
rf.prototype.set=function(a,b){this.h=a;this.scope=b;this.next=null};
rf.prototype.reset=function(){this.next=this.scope=this.h=null};var sf,tf=!1,uf=new pf;function vf(a,b){sf||wf();tf||(sf(),tf=!0);uf.add(a,b)}
function wf(){if(z.Promise&&z.Promise.resolve){var a=z.Promise.resolve(void 0);sf=function(){a.then(xf)}}else sf=function(){var b=xf;
"function"!==typeof z.setImmediate||z.Window&&z.Window.prototype&&!F("Edge")&&z.Window.prototype.setImmediate==z.setImmediate?(nf||(nf=of()),nf(b)):z.setImmediate(b)}}
function xf(){for(var a;a=uf.remove();){try{a.h.call(a.scope)}catch(b){$b(b)}mf(qf,a)}tf=!1}
;function yf(a,b){this.h=a[z.Symbol.iterator]();this.i=b}
yf.prototype[Symbol.iterator]=function(){return this};
yf.prototype.next=function(){var a=this.h.next();return{value:a.done?void 0:this.i.call(void 0,a.value),done:a.done}};
function zf(a,b){return new yf(a,b)}
;function Af(){this.blockSize=-1}
;function Bf(){this.blockSize=-1;this.blockSize=64;this.h=[];this.v=[];this.s=[];this.j=[];this.j[0]=128;for(var a=1;a<this.blockSize;++a)this.j[a]=0;this.m=this.i=0;this.reset()}
D(Bf,Af);Bf.prototype.reset=function(){this.h[0]=1732584193;this.h[1]=4023233417;this.h[2]=2562383102;this.h[3]=271733878;this.h[4]=3285377520;this.m=this.i=0};
function Cf(a,b,c){c||(c=0);var d=a.s;if("string"===typeof b)for(var e=0;16>e;e++)d[e]=b.charCodeAt(c)<<24|b.charCodeAt(c+1)<<16|b.charCodeAt(c+2)<<8|b.charCodeAt(c+3),c+=4;else for(e=0;16>e;e++)d[e]=b[c]<<24|b[c+1]<<16|b[c+2]<<8|b[c+3],c+=4;for(e=16;80>e;e++){var f=d[e-3]^d[e-8]^d[e-14]^d[e-16];d[e]=(f<<1|f>>>31)&4294967295}b=a.h[0];c=a.h[1];var g=a.h[2],h=a.h[3],k=a.h[4];for(e=0;80>e;e++){if(40>e)if(20>e){f=h^c&(g^h);var l=1518500249}else f=c^g^h,l=1859775393;else 60>e?(f=c&g|h&(c|g),l=2400959708):
(f=c^g^h,l=3395469782);f=(b<<5|b>>>27)+f+k+l+d[e]&4294967295;k=h;h=g;g=(c<<30|c>>>2)&4294967295;c=b;b=f}a.h[0]=a.h[0]+b&4294967295;a.h[1]=a.h[1]+c&4294967295;a.h[2]=a.h[2]+g&4294967295;a.h[3]=a.h[3]+h&4294967295;a.h[4]=a.h[4]+k&4294967295}
Bf.prototype.update=function(a,b){if(null!=a){void 0===b&&(b=a.length);for(var c=b-this.blockSize,d=0,e=this.v,f=this.i;d<b;){if(0==f)for(;d<=c;)Cf(this,a,d),d+=this.blockSize;if("string"===typeof a)for(;d<b;){if(e[f]=a.charCodeAt(d),++f,++d,f==this.blockSize){Cf(this,e);f=0;break}}else for(;d<b;)if(e[f]=a[d],++f,++d,f==this.blockSize){Cf(this,e);f=0;break}}this.i=f;this.m+=b}};
Bf.prototype.digest=function(){var a=[],b=8*this.m;56>this.i?this.update(this.j,56-this.i):this.update(this.j,this.blockSize-(this.i-56));for(var c=this.blockSize-1;56<=c;c--)this.v[c]=b&255,b/=256;Cf(this,this.v);for(c=b=0;5>c;c++)for(var d=24;0<=d;d-=8)a[b]=this.h[c]>>d&255,++b;return a};function Df(){}
Df.prototype.next=function(){return Ef};
var Ef={done:!0,value:void 0};function Ff(a){return{value:a,done:!1}}
Df.prototype.ja=function(){return this};function Gf(a){if(a instanceof Hf||a instanceof If||a instanceof Jf)return a;if("function"==typeof a.next)return new Hf(function(){return a});
if("function"==typeof a[Symbol.iterator])return new Hf(function(){return a[Symbol.iterator]()});
if("function"==typeof a.ja)return new Hf(function(){return a.ja()});
throw Error("Not an iterator or iterable.");}
function Hf(a){this.i=a}
Hf.prototype.ja=function(){return new If(this.i())};
Hf.prototype[Symbol.iterator]=function(){return new Jf(this.i())};
Hf.prototype.h=function(){return new Jf(this.i())};
function If(a){this.i=a}
v(If,Df);If.prototype.next=function(){return this.i.next()};
If.prototype[Symbol.iterator]=function(){return new Jf(this.i)};
If.prototype.h=function(){return new Jf(this.i)};
function Jf(a){Hf.call(this,function(){return a});
this.j=a}
v(Jf,Hf);Jf.prototype.next=function(){return this.j.next()};function Kf(a,b){this.i={};this.h=[];this.j=this.size=0;var c=arguments.length;if(1<c){if(c%2)throw Error("Uneven number of arguments");for(var d=0;d<c;d+=2)this.set(arguments[d],arguments[d+1])}else if(a)if(a instanceof Kf)for(c=Lf(a),d=0;d<c.length;d++)this.set(c[d],a.get(c[d]));else for(d in a)this.set(d,a[d])}
function Lf(a){Mf(a);return a.h.concat()}
q=Kf.prototype;q.has=function(a){return Nf(this.i,a)};
q.equals=function(a,b){if(this===a)return!0;if(this.size!=a.size)return!1;b=b||Of;Mf(this);for(var c,d=0;c=this.h[d];d++)if(!b(this.get(c),a.get(c)))return!1;return!0};
function Of(a,b){return a===b}
q.Lb=function(){return 0==this.size};
q.clear=function(){this.i={};this.j=this.size=this.h.length=0};
q.remove=function(a){return this.delete(a)};
q.delete=function(a){return Nf(this.i,a)?(delete this.i[a],--this.size,this.j++,this.h.length>2*this.size&&Mf(this),!0):!1};
function Mf(a){if(a.size!=a.h.length){for(var b=0,c=0;b<a.h.length;){var d=a.h[b];Nf(a.i,d)&&(a.h[c++]=d);b++}a.h.length=c}if(a.size!=a.h.length){var e={};for(c=b=0;b<a.h.length;)d=a.h[b],Nf(e,d)||(a.h[c++]=d,e[d]=1),b++;a.h.length=c}}
q.get=function(a,b){return Nf(this.i,a)?this.i[a]:b};
q.set=function(a,b){Nf(this.i,a)||(this.size+=1,this.h.push(a),this.j++);this.i[a]=b};
q.forEach=function(a,b){for(var c=Lf(this),d=0;d<c.length;d++){var e=c[d],f=this.get(e);a.call(b,f,e,this)}};
q.clone=function(){return new Kf(this)};
q.keys=function(){return Gf(this.ja(!0)).h()};
q.values=function(){return Gf(this.ja(!1)).h()};
q.entries=function(){var a=this;return zf(this.keys(),function(b){return[b,a.get(b)]})};
q.ja=function(a){Mf(this);var b=0,c=this.j,d=this,e=new Df;e.next=function(){if(c!=d.j)throw Error("The map has changed since the iterator was created");if(b>=d.h.length)return Ef;var f=d.h[b++];return Ff(a?f:d.i[f])};
return e};
function Nf(a,b){return Object.prototype.hasOwnProperty.call(a,b)}
;var Pf=z.JSON.stringify;function Qf(){var a=this;this.promise=new Promise(function(b,c){a.resolve=b;a.reject=c})}
;function Rf(a){this.h=0;this.H=void 0;this.m=this.i=this.j=null;this.v=this.s=!1;if(a!=cb)try{var b=this;a.call(void 0,function(c){Sf(b,2,c)},function(c){Sf(b,3,c)})}catch(c){Sf(this,3,c)}}
function Tf(){this.next=this.context=this.i=this.j=this.h=null;this.m=!1}
Tf.prototype.reset=function(){this.context=this.i=this.j=this.h=null;this.m=!1};
var Uf=new lf(function(){return new Tf},function(a){a.reset()});
function Vf(a,b,c){var d=Uf.get();d.j=a;d.i=b;d.context=c;return d}
Rf.prototype.then=function(a,b,c){return Wf(this,"function"===typeof a?a:null,"function"===typeof b?b:null,c)};
Rf.prototype.$goog_Thenable=!0;Rf.prototype.cancel=function(a){if(0==this.h){var b=new Xf(a);vf(function(){Yf(this,b)},this)}};
function Yf(a,b){if(0==a.h)if(a.j){var c=a.j;if(c.i){for(var d=0,e=null,f=null,g=c.i;g&&(g.m||(d++,g.h==a&&(e=g),!(e&&1<d)));g=g.next)e||(f=g);e&&(0==c.h&&1==d?Yf(c,b):(f?(d=f,d.next==c.m&&(c.m=d),d.next=d.next.next):Zf(c),$f(c,e,3,b)))}a.j=null}else Sf(a,3,b)}
function ag(a,b){a.i||2!=a.h&&3!=a.h||bg(a);a.m?a.m.next=b:a.i=b;a.m=b}
function Wf(a,b,c,d){var e=Vf(null,null,null);e.h=new Rf(function(f,g){e.j=b?function(h){try{var k=b.call(d,h);f(k)}catch(l){g(l)}}:f;
e.i=c?function(h){try{var k=c.call(d,h);void 0===k&&h instanceof Xf?g(h):f(k)}catch(l){g(l)}}:g});
e.h.j=a;ag(a,e);return e.h}
Rf.prototype.X=function(a){this.h=0;Sf(this,2,a)};
Rf.prototype.Ga=function(a){this.h=0;Sf(this,3,a)};
function Sf(a,b,c){if(0==a.h){a===c&&(b=3,c=new TypeError("Promise cannot resolve to itself"));a.h=1;a:{var d=c,e=a.X,f=a.Ga;if(d instanceof Rf){ag(d,Vf(e||cb,f||null,a));var g=!0}else{if(d)try{var h=!!d.$goog_Thenable}catch(l){h=!1}else h=!1;if(h)d.then(e,f,a),g=!0;else{if(Ra(d))try{var k=d.then;if("function"===typeof k){cg(d,k,e,f,a);g=!0;break a}}catch(l){f.call(a,l);g=!0;break a}g=!1}}}g||(a.H=c,a.h=b,a.j=null,bg(a),3!=b||c instanceof Xf||dg(a,c))}}
function cg(a,b,c,d,e){function f(k){h||(h=!0,d.call(e,k))}
function g(k){h||(h=!0,c.call(e,k))}
var h=!1;try{b.call(a,g,f)}catch(k){f(k)}}
function bg(a){a.s||(a.s=!0,vf(a.N,a))}
function Zf(a){var b=null;a.i&&(b=a.i,a.i=b.next,b.next=null);a.i||(a.m=null);return b}
Rf.prototype.N=function(){for(var a;a=Zf(this);)$f(this,a,this.h,this.H);this.s=!1};
function $f(a,b,c,d){if(3==c&&b.i&&!b.m)for(;a&&a.v;a=a.j)a.v=!1;if(b.h)b.h.j=null,eg(b,c,d);else try{b.m?b.j.call(b.context):eg(b,c,d)}catch(e){fg.call(null,e)}mf(Uf,b)}
function eg(a,b,c){2==b?a.j.call(a.context,c):a.i&&a.i.call(a.context,c)}
function dg(a,b){a.v=!0;vf(function(){a.v&&fg.call(null,b)})}
var fg=$b;function Xf(a){ab.call(this,a)}
D(Xf,ab);Xf.prototype.name="cancel";function L(a){he.call(this);this.H=1;this.m=[];this.s=0;this.h=[];this.i={};this.N=!!a}
D(L,he);q=L.prototype;q.subscribe=function(a,b,c){var d=this.i[a];d||(d=this.i[a]=[]);var e=this.H;this.h[e]=a;this.h[e+1]=b;this.h[e+2]=c;this.H=e+3;d.push(e);return e};
function gg(a,b,c){var d=hg;if(a=d.i[a]){var e=d.h;(a=a.find(function(f){return e[f+1]==b&&e[f+2]==c}))&&d.nb(a)}}
q.nb=function(a){var b=this.h[a];if(b){var c=this.i[b];0!=this.s?(this.m.push(a),this.h[a+1]=function(){}):(c&&hb(c,a),delete this.h[a],delete this.h[a+1],delete this.h[a+2])}return!!b};
q.ab=function(a,b){var c=this.i[a];if(c){for(var d=Array(arguments.length-1),e=1,f=arguments.length;e<f;e++)d[e-1]=arguments[e];if(this.N)for(e=0;e<c.length;e++){var g=c[e];ig(this.h[g+1],this.h[g+2],d)}else{this.s++;try{for(e=0,f=c.length;e<f&&!this.j;e++)g=c[e],this.h[g+1].apply(this.h[g+2],d)}finally{if(this.s--,0<this.m.length&&0==this.s)for(;c=this.m.pop();)this.nb(c)}}return 0!=e}return!1};
function ig(a,b,c){vf(function(){a.apply(b,c)})}
q.clear=function(a){if(a){var b=this.i[a];b&&(b.forEach(this.nb,this),delete this.i[a])}else this.h.length=0,this.i={}};
q.Sa=function(){L.pa.Sa.call(this);this.clear();this.m.length=0};function jg(a){this.h=a}
jg.prototype.set=function(a,b){void 0===b?this.h.remove(a):this.h.set(a,Pf(b))};
jg.prototype.get=function(a){try{var b=this.h.get(a)}catch(c){return}if(null!==b)try{return JSON.parse(b)}catch(c){throw"Storage: Invalid value was encountered";}};
jg.prototype.remove=function(a){this.h.remove(a)};function kg(a){this.h=a}
D(kg,jg);function lg(a){this.data=a}
function mg(a){return void 0===a||a instanceof lg?a:new lg(a)}
kg.prototype.set=function(a,b){kg.pa.set.call(this,a,mg(b))};
kg.prototype.i=function(a){a=kg.pa.get.call(this,a);if(void 0===a||a instanceof Object)return a;throw"Storage: Invalid value was encountered";};
kg.prototype.get=function(a){if(a=this.i(a)){if(a=a.data,void 0===a)throw"Storage: Invalid value was encountered";}else a=void 0;return a};function ng(a){this.h=a}
D(ng,kg);ng.prototype.set=function(a,b,c){if(b=mg(b)){if(c){if(c<Date.now()){ng.prototype.remove.call(this,a);return}b.expiration=c}b.creation=Date.now()}ng.pa.set.call(this,a,b)};
ng.prototype.i=function(a){var b=ng.pa.i.call(this,a);if(b){var c=b.creation,d=b.expiration;if(d&&d<Date.now()||c&&c>Date.now())ng.prototype.remove.call(this,a);else return b}};function og(){}
;function pg(){}
D(pg,og);pg.prototype[Symbol.iterator]=function(){return Gf(this.ja(!0)).h()};
pg.prototype.clear=function(){var a=Array.from(this);a=u(a);for(var b=a.next();!b.done;b=a.next())this.remove(b.value)};function qg(a){this.h=a}
D(qg,pg);q=qg.prototype;q.set=function(a,b){try{this.h.setItem(a,b)}catch(c){if(0==this.h.length)throw"Storage mechanism: Storage disabled";throw"Storage mechanism: Quota exceeded";}};
q.get=function(a){a=this.h.getItem(a);if("string"!==typeof a&&null!==a)throw"Storage mechanism: Invalid value was encountered";return a};
q.remove=function(a){this.h.removeItem(a)};
q.ja=function(a){var b=0,c=this.h,d=new Df;d.next=function(){if(b>=c.length)return Ef;var e=c.key(b++);if(a)return Ff(e);e=c.getItem(e);if("string"!==typeof e)throw"Storage mechanism: Invalid value was encountered";return Ff(e)};
return d};
q.clear=function(){this.h.clear()};
q.key=function(a){return this.h.key(a)};function rg(){var a=null;try{a=window.localStorage||null}catch(b){}this.h=a}
D(rg,qg);function sg(a,b){this.i=a;this.h=null;var c;if(c=dc)c=!(9<=Number(tc));if(c){tg||(tg=new Kf);this.h=tg.get(a);this.h||(b?this.h=document.getElementById(b):(this.h=document.createElement("userdata"),this.h.addBehavior("#default#userData"),document.body.appendChild(this.h)),tg.set(a,this.h));try{this.h.load(this.i)}catch(d){this.h=null}}}
D(sg,pg);var ug={".":".2E","!":".21","~":".7E","*":".2A","'":".27","(":".28",")":".29","%":"."},tg=null;function vg(a){return"_"+encodeURIComponent(a).replace(/[.!~*'()%]/g,function(b){return ug[b]})}
q=sg.prototype;q.set=function(a,b){this.h.setAttribute(vg(a),b);wg(this)};
q.get=function(a){a=this.h.getAttribute(vg(a));if("string"!==typeof a&&null!==a)throw"Storage mechanism: Invalid value was encountered";return a};
q.remove=function(a){this.h.removeAttribute(vg(a));wg(this)};
q.ja=function(a){var b=0,c=this.h.XMLDocument.documentElement.attributes,d=new Df;d.next=function(){if(b>=c.length)return Ef;var e=c[b++];if(a)return Ff(decodeURIComponent(e.nodeName.replace(/\./g,"%")).slice(1));e=e.nodeValue;if("string"!==typeof e)throw"Storage mechanism: Invalid value was encountered";return Ff(e)};
return d};
q.clear=function(){for(var a=this.h.XMLDocument.documentElement,b=a.attributes.length;0<b;b--)a.removeAttribute(a.attributes[b-1].nodeName);wg(this)};
function wg(a){try{a.h.save(a.i)}catch(b){throw"Storage mechanism: Quota exceeded";}}
;function xg(a,b){this.i=a;this.h=b+"::"}
D(xg,pg);xg.prototype.set=function(a,b){this.i.set(this.h+a,b)};
xg.prototype.get=function(a){return this.i.get(this.h+a)};
xg.prototype.remove=function(a){this.i.remove(this.h+a)};
xg.prototype.ja=function(a){var b=this.i[Symbol.iterator](),c=this,d=new Df;d.next=function(){var e=b.next();if(e.done)return e;for(e=e.value;e.slice(0,c.h.length)!=c.h;){e=b.next();if(e.done)return e;e=e.value}return Ff(a?e.slice(c.h.length):c.i.get(e))};
return d};/*

 (The MIT License)

 Copyright (C) 2014 by Vitaly Puzrin

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.

 -----------------------------------------------------------------------------
 Ported from zlib, which is under the following license
 https://github.com/madler/zlib/blob/master/zlib.h

 zlib.h -- interface of the 'zlib' general purpose compression library
   version 1.2.8, April 28th, 2013
   Copyright (C) 1995-2013 Jean-loup Gailly and Mark Adler
   This software is provided 'as-is', without any express or implied
   warranty.  In no event will the authors be held liable for any damages
   arising from the use of this software.
   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:
   1. The origin of this software must not be misrepresented; you must not
      claim that you wrote the original software. If you use this software
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.
   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original software.
   3. This notice may not be removed or altered from any source distribution.
   Jean-loup Gailly        Mark Adler
   jloup@gzip.org          madler@alumni.caltech.edu
   The data format used by the zlib library is described by RFCs (Request for
   Comments) 1950 to 1952 in the files http://tools.ietf.org/html/rfc1950
   (zlib format), rfc1951 (deflate format) and rfc1952 (gzip format).
*/
var M={},yg="undefined"!==typeof Uint8Array&&"undefined"!==typeof Uint16Array&&"undefined"!==typeof Int32Array;M.assign=function(a){for(var b=Array.prototype.slice.call(arguments,1);b.length;){var c=b.shift();if(c){if("object"!==typeof c)throw new TypeError(c+"must be non-object");for(var d in c)Object.prototype.hasOwnProperty.call(c,d)&&(a[d]=c[d])}}return a};
M.Ub=function(a,b){if(a.length===b)return a;if(a.subarray)return a.subarray(0,b);a.length=b;return a};
var zg={Pa:function(a,b,c,d,e){if(b.subarray&&a.subarray)a.set(b.subarray(c,c+d),e);else for(var f=0;f<d;f++)a[e+f]=b[c+f]},
fc:function(a){var b,c;var d=c=0;for(b=a.length;d<b;d++)c+=a[d].length;var e=new Uint8Array(c);d=c=0;for(b=a.length;d<b;d++){var f=a[d];e.set(f,c);c+=f.length}return e}},Ag={Pa:function(a,b,c,d,e){for(var f=0;f<d;f++)a[e+f]=b[c+f]},
fc:function(a){return[].concat.apply([],a)}};
M.md=function(){yg?(M.Ha=Uint8Array,M.qa=Uint16Array,M.Fc=Int32Array,M.assign(M,zg)):(M.Ha=Array,M.qa=Array,M.Fc=Array,M.assign(M,Ag))};
M.md();var Bg=!0;try{new Uint8Array(1)}catch(a){Bg=!1}for(var Cg=new M.Ha(256),Dg=0;256>Dg;Dg++)Cg[Dg]=252<=Dg?6:248<=Dg?5:240<=Dg?4:224<=Dg?3:192<=Dg?2:1;Cg[254]=Cg[254]=1;
function Eg(a){var b,c,d=a.length,e=0;for(b=0;b<d;b++){var f=a.charCodeAt(b);if(55296===(f&64512)&&b+1<d){var g=a.charCodeAt(b+1);56320===(g&64512)&&(f=65536+(f-55296<<10)+(g-56320),b++)}e+=128>f?1:2048>f?2:65536>f?3:4}var h=new M.Ha(e);for(b=c=0;c<e;b++)f=a.charCodeAt(b),55296===(f&64512)&&b+1<d&&(g=a.charCodeAt(b+1),56320===(g&64512)&&(f=65536+(f-55296<<10)+(g-56320),b++)),128>f?h[c++]=f:(2048>f?h[c++]=192|f>>>6:(65536>f?h[c++]=224|f>>>12:(h[c++]=240|f>>>18,h[c++]=128|f>>>12&63),h[c++]=128|f>>>
6&63),h[c++]=128|f&63);return h}
;var Fg={};Fg=function(a,b,c,d){var e=a&65535|0;a=a>>>16&65535|0;for(var f;0!==c;){f=2E3<c?2E3:c;c-=f;do e=e+b[d++]|0,a=a+e|0;while(--f);e%=65521;a%=65521}return e|a<<16|0};for(var ch={},dh,eh=[],fh=0;256>fh;fh++){dh=fh;for(var gh=0;8>gh;gh++)dh=dh&1?3988292384^dh>>>1:dh>>>1;eh[fh]=dh}ch=function(a,b,c,d){c=d+c;for(a^=-1;d<c;d++)a=a>>>8^eh[(a^b[d])&255];return a^-1};var hh={};hh={2:"need dictionary",1:"stream end",0:"","-1":"file error","-2":"stream error","-3":"data error","-4":"insufficient memory","-5":"buffer error","-6":"incompatible version"};function ih(a){for(var b=a.length;0<=--b;)a[b]=0}
var jh=[0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0],kh=[0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13],lh=[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7],mh=[16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15],nh=Array(576);ih(nh);var oh=Array(60);ih(oh);var ph=Array(512);ih(ph);var qh=Array(256);ih(qh);var rh=Array(29);ih(rh);var sh=Array(30);ih(sh);function th(a,b,c,d,e){this.Ac=a;this.Rc=b;this.Qc=c;this.Nc=d;this.gd=e;this.jc=a&&a.length}
var uh,vh,wh;function xh(a,b){this.dc=a;this.Xa=0;this.Ca=b}
function yh(a,b){a.J[a.pending++]=b&255;a.J[a.pending++]=b>>>8&255}
function O(a,b,c){a.P>16-c?(a.V|=b<<a.P&65535,yh(a,a.V),a.V=b>>16-a.P,a.P+=c-16):(a.V|=b<<a.P&65535,a.P+=c)}
function zh(a,b,c){O(a,c[2*b],c[2*b+1])}
function Ah(a,b){var c=0;do c|=a&1,a>>>=1,c<<=1;while(0<--b);return c>>>1}
function Bh(a,b,c){var d=Array(16),e=0,f;for(f=1;15>=f;f++)d[f]=e=e+c[f-1]<<1;for(c=0;c<=b;c++)e=a[2*c+1],0!==e&&(a[2*c]=Ah(d[e]++,e))}
function Ch(a){var b;for(b=0;286>b;b++)a.Y[2*b]=0;for(b=0;30>b;b++)a.Ja[2*b]=0;for(b=0;19>b;b++)a.S[2*b]=0;a.Y[512]=1;a.wa=a.bb=0;a.ea=a.matches=0}
function Dh(a){8<a.P?yh(a,a.V):0<a.P&&(a.J[a.pending++]=a.V);a.V=0;a.P=0}
function Eh(a,b,c){Dh(a);yh(a,c);yh(a,~c);M.Pa(a.J,a.window,b,c,a.pending);a.pending+=c}
function Fh(a,b,c,d){var e=2*b,f=2*c;return a[e]<a[f]||a[e]===a[f]&&d[b]<=d[c]}
function Gh(a,b,c){for(var d=a.L[c],e=c<<1;e<=a.va;){e<a.va&&Fh(b,a.L[e+1],a.L[e],a.depth)&&e++;if(Fh(b,d,a.L[e],a.depth))break;a.L[c]=a.L[e];c=e;e<<=1}a.L[c]=d}
function Hh(a,b,c){var d=0;if(0!==a.ea){do{var e=a.J[a.fb+2*d]<<8|a.J[a.fb+2*d+1];var f=a.J[a.Mb+d];d++;if(0===e)zh(a,f,b);else{var g=qh[f];zh(a,g+256+1,b);var h=jh[g];0!==h&&(f-=rh[g],O(a,f,h));e--;g=256>e?ph[e]:ph[256+(e>>>7)];zh(a,g,c);h=kh[g];0!==h&&(e-=sh[g],O(a,e,h))}}while(d<a.ea)}zh(a,256,b)}
function Ih(a,b){var c=b.dc,d=b.Ca.Ac,e=b.Ca.jc,f=b.Ca.Nc,g,h=-1;a.va=0;a.Ua=573;for(g=0;g<f;g++)0!==c[2*g]?(a.L[++a.va]=h=g,a.depth[g]=0):c[2*g+1]=0;for(;2>a.va;){var k=a.L[++a.va]=2>h?++h:0;c[2*k]=1;a.depth[k]=0;a.wa--;e&&(a.bb-=d[2*k+1])}b.Xa=h;for(g=a.va>>1;1<=g;g--)Gh(a,c,g);k=f;do g=a.L[1],a.L[1]=a.L[a.va--],Gh(a,c,1),d=a.L[1],a.L[--a.Ua]=g,a.L[--a.Ua]=d,c[2*k]=c[2*g]+c[2*d],a.depth[k]=(a.depth[g]>=a.depth[d]?a.depth[g]:a.depth[d])+1,c[2*g+1]=c[2*d+1]=k,a.L[1]=k++,Gh(a,c,1);while(2<=a.va);a.L[--a.Ua]=
a.L[1];g=b.dc;k=b.Xa;d=b.Ca.Ac;e=b.Ca.jc;f=b.Ca.Rc;var l=b.Ca.Qc,m=b.Ca.gd,n,r=0;for(n=0;15>=n;n++)a.ra[n]=0;g[2*a.L[a.Ua]+1]=0;for(b=a.Ua+1;573>b;b++){var p=a.L[b];n=g[2*g[2*p+1]+1]+1;n>m&&(n=m,r++);g[2*p+1]=n;if(!(p>k)){a.ra[n]++;var y=0;p>=l&&(y=f[p-l]);var A=g[2*p];a.wa+=A*(n+y);e&&(a.bb+=A*(d[2*p+1]+y))}}if(0!==r){do{for(n=m-1;0===a.ra[n];)n--;a.ra[n]--;a.ra[n+1]+=2;a.ra[m]--;r-=2}while(0<r);for(n=m;0!==n;n--)for(p=a.ra[n];0!==p;)d=a.L[--b],d>k||(g[2*d+1]!==n&&(a.wa+=(n-g[2*d+1])*g[2*d],g[2*
d+1]=n),p--)}Bh(c,h,a.ra)}
function Jh(a,b,c){var d,e=-1,f=b[1],g=0,h=7,k=4;0===f&&(h=138,k=3);b[2*(c+1)+1]=65535;for(d=0;d<=c;d++){var l=f;f=b[2*(d+1)+1];++g<h&&l===f||(g<k?a.S[2*l]+=g:0!==l?(l!==e&&a.S[2*l]++,a.S[32]++):10>=g?a.S[34]++:a.S[36]++,g=0,e=l,0===f?(h=138,k=3):l===f?(h=6,k=3):(h=7,k=4))}}
function Kh(a,b,c){var d,e=-1,f=b[1],g=0,h=7,k=4;0===f&&(h=138,k=3);for(d=0;d<=c;d++){var l=f;f=b[2*(d+1)+1];if(!(++g<h&&l===f)){if(g<k){do zh(a,l,a.S);while(0!==--g)}else 0!==l?(l!==e&&(zh(a,l,a.S),g--),zh(a,16,a.S),O(a,g-3,2)):10>=g?(zh(a,17,a.S),O(a,g-3,3)):(zh(a,18,a.S),O(a,g-11,7));g=0;e=l;0===f?(h=138,k=3):l===f?(h=6,k=3):(h=7,k=4)}}}
function Lh(a){var b=4093624447,c;for(c=0;31>=c;c++,b>>>=1)if(b&1&&0!==a.Y[2*c])return 0;if(0!==a.Y[18]||0!==a.Y[20]||0!==a.Y[26])return 1;for(c=32;256>c;c++)if(0!==a.Y[2*c])return 1;return 0}
var Mh=!1;function Nh(a,b,c){a.J[a.fb+2*a.ea]=b>>>8&255;a.J[a.fb+2*a.ea+1]=b&255;a.J[a.Mb+a.ea]=c&255;a.ea++;0===b?a.Y[2*c]++:(a.matches++,b--,a.Y[2*(qh[c]+256+1)]++,a.Ja[2*(256>b?ph[b]:ph[256+(b>>>7)])]++);return a.ea===a.ib-1}
;function Oh(a,b){a.msg=hh[b];return b}
function Ph(a){for(var b=a.length;0<=--b;)a[b]=0}
function Qh(a){var b=a.state,c=b.pending;c>a.F&&(c=a.F);0!==c&&(M.Pa(a.jb,b.J,b.kb,c,a.Ya),a.Ya+=c,b.kb+=c,a.Vb+=c,a.F-=c,b.pending-=c,0===b.pending&&(b.kb=0))}
function Q(a,b){var c=0<=a.aa?a.aa:-1,d=a.l-a.aa,e=0;if(0<a.level){2===a.B.Ib&&(a.B.Ib=Lh(a));Ih(a,a.zb);Ih(a,a.vb);Jh(a,a.Y,a.zb.Xa);Jh(a,a.Ja,a.vb.Xa);Ih(a,a.Yb);for(e=18;3<=e&&0===a.S[2*mh[e]+1];e--);a.wa+=3*(e+1)+14;var f=a.wa+3+7>>>3;var g=a.bb+3+7>>>3;g<=f&&(f=g)}else f=g=d+5;if(d+4<=f&&-1!==c)O(a,b?1:0,3),Eh(a,c,d);else if(4===a.strategy||g===f)O(a,2+(b?1:0),3),Hh(a,nh,oh);else{O(a,4+(b?1:0),3);c=a.zb.Xa+1;d=a.vb.Xa+1;e+=1;O(a,c-257,5);O(a,d-1,5);O(a,e-4,4);for(f=0;f<e;f++)O(a,a.S[2*mh[f]+
1],3);Kh(a,a.Y,c-1);Kh(a,a.Ja,d-1);Hh(a,a.Y,a.Ja)}Ch(a);b&&Dh(a);a.aa=a.l;Qh(a.B)}
function R(a,b){a.J[a.pending++]=b}
function Rh(a,b){a.J[a.pending++]=b>>>8&255;a.J[a.pending++]=b&255}
function Sh(a,b){var c=a.qc,d=a.l,e=a.ba,f=a.tc,g=a.l>a.T-262?a.l-(a.T-262):0,h=a.window,k=a.Da,l=a.oa,m=a.l+258,n=h[d+e-1],r=h[d+e];a.ba>=a.ic&&(c>>=2);f>a.o&&(f=a.o);do{var p=b;if(h[p+e]===r&&h[p+e-1]===n&&h[p]===h[d]&&h[++p]===h[d+1]){d+=2;for(p++;h[++d]===h[++p]&&h[++d]===h[++p]&&h[++d]===h[++p]&&h[++d]===h[++p]&&h[++d]===h[++p]&&h[++d]===h[++p]&&h[++d]===h[++p]&&h[++d]===h[++p]&&d<m;);p=258-(m-d);d=m-258;if(p>e){a.Wa=b;e=p;if(p>=f)break;n=h[d+e-1];r=h[d+e]}}}while((b=l[b&k])>g&&0!==--c);return e<=
a.o?e:a.o}
function Th(a){var b=a.T,c;do{var d=a.Dc-a.o-a.l;if(a.l>=b+(b-262)){M.Pa(a.window,a.window,b,b,0);a.Wa-=b;a.l-=b;a.aa-=b;var e=c=a.yb;do{var f=a.head[--e];a.head[e]=f>=b?f-b:0}while(--c);e=c=b;do f=a.oa[--e],a.oa[e]=f>=b?f-b:0;while(--c);d+=b}if(0===a.B.U)break;e=a.B;c=a.window;f=a.l+a.o;var g=e.U;g>d&&(g=d);0===g?c=0:(e.U-=g,M.Pa(c,e.input,e.Na,g,f),1===e.state.wrap?e.A=Fg(e.A,c,g,f):2===e.state.wrap&&(e.A=ch(e.A,c,g,f)),e.Na+=g,e.Oa+=g,c=g);a.o+=c;if(3<=a.o+a.Z)for(d=a.l-a.Z,a.C=a.window[d],a.C=
(a.C<<a.ta^a.window[d+1])&a.sa;a.Z&&!(a.C=(a.C<<a.ta^a.window[d+3-1])&a.sa,a.oa[d&a.Da]=a.head[a.C],a.head[a.C]=d,d++,a.Z--,3>a.o+a.Z););}while(262>a.o&&0!==a.B.U)}
function Uh(a,b){for(var c;;){if(262>a.o){Th(a);if(262>a.o&&0===b)return 1;if(0===a.o)break}c=0;3<=a.o&&(a.C=(a.C<<a.ta^a.window[a.l+3-1])&a.sa,c=a.oa[a.l&a.Da]=a.head[a.C],a.head[a.C]=a.l);0!==c&&a.l-c<=a.T-262&&(a.G=Sh(a,c));if(3<=a.G)if(c=Nh(a,a.l-a.Wa,a.G-3),a.o-=a.G,a.G<=a.Ob&&3<=a.o){a.G--;do a.l++,a.C=(a.C<<a.ta^a.window[a.l+3-1])&a.sa,a.oa[a.l&a.Da]=a.head[a.C],a.head[a.C]=a.l;while(0!==--a.G);a.l++}else a.l+=a.G,a.G=0,a.C=a.window[a.l],a.C=(a.C<<a.ta^a.window[a.l+1])&a.sa;else c=Nh(a,0,a.window[a.l]),
a.o--,a.l++;if(c&&(Q(a,!1),0===a.B.F))return 1}a.Z=2>a.l?a.l:2;return 4===b?(Q(a,!0),0===a.B.F?3:4):a.ea&&(Q(a,!1),0===a.B.F)?1:2}
function Vh(a,b){for(var c,d;;){if(262>a.o){Th(a);if(262>a.o&&0===b)return 1;if(0===a.o)break}c=0;3<=a.o&&(a.C=(a.C<<a.ta^a.window[a.l+3-1])&a.sa,c=a.oa[a.l&a.Da]=a.head[a.C],a.head[a.C]=a.l);a.ba=a.G;a.vc=a.Wa;a.G=2;0!==c&&a.ba<a.Ob&&a.l-c<=a.T-262&&(a.G=Sh(a,c),5>=a.G&&(1===a.strategy||3===a.G&&4096<a.l-a.Wa)&&(a.G=2));if(3<=a.ba&&a.G<=a.ba){d=a.l+a.o-3;c=Nh(a,a.l-1-a.vc,a.ba-3);a.o-=a.ba-1;a.ba-=2;do++a.l<=d&&(a.C=(a.C<<a.ta^a.window[a.l+3-1])&a.sa,a.oa[a.l&a.Da]=a.head[a.C],a.head[a.C]=a.l);while(0!==
--a.ba);a.La=0;a.G=2;a.l++;if(c&&(Q(a,!1),0===a.B.F))return 1}else if(a.La){if((c=Nh(a,0,a.window[a.l-1]))&&Q(a,!1),a.l++,a.o--,0===a.B.F)return 1}else a.La=1,a.l++,a.o--}a.La&&(Nh(a,0,a.window[a.l-1]),a.La=0);a.Z=2>a.l?a.l:2;return 4===b?(Q(a,!0),0===a.B.F?3:4):a.ea&&(Q(a,!1),0===a.B.F)?1:2}
function Wh(a,b){for(var c,d,e,f=a.window;;){if(258>=a.o){Th(a);if(258>=a.o&&0===b)return 1;if(0===a.o)break}a.G=0;if(3<=a.o&&0<a.l&&(d=a.l-1,c=f[d],c===f[++d]&&c===f[++d]&&c===f[++d])){for(e=a.l+258;c===f[++d]&&c===f[++d]&&c===f[++d]&&c===f[++d]&&c===f[++d]&&c===f[++d]&&c===f[++d]&&c===f[++d]&&d<e;);a.G=258-(e-d);a.G>a.o&&(a.G=a.o)}3<=a.G?(c=Nh(a,1,a.G-3),a.o-=a.G,a.l+=a.G,a.G=0):(c=Nh(a,0,a.window[a.l]),a.o--,a.l++);if(c&&(Q(a,!1),0===a.B.F))return 1}a.Z=0;return 4===b?(Q(a,!0),0===a.B.F?3:4):a.ea&&
(Q(a,!1),0===a.B.F)?1:2}
function Xh(a,b){for(var c;;){if(0===a.o&&(Th(a),0===a.o)){if(0===b)return 1;break}a.G=0;c=Nh(a,0,a.window[a.l]);a.o--;a.l++;if(c&&(Q(a,!1),0===a.B.F))return 1}a.Z=0;return 4===b?(Q(a,!0),0===a.B.F?3:4):a.ea&&(Q(a,!1),0===a.B.F)?1:2}
function Yh(a,b,c,d,e){this.Uc=a;this.fd=b;this.jd=c;this.ed=d;this.Sc=e}
var Zh;Zh=[new Yh(0,0,0,0,function(a,b){var c=65535;for(c>a.fa-5&&(c=a.fa-5);;){if(1>=a.o){Th(a);if(0===a.o&&0===b)return 1;if(0===a.o)break}a.l+=a.o;a.o=0;var d=a.aa+c;if(0===a.l||a.l>=d)if(a.o=a.l-d,a.l=d,Q(a,!1),0===a.B.F)return 1;if(a.l-a.aa>=a.T-262&&(Q(a,!1),0===a.B.F))return 1}a.Z=0;if(4===b)return Q(a,!0),0===a.B.F?3:4;a.l>a.aa&&Q(a,!1);return 1}),
new Yh(4,4,8,4,Uh),new Yh(4,5,16,8,Uh),new Yh(4,6,32,32,Uh),new Yh(4,4,16,16,Vh),new Yh(8,16,32,32,Vh),new Yh(8,16,128,128,Vh),new Yh(8,32,128,256,Vh),new Yh(32,128,258,1024,Vh),new Yh(32,258,258,4096,Vh)];
function $h(){this.B=null;this.status=0;this.J=null;this.wrap=this.pending=this.kb=this.fa=0;this.u=null;this.ga=0;this.method=8;this.Va=-1;this.Da=this.Wb=this.T=0;this.window=null;this.Dc=0;this.head=this.oa=null;this.tc=this.ic=this.strategy=this.level=this.Ob=this.qc=this.ba=this.o=this.Wa=this.l=this.La=this.vc=this.G=this.aa=this.ta=this.sa=this.Jb=this.yb=this.C=0;this.Y=new M.qa(1146);this.Ja=new M.qa(122);this.S=new M.qa(78);Ph(this.Y);Ph(this.Ja);Ph(this.S);this.Yb=this.vb=this.zb=null;
this.ra=new M.qa(16);this.L=new M.qa(573);Ph(this.L);this.Ua=this.va=0;this.depth=new M.qa(573);Ph(this.depth);this.P=this.V=this.Z=this.matches=this.bb=this.wa=this.fb=this.ea=this.ib=this.Mb=0}
function ai(a,b){if(!a||!a.state||5<b||0>b)return a?Oh(a,-2):-2;var c=a.state;if(!a.jb||!a.input&&0!==a.U||666===c.status&&4!==b)return Oh(a,0===a.F?-5:-2);c.B=a;var d=c.Va;c.Va=b;if(42===c.status)if(2===c.wrap)a.A=0,R(c,31),R(c,139),R(c,8),c.u?(R(c,(c.u.text?1:0)+(c.u.Aa?2:0)+(c.u.za?4:0)+(c.u.name?8:0)+(c.u.comment?16:0)),R(c,c.u.time&255),R(c,c.u.time>>8&255),R(c,c.u.time>>16&255),R(c,c.u.time>>24&255),R(c,9===c.level?2:2<=c.strategy||2>c.level?4:0),R(c,c.u.Rd&255),c.u.za&&c.u.za.length&&(R(c,
c.u.za.length&255),R(c,c.u.za.length>>8&255)),c.u.Aa&&(a.A=ch(a.A,c.J,c.pending,0)),c.ga=0,c.status=69):(R(c,0),R(c,0),R(c,0),R(c,0),R(c,0),R(c,9===c.level?2:2<=c.strategy||2>c.level?4:0),R(c,3),c.status=113);else{var e=8+(c.Wb-8<<4)<<8;e|=(2<=c.strategy||2>c.level?0:6>c.level?1:6===c.level?2:3)<<6;0!==c.l&&(e|=32);c.status=113;Rh(c,e+(31-e%31));0!==c.l&&(Rh(c,a.A>>>16),Rh(c,a.A&65535));a.A=1}if(69===c.status)if(c.u.za){for(e=c.pending;c.ga<(c.u.za.length&65535)&&(c.pending!==c.fa||(c.u.Aa&&c.pending>
e&&(a.A=ch(a.A,c.J,c.pending-e,e)),Qh(a),e=c.pending,c.pending!==c.fa));)R(c,c.u.za[c.ga]&255),c.ga++;c.u.Aa&&c.pending>e&&(a.A=ch(a.A,c.J,c.pending-e,e));c.ga===c.u.za.length&&(c.ga=0,c.status=73)}else c.status=73;if(73===c.status)if(c.u.name){e=c.pending;do{if(c.pending===c.fa&&(c.u.Aa&&c.pending>e&&(a.A=ch(a.A,c.J,c.pending-e,e)),Qh(a),e=c.pending,c.pending===c.fa)){var f=1;break}f=c.ga<c.u.name.length?c.u.name.charCodeAt(c.ga++)&255:0;R(c,f)}while(0!==f);c.u.Aa&&c.pending>e&&(a.A=ch(a.A,c.J,c.pending-
e,e));0===f&&(c.ga=0,c.status=91)}else c.status=91;if(91===c.status)if(c.u.comment){e=c.pending;do{if(c.pending===c.fa&&(c.u.Aa&&c.pending>e&&(a.A=ch(a.A,c.J,c.pending-e,e)),Qh(a),e=c.pending,c.pending===c.fa)){f=1;break}f=c.ga<c.u.comment.length?c.u.comment.charCodeAt(c.ga++)&255:0;R(c,f)}while(0!==f);c.u.Aa&&c.pending>e&&(a.A=ch(a.A,c.J,c.pending-e,e));0===f&&(c.status=103)}else c.status=103;103===c.status&&(c.u.Aa?(c.pending+2>c.fa&&Qh(a),c.pending+2<=c.fa&&(R(c,a.A&255),R(c,a.A>>8&255),a.A=0,
c.status=113)):c.status=113);if(0!==c.pending){if(Qh(a),0===a.F)return c.Va=-1,0}else if(0===a.U&&(b<<1)-(4<b?9:0)<=(d<<1)-(4<d?9:0)&&4!==b)return Oh(a,-5);if(666===c.status&&0!==a.U)return Oh(a,-5);if(0!==a.U||0!==c.o||0!==b&&666!==c.status){d=2===c.strategy?Xh(c,b):3===c.strategy?Wh(c,b):Zh[c.level].Sc(c,b);if(3===d||4===d)c.status=666;if(1===d||3===d)return 0===a.F&&(c.Va=-1),0;if(2===d&&(1===b?(O(c,2,3),zh(c,256,nh),16===c.P?(yh(c,c.V),c.V=0,c.P=0):8<=c.P&&(c.J[c.pending++]=c.V&255,c.V>>=8,c.P-=
8)):5!==b&&(O(c,0,3),Eh(c,0,0),3===b&&(Ph(c.head),0===c.o&&(c.l=0,c.aa=0,c.Z=0))),Qh(a),0===a.F))return c.Va=-1,0}if(4!==b)return 0;if(0>=c.wrap)return 1;2===c.wrap?(R(c,a.A&255),R(c,a.A>>8&255),R(c,a.A>>16&255),R(c,a.A>>24&255),R(c,a.Oa&255),R(c,a.Oa>>8&255),R(c,a.Oa>>16&255),R(c,a.Oa>>24&255)):(Rh(c,a.A>>>16),Rh(c,a.A&65535));Qh(a);0<c.wrap&&(c.wrap=-c.wrap);return 0!==c.pending?0:1}
;var bi={};bi=function(){this.input=null;this.Oa=this.U=this.Na=0;this.jb=null;this.Vb=this.F=this.Ya=0;this.msg="";this.state=null;this.Ib=2;this.A=0};var ci=Object.prototype.toString;
function di(a){if(!(this instanceof di))return new di(a);a=this.options=M.assign({level:-1,method:8,chunkSize:16384,Ea:15,hd:8,strategy:0,Cc:""},a||{});a.raw&&0<a.Ea?a.Ea=-a.Ea:a.Vc&&0<a.Ea&&16>a.Ea&&(a.Ea+=16);this.err=0;this.msg="";this.ended=!1;this.chunks=[];this.B=new bi;this.B.F=0;var b=this.B;var c=a.level,d=a.method,e=a.Ea,f=a.hd,g=a.strategy;if(b){var h=1;-1===c&&(c=6);0>e?(h=0,e=-e):15<e&&(h=2,e-=16);if(1>f||9<f||8!==d||8>e||15<e||0>c||9<c||0>g||4<g)b=Oh(b,-2);else{8===e&&(e=9);var k=new $h;
b.state=k;k.B=b;k.wrap=h;k.u=null;k.Wb=e;k.T=1<<k.Wb;k.Da=k.T-1;k.Jb=f+7;k.yb=1<<k.Jb;k.sa=k.yb-1;k.ta=~~((k.Jb+3-1)/3);k.window=new M.Ha(2*k.T);k.head=new M.qa(k.yb);k.oa=new M.qa(k.T);k.ib=1<<f+6;k.fa=4*k.ib;k.J=new M.Ha(k.fa);k.fb=1*k.ib;k.Mb=3*k.ib;k.level=c;k.strategy=g;k.method=d;if(b&&b.state){b.Oa=b.Vb=0;b.Ib=2;c=b.state;c.pending=0;c.kb=0;0>c.wrap&&(c.wrap=-c.wrap);c.status=c.wrap?42:113;b.A=2===c.wrap?0:1;c.Va=0;if(!Mh){d=Array(16);for(f=g=0;28>f;f++)for(rh[f]=g,e=0;e<1<<jh[f];e++)qh[g++]=
f;qh[g-1]=f;for(f=g=0;16>f;f++)for(sh[f]=g,e=0;e<1<<kh[f];e++)ph[g++]=f;for(g>>=7;30>f;f++)for(sh[f]=g<<7,e=0;e<1<<kh[f]-7;e++)ph[256+g++]=f;for(e=0;15>=e;e++)d[e]=0;for(e=0;143>=e;)nh[2*e+1]=8,e++,d[8]++;for(;255>=e;)nh[2*e+1]=9,e++,d[9]++;for(;279>=e;)nh[2*e+1]=7,e++,d[7]++;for(;287>=e;)nh[2*e+1]=8,e++,d[8]++;Bh(nh,287,d);for(e=0;30>e;e++)oh[2*e+1]=5,oh[2*e]=Ah(e,5);uh=new th(nh,jh,257,286,15);vh=new th(oh,kh,0,30,15);wh=new th([],lh,0,19,7);Mh=!0}c.zb=new xh(c.Y,uh);c.vb=new xh(c.Ja,vh);c.Yb=new xh(c.S,
wh);c.V=0;c.P=0;Ch(c);c=0}else c=Oh(b,-2);0===c&&(b=b.state,b.Dc=2*b.T,Ph(b.head),b.Ob=Zh[b.level].fd,b.ic=Zh[b.level].Uc,b.tc=Zh[b.level].jd,b.qc=Zh[b.level].ed,b.l=0,b.aa=0,b.o=0,b.Z=0,b.G=b.ba=2,b.La=0,b.C=0);b=c}}else b=-2;if(0!==b)throw Error(hh[b]);a.header&&(b=this.B)&&b.state&&2===b.state.wrap&&(b.state.u=a.header);if(a.gb){var l;"string"===typeof a.gb?l=Eg(a.gb):"[object ArrayBuffer]"===ci.call(a.gb)?l=new Uint8Array(a.gb):l=a.gb;a=this.B;f=l;g=f.length;if(a&&a.state)if(l=a.state,b=l.wrap,
2===b||1===b&&42!==l.status||l.o)b=-2;else{1===b&&(a.A=Fg(a.A,f,g,0));l.wrap=0;g>=l.T&&(0===b&&(Ph(l.head),l.l=0,l.aa=0,l.Z=0),c=new M.Ha(l.T),M.Pa(c,f,g-l.T,l.T,0),f=c,g=l.T);c=a.U;d=a.Na;e=a.input;a.U=g;a.Na=0;a.input=f;for(Th(l);3<=l.o;){f=l.l;g=l.o-2;do l.C=(l.C<<l.ta^l.window[f+3-1])&l.sa,l.oa[f&l.Da]=l.head[l.C],l.head[l.C]=f,f++;while(--g);l.l=f;l.o=2;Th(l)}l.l+=l.o;l.aa=l.l;l.Z=l.o;l.o=0;l.G=l.ba=2;l.La=0;a.Na=d;a.input=e;a.U=c;l.wrap=b;b=0}else b=-2;if(0!==b)throw Error(hh[b]);this.Id=!0}}
di.prototype.push=function(a,b){var c=this.B,d=this.options.chunkSize;if(this.ended)return!1;var e=b===~~b?b:!0===b?4:0;"string"===typeof a?c.input=Eg(a):"[object ArrayBuffer]"===ci.call(a)?c.input=new Uint8Array(a):c.input=a;c.Na=0;c.U=c.input.length;do{0===c.F&&(c.jb=new M.Ha(d),c.Ya=0,c.F=d);a=ai(c,e);if(1!==a&&0!==a)return ei(this,a),this.ended=!0,!1;if(0===c.F||0===c.U&&(4===e||2===e))if("string"===this.options.Cc){var f=M.Ub(c.jb,c.Ya);b=f;f=f.length;if(65537>f&&(b.subarray&&Bg||!b.subarray))b=
String.fromCharCode.apply(null,M.Ub(b,f));else{for(var g="",h=0;h<f;h++)g+=String.fromCharCode(b[h]);b=g}this.chunks.push(b)}else b=M.Ub(c.jb,c.Ya),this.chunks.push(b)}while((0<c.U||0===c.F)&&1!==a);if(4===e)return(c=this.B)&&c.state?(d=c.state.status,42!==d&&69!==d&&73!==d&&91!==d&&103!==d&&113!==d&&666!==d?a=Oh(c,-2):(c.state=null,a=113===d?Oh(c,-3):0)):a=-2,ei(this,a),this.ended=!0,0===a;2===e&&(ei(this,0),c.F=0);return!0};
function ei(a,b){0===b&&(a.result="string"===a.options.Cc?a.chunks.join(""):M.fc(a.chunks));a.chunks=[];a.err=b;a.msg=a.B.msg}
;function fi(a){this.name=a}
;var gi=new fi("rawColdConfigGroup");var hi=new fi("rawHotConfigGroup");function ii(a){K.call(this,a)}
v(ii,K);function ji(a){K.call(this,a)}
v(ji,K);function ki(a){K.call(this,a,-1,li)}
v(ki,K);var li=[2];function mi(a){K.call(this,a,-1,ni)}
v(mi,K);mi.prototype.getPlayerType=function(){return Vc(this,36)};
mi.prototype.setHomeGroupInfo=function(a){return J(this,ki,81,a)};
var ni=[9,66,24,32,86,100,101];function oi(a){K.call(this,a)}
v(oi,K);oi.prototype.getKey=function(){return hd(this,1)};
oi.prototype.la=function(){return hd(this,2===Zc(this,pi)?2:-1)};
var pi=[2,3,4,5,6];function qi(a){K.call(this,a,-1,ri)}
v(qi,K);var ri=[15,26,28];function si(a){K.call(this,a,-1,ti)}
v(si,K);var ti=[5];function ui(a){K.call(this,a)}
v(ui,K);function vi(a){K.call(this,a,-1,wi)}
v(vi,K);vi.prototype.setSafetyMode=function(a){return H(this,5,a)};
var wi=[12];function xi(a){K.call(this,a,-1,yi)}
v(xi,K);var yi=[12];var zi={Hd:"WEB_DISPLAY_MODE_UNKNOWN",Dd:"WEB_DISPLAY_MODE_BROWSER",Fd:"WEB_DISPLAY_MODE_MINIMAL_UI",Gd:"WEB_DISPLAY_MODE_STANDALONE",Ed:"WEB_DISPLAY_MODE_FULLSCREEN"};function Ai(a){K.call(this,a)}
v(Ai,K);Ai.prototype.getKey=function(){return hd(this,1)};
Ai.prototype.la=function(){return hd(this,2)};function Bi(a){K.call(this,a,-1,Ci)}
v(Bi,K);var Ci=[4,5];function Di(a){K.call(this,a)}
v(Di,K);function Ei(a){K.call(this,a)}
v(Ei,K);var Fi=[2,3,4];function Gi(a){K.call(this,a)}
v(Gi,K);function Hi(a){K.call(this,a)}
v(Hi,K);function Ii(a){K.call(this,a)}
v(Ii,K);function Ji(a){K.call(this,a,-1,Ki)}
v(Ji,K);var Ki=[10,17];function Li(a){K.call(this,a)}
v(Li,K);function Mi(a){K.call(this,a)}
v(Mi,K);function Ni(a){K.call(this,a)}
v(Ni,K);function Oi(a){K.call(this,a,459)}
v(Oi,K);
var Pi=[2,3,5,6,7,11,13,20,21,22,23,24,28,32,37,45,59,72,73,74,76,78,79,80,85,91,97,100,102,105,111,117,119,126,127,136,146,148,151,156,157,158,159,163,164,168,176,177,178,179,184,188,189,190,191,193,194,195,196,197,198,199,200,201,202,203,204,205,206,208,209,215,219,222,225,226,227,229,232,233,234,240,241,244,247,248,249,251,254,255,256,257,258,259,260,261,266,270,272,278,288,291,293,300,304,308,309,310,311,313,314,319,320,321,323,324,327,328,330,331,332,334,337,338,340,344,348,350,351,352,353,354,
355,356,357,358,361,363,364,368,369,370,373,374,375,378,380,381,383,388,389,402,403,410,411,412,413,414,415,416,417,418,423,424,425,426,427,429,430,431,439,441,444,448,458];function Qi(a){K.call(this,a)}
v(Qi,K);function Ri(a){K.call(this,a)}
v(Ri,K);Ri.prototype.getPlaylistId=function(){return id(this,2)};
var jd=[1,2];function Si(a){K.call(this,a,-1,Ti)}
v(Si,K);var Ti=[3];var Ui=z.window,Vi,Wi,Xi=(null==Ui?void 0:null==(Vi=Ui.yt)?void 0:Vi.config_)||(null==Ui?void 0:null==(Wi=Ui.ytcfg)?void 0:Wi.data_)||{};B("yt.config_",Xi);function Yi(){var a=arguments;1<a.length?Xi[a[0]]=a[1]:1===a.length&&Object.assign(Xi,a[0])}
function U(a,b){return a in Xi?Xi[a]:b}
function Zi(){return U("LATEST_ECATCHER_SERVICE_TRACKING_PARAMS")}
function $i(){var a=Xi.EXPERIMENT_FLAGS;return a?a.web_disable_gel_stp_ecatcher_killswitch:void 0}
;var aj=[];function bj(a){aj.forEach(function(b){return b(a)})}
function cj(a){return a&&window.yterr?function(){try{return a.apply(this,arguments)}catch(b){dj(b)}}:a}
function dj(a,b,c,d){var e=C("yt.logging.errors.log");e?e(a,"ERROR",b,c,d):(e=U("ERRORS",[]),e.push([a,"ERROR",b,c,d]),Yi("ERRORS",e));bj(a)}
function ej(a,b,c,d){var e=C("yt.logging.errors.log");e?e(a,"WARNING",b,c,d):(e=U("ERRORS",[]),e.push([a,"WARNING",b,c,d]),Yi("ERRORS",e))}
;function W(a){a=fj(a);return"string"===typeof a&&"false"===a?!1:!!a}
function gj(a,b){a=fj(a);return void 0===a&&void 0!==b?b:Number(a||0)}
function fj(a){var b=U("EXPERIMENTS_FORCED_FLAGS",{})||{};return void 0!==b[a]?b[a]:U("EXPERIMENT_FLAGS",{})[a]}
function hj(){for(var a=[],b=U("EXPERIMENTS_FORCED_FLAGS",{}),c=u(Object.keys(b)),d=c.next();!d.done;d=c.next())d=d.value,a.push({key:d,value:String(b[d])});c=U("EXPERIMENT_FLAGS",{});var e=u(Object.keys(c));for(d=e.next();!d.done;d=e.next())d=d.value,d.startsWith("force_")&&void 0===b[d]&&a.push({key:d,value:String(c[d])});return a}
;var ij=0;B("ytDomDomGetNextId",C("ytDomDomGetNextId")||function(){return++ij});var jj={stopImmediatePropagation:1,stopPropagation:1,preventMouseEvent:1,preventManipulation:1,preventDefault:1,layerX:1,layerY:1,screenX:1,screenY:1,scale:1,rotation:1,webkitMovementX:1,webkitMovementY:1};
function kj(a){this.type="";this.state=this.source=this.data=this.currentTarget=this.relatedTarget=this.target=null;this.charCode=this.keyCode=0;this.metaKey=this.shiftKey=this.ctrlKey=this.altKey=!1;this.clientY=this.clientX=0;this.changedTouches=this.touches=null;try{if(a=a||window.event){this.event=a;for(var b in a)b in jj||(this[b]=a[b]);var c=a.target||a.srcElement;c&&3==c.nodeType&&(c=c.parentNode);this.target=c;var d=a.relatedTarget;if(d)try{d=d.nodeName?d:null}catch(e){d=null}else"mouseover"==
this.type?d=a.fromElement:"mouseout"==this.type&&(d=a.toElement);this.relatedTarget=d;this.clientX=void 0!=a.clientX?a.clientX:a.pageX;this.clientY=void 0!=a.clientY?a.clientY:a.pageY;this.keyCode=a.keyCode?a.keyCode:a.which;this.charCode=a.charCode||("keypress"==this.type?this.keyCode:0);this.altKey=a.altKey;this.ctrlKey=a.ctrlKey;this.shiftKey=a.shiftKey;this.metaKey=a.metaKey;this.h=a.pageX;this.i=a.pageY}}catch(e){}}
function lj(a){if(document.body&&document.documentElement){var b=document.body.scrollTop+document.documentElement.scrollTop;a.h=a.clientX+(document.body.scrollLeft+document.documentElement.scrollLeft);a.i=a.clientY+b}}
kj.prototype.preventDefault=function(){this.event&&(this.event.returnValue=!1,this.event.preventDefault&&this.event.preventDefault())};
kj.prototype.stopPropagation=function(){this.event&&(this.event.cancelBubble=!0,this.event.stopPropagation&&this.event.stopPropagation())};
kj.prototype.stopImmediatePropagation=function(){this.event&&(this.event.cancelBubble=!0,this.event.stopImmediatePropagation&&this.event.stopImmediatePropagation())};var ob=z.ytEventsEventsListeners||{};B("ytEventsEventsListeners",ob);var mj=z.ytEventsEventsCounter||{count:0};B("ytEventsEventsCounter",mj);
function nj(a,b,c,d){d=void 0===d?{}:d;a.addEventListener&&("mouseenter"!=b||"onmouseenter"in document?"mouseleave"!=b||"onmouseenter"in document?"mousewheel"==b&&"MozBoxSizing"in document.documentElement.style&&(b="MozMousePixelScroll"):b="mouseout":b="mouseover");return nb(function(e){var f="boolean"===typeof e[4]&&e[4]==!!d,g=Ra(e[4])&&Ra(d)&&pb(e[4],d);return!!e.length&&e[0]==a&&e[1]==b&&e[2]==c&&(f||g)})}
function oj(a){a&&("string"==typeof a&&(a=[a]),E(a,function(b){if(b in ob){var c=ob[b],d=c[0],e=c[1],f=c[3];c=c[4];d.removeEventListener?pj()||"boolean"===typeof c?d.removeEventListener(e,f,c):d.removeEventListener(e,f,!!c.capture):d.detachEvent&&d.detachEvent("on"+e,f);delete ob[b]}}))}
var pj=db(function(){var a=!1;try{var b=Object.defineProperty({},"capture",{get:function(){a=!0}});
window.addEventListener("test",null,b)}catch(c){}return a});
function qj(a,b,c,d){d=void 0===d?{}:d;if(!a||!a.addEventListener&&!a.attachEvent)return"";var e=nj(a,b,c,d);if(e)return e;e=++mj.count+"";var f=!("mouseenter"!=b&&"mouseleave"!=b||!a.addEventListener||"onmouseenter"in document);var g=f?function(h){h=new kj(h);if(!Ad(h.relatedTarget,function(k){return k==a}))return h.currentTarget=a,h.type=b,c.call(a,h)}:function(h){h=new kj(h);
h.currentTarget=a;return c.call(a,h)};
g=cj(g);a.addEventListener?("mouseenter"==b&&f?b="mouseover":"mouseleave"==b&&f?b="mouseout":"mousewheel"==b&&"MozBoxSizing"in document.documentElement.style&&(b="MozMousePixelScroll"),pj()||"boolean"===typeof d?a.addEventListener(b,g,d):a.addEventListener(b,g,!!d.capture)):a.attachEvent("on"+b,g);ob[e]=[a,b,c,g,d];return e}
;function rj(a,b){"function"===typeof a&&(a=cj(a));return window.setTimeout(a,b)}
function sj(a,b){"function"===typeof a&&(a=cj(a));return window.setInterval(a,b)}
;var tj=/^[\w.]*$/,uj={q:!0,search_query:!0};function vj(a,b){b=a.split(b);for(var c={},d=0,e=b.length;d<e;d++){var f=b[d].split("=");if(1==f.length&&f[0]||2==f.length)try{var g=wj(f[0]||""),h=wj(f[1]||"");g in c?Array.isArray(c[g])?kb(c[g],h):c[g]=[c[g],h]:c[g]=h}catch(n){var k=n,l=f[0],m=String(vj);k.args=[{key:l,value:f[1],query:a,method:xj==m?"unchanged":m}];uj.hasOwnProperty(l)||ej(k)}}return c}
var xj=String(vj);function yj(a){var b=[];mb(a,function(c,d){var e=encodeURIComponent(String(d)),f;Array.isArray(c)?f=c:f=[c];E(f,function(g){""==g?b.push(e):b.push(e+"="+encodeURIComponent(String(g)))})});
return b.join("&")}
function zj(a){"?"==a.charAt(0)&&(a=a.substr(1));return vj(a,"&")}
function Aj(a,b,c){var d=a.split("#",2);a=d[0];d=1<d.length?"#"+d[1]:"";var e=a.split("?",2);a=e[0];e=zj(e[1]||"");for(var f in b)!c&&null!==e&&f in e||(e[f]=b[f]);b=a;a=Xb(e);a?(c=b.indexOf("#"),0>c&&(c=b.length),f=b.indexOf("?"),0>f||f>c?(f=c,e=""):e=b.substring(f+1,c),b=[b.slice(0,f),e,b.slice(c)],c=b[1],b[1]=a?c?c+"&"+a:a:c,a=b[0]+(b[1]?"?"+b[1]:"")+b[2]):a=b;return a+d}
function Bj(a){if(!b)var b=window.location.href;var c=a.match(Sb)[1]||null,d=Ub(a);c&&d?(a=a.match(Sb),b=b.match(Sb),a=a[3]==b[3]&&a[1]==b[1]&&a[4]==b[4]):a=d?Ub(b)==d&&(Number(b.match(Sb)[4]||null)||null)==(Number(a.match(Sb)[4]||null)||null):!0;return a}
function wj(a){return a&&a.match(tj)?a:decodeURIComponent(a.replace(/\+/g," "))}
;function Cj(a){var b=Dj;a=void 0===a?C("yt.ads.biscotti.lastId_")||"":a;var c=Object,d=c.assign,e={};e.dt=Td;e.flash="0";a:{try{var f=b.h.top.location.href}catch(aa){f=2;break a}f=f?f===b.i.location.href?0:1:2}e=(e.frm=f,e);try{e.u_tz=-(new Date).getTimezoneOffset();var g=void 0===g?wd:g;try{var h=g.history.length}catch(aa){h=0}e.u_his=h;var k;e.u_h=null==(k=wd.screen)?void 0:k.height;var l;e.u_w=null==(l=wd.screen)?void 0:l.width;var m;e.u_ah=null==(m=wd.screen)?void 0:m.availHeight;var n;e.u_aw=
null==(n=wd.screen)?void 0:n.availWidth;var r;e.u_cd=null==(r=wd.screen)?void 0:r.colorDepth}catch(aa){}h=b.h;try{var p=h.screenX;var y=h.screenY}catch(aa){}try{var A=h.outerWidth;var I=h.outerHeight}catch(aa){}try{var N=h.innerWidth;var T=h.innerHeight}catch(aa){}try{var P=h.screenLeft;var wa=h.screenTop}catch(aa){}try{N=h.innerWidth,T=h.innerHeight}catch(aa){}try{var Mc=h.screen.availWidth;var Ha=h.screen.availTop}catch(aa){}p=[P,wa,p,y,Mc,Ha,A,I,N,T];y=b.h.top;try{var xa=(y||window).document,ba=
"CSS1Compat"==xa.compatMode?xa.documentElement:xa.body;var ia=(new yd(ba.clientWidth,ba.clientHeight)).round()}catch(aa){ia=new yd(-12245933,-12245933)}xa=ia;ia={};var ja=void 0===ja?z:ja;ba=new Re;ja.SVGElement&&ja.document.createElementNS&&ba.set(0);y=Rd();y["allow-top-navigation-by-user-activation"]&&ba.set(1);y["allow-popups-to-escape-sandbox"]&&ba.set(2);ja.crypto&&ja.crypto.subtle&&ba.set(3);ja.TextDecoder&&ja.TextEncoder&&ba.set(4);ja=Se(ba);ia.bc=ja;ia.bih=xa.height;ia.biw=xa.width;ia.brdim=
p.join();b=b.i;b=(ia.vis=b.prerendering?3:{visible:1,hidden:2,prerender:3,preview:4,unloaded:5}[b.visibilityState||b.webkitVisibilityState||b.mozVisibilityState||""]||0,ia.wgl=!!wd.WebGLRenderingContext,ia);c=d.call(c,e,b);c.ca_type="image";a&&(c.bid=a);return c}
var Dj=new function(){var a=window.document;this.h=window;this.i=a};
B("yt.ads_.signals_.getAdSignalsString",function(a){return yj(Cj(a))});Date.now();var Ej="XMLHttpRequest"in z?function(){return new XMLHttpRequest}:null;
function Fj(){if(!Ej)return null;var a=Ej();return"open"in a?a:null}
;var Gj={Authorization:"AUTHORIZATION","X-Goog-EOM-Visitor-Id":"EOM_VISITOR_DATA","X-Goog-Visitor-Id":"SANDBOXED_VISITOR_ID","X-Youtube-Domain-Admin-State":"DOMAIN_ADMIN_STATE","X-Youtube-Chrome-Connected":"CHROME_CONNECTED_HEADER","X-YouTube-Client-Name":"INNERTUBE_CONTEXT_CLIENT_NAME","X-YouTube-Client-Version":"INNERTUBE_CONTEXT_CLIENT_VERSION","X-YouTube-Delegation-Context":"INNERTUBE_CONTEXT_SERIALIZED_DELEGATION_CONTEXT","X-YouTube-Device":"DEVICE","X-Youtube-Identity-Token":"ID_TOKEN","X-YouTube-Page-CL":"PAGE_CL",
"X-YouTube-Page-Label":"PAGE_BUILD_LABEL","X-YouTube-Variants-Checksum":"VARIANTS_CHECKSUM","X-Goog-AuthUser":"SESSION_INDEX","X-Goog-PageId":"DELEGATED_SESSION_ID"},Hj="app debugcss debugjs expflag force_ad_params force_ad_encrypted force_viral_ad_response_params forced_experiments innertube_snapshots innertube_goldens internalcountrycode internalipoverride absolute_experiments conditional_experiments sbb sr_bns_address".split(" ").concat(ka(Vd)),Ij=!1;
function Jj(a,b){b=void 0===b?{}:b;var c=Bj(a),d=W("web_ajax_ignore_global_headers_if_set"),e;for(e in Gj){var f=U(Gj[e]);"X-Goog-Visitor-Id"!==e||f||(f=U("VISITOR_DATA"));!f||!c&&Ub(a)||d&&void 0!==b[e]||!(W("move_vss_away_from_login_info_cookie")||"X-Goog-AuthUser"!==e&&"X-Goog-PageId"!==e)||(b[e]=f)}W("move_vss_away_from_login_info_cookie")&&(b["X-Yt-Auth-Test"]="test");"X-Goog-EOM-Visitor-Id"in b&&"X-Goog-Visitor-Id"in b&&delete b["X-Goog-Visitor-Id"];if(c||!Ub(a))b["X-YouTube-Utc-Offset"]=String(-(new Date).getTimezoneOffset());
if(c||!Ub(a)){try{var g=(new Intl.DateTimeFormat).resolvedOptions().timeZone}catch(h){}g&&(b["X-YouTube-Time-Zone"]=g)}document.location.hostname.endsWith("youtubeeducation.com")||!c&&Ub(a)||(b["X-YouTube-Ad-Signals"]=yj(Cj()));return b}
function Kj(a){var b=window.location.search,c=Ub(a);W("debug_handle_relative_url_for_query_forward_killswitch")||!c&&Bj(a)&&(c=document.location.hostname);var d=Tb(a.match(Sb)[5]||null);d=(c=c&&(c.endsWith("youtube.com")||c.endsWith("youtube-nocookie.com")))&&d&&d.startsWith("/api/");if(!c||d)return a;var e=zj(b),f={};E(Hj,function(g){e[g]&&(f[g]=e[g])});
return Aj(a,f||{},!1)}
function Lj(a,b){var c=b.format||"JSON";a=Mj(a,b);var d=Nj(a,b),e=!1,f=Oj(a,function(k){if(!e){e=!0;h&&window.clearTimeout(h);a:switch(k&&"status"in k?k.status:-1){case 200:case 201:case 202:case 203:case 204:case 205:case 206:case 304:var l=!0;break a;default:l=!1}var m=null,n=400<=k.status&&500>k.status,r=500<=k.status&&600>k.status;if(l||n||r)m=Pj(a,c,k,b.convertToSafeHtml);if(l)a:if(k&&204==k.status)l=!0;else{switch(c){case "XML":l=0==parseInt(m&&m.return_code,10);break a;case "RAW":l=!0;break a}l=
!!m}m=m||{};n=b.context||z;l?b.onSuccess&&b.onSuccess.call(n,k,m):b.onError&&b.onError.call(n,k,m);b.onFinish&&b.onFinish.call(n,k,m)}},b.method,d,b.headers,b.responseType,b.withCredentials);
d=b.timeout||0;if(b.onTimeout&&0<d){var g=b.onTimeout;var h=rj(function(){e||(e=!0,f.abort(),window.clearTimeout(h),g.call(b.context||z,f))},d)}return f}
function Mj(a,b){b.includeDomain&&(a=document.location.protocol+"//"+document.location.hostname+(document.location.port?":"+document.location.port:"")+a);var c=U("XSRF_FIELD_NAME");if(b=b.urlParams)b[c]&&delete b[c],a=Aj(a,b||{},!0);return a}
function Nj(a,b){var c=U("XSRF_FIELD_NAME"),d=U("XSRF_TOKEN"),e=b.postBody||"",f=b.postParams,g=U("XSRF_FIELD_NAME"),h;b.headers&&(h=b.headers["Content-Type"]);b.excludeXsrf||Ub(a)&&!b.withCredentials&&Ub(a)!=document.location.hostname||"POST"!=b.method||h&&"application/x-www-form-urlencoded"!=h||b.postParams&&b.postParams[g]||(f||(f={}),f[c]=d);(W("ajax_parse_query_data_only_when_filled")&&f&&0<Object.keys(f).length||f)&&"string"===typeof e&&(e=zj(e),sb(e,f),e=b.postBodyFormat&&"JSON"==b.postBodyFormat?
JSON.stringify(e):Xb(e));if(!(a=e)&&(a=f)){a:{for(var k in f){f=!1;break a}f=!0}a=!f}!Ij&&a&&"POST"!=b.method&&(Ij=!0,dj(Error("AJAX request with postData should use POST")));return e}
function Pj(a,b,c,d){var e=null;switch(b){case "JSON":try{var f=c.responseText}catch(g){throw d=Error("Error reading responseText"),d.params=a,ej(d),g;}a=c.getResponseHeader("Content-Type")||"";f&&0<=a.indexOf("json")&&(")]}'\n"===f.substring(0,5)&&(f=f.substring(5)),e=JSON.parse(f));break;case "XML":if(a=(a=c.responseXML)?Qj(a):null)e={},E(a.getElementsByTagName("*"),function(g){e[g.tagName]=Rj(g)})}d&&Sj(e);
return e}
function Sj(a){if(Ra(a))for(var b in a){var c;(c="html_content"==b)||(c=b.length-5,c=0<=c&&b.indexOf("_html",c)==c);if(c){c=b;var d=a[b];if(void 0===tb){var e=null;var f=z.trustedTypes;if(f&&f.createPolicy){try{e=f.createPolicy("goog#html",{createHTML:$a,createScript:$a,createScriptURL:$a})}catch(g){z.console&&z.console.error(g.message)}tb=e}else tb=e}d=(e=tb)?e.createHTML(d):d;a[c]=new Qb(d)}else Sj(a[b])}}
function Qj(a){return a?(a=("responseXML"in a?a.responseXML:a).getElementsByTagName("root"))&&0<a.length?a[0]:null:null}
function Rj(a){var b="";E(a.childNodes,function(c){b+=c.nodeValue});
return b}
function Tj(a,b){b.method="POST";b.postParams||(b.postParams={});return Lj(a,b)}
function Oj(a,b,c,d,e,f,g){function h(){4==(k&&"readyState"in k?k.readyState:0)&&b&&cj(b)(k)}
c=void 0===c?"GET":c;d=void 0===d?"":d;var k=Fj();if(!k)return null;"onloadend"in k?k.addEventListener("loadend",h,!1):k.onreadystatechange=h;W("debug_forward_web_query_parameters")&&(a=Kj(a));k.open(c,a,!0);f&&(k.responseType=f);g&&(k.withCredentials=!0);c="POST"==c&&(void 0===window.FormData||!(d instanceof FormData));if(e=Jj(a,e))for(var l in e)k.setRequestHeader(l,e[l]),"content-type"==l.toLowerCase()&&(c=!1);c&&k.setRequestHeader("Content-Type","application/x-www-form-urlencoded");k.send(d);
return k}
;var Uj=uc||vc;var Vj=[{Pb:function(a){return"Cannot read property '"+a.key+"'"},
Ab:{Error:[{regexp:/(Permission denied) to access property "([^']+)"/,groups:["reason","key"]}],TypeError:[{regexp:/Cannot read property '([^']+)' of (null|undefined)/,groups:["key","value"]},{regexp:/\u65e0\u6cd5\u83b7\u53d6\u672a\u5b9a\u4e49\u6216 (null|undefined) \u5f15\u7528\u7684\u5c5e\u6027\u201c([^\u201d]+)\u201d/,groups:["value","key"]},{regexp:/\uc815\uc758\ub418\uc9c0 \uc54a\uc74c \ub610\ub294 (null|undefined) \ucc38\uc870\uc778 '([^']+)' \uc18d\uc131\uc744 \uac00\uc838\uc62c \uc218 \uc5c6\uc2b5\ub2c8\ub2e4./,
groups:["value","key"]},{regexp:/No se puede obtener la propiedad '([^']+)' de referencia nula o sin definir/,groups:["key"]},{regexp:/Unable to get property '([^']+)' of (undefined or null) reference/,groups:["key","value"]},{regexp:/(null) is not an object \(evaluating '(?:([^.]+)\.)?([^']+)'\)/,groups:["value","base","key"]}]}},{Pb:function(a){return"Cannot call '"+a.key+"'"},
Ab:{TypeError:[{regexp:/(?:([^ ]+)?\.)?([^ ]+) is not a function/,groups:["base","key"]},{regexp:/([^ ]+) called on (null or undefined)/,groups:["key","value"]},{regexp:/Object (.*) has no method '([^ ]+)'/,groups:["base","key"]},{regexp:/Object doesn't support property or method '([^ ]+)'/,groups:["key"]},{regexp:/\u30aa\u30d6\u30b8\u30a7\u30af\u30c8\u306f '([^']+)' \u30d7\u30ed\u30d1\u30c6\u30a3\u307e\u305f\u306f\u30e1\u30bd\u30c3\u30c9\u3092\u30b5\u30dd\u30fc\u30c8\u3057\u3066\u3044\u307e\u305b\u3093/,
groups:["key"]},{regexp:/\uac1c\uccb4\uac00 '([^']+)' \uc18d\uc131\uc774\ub098 \uba54\uc11c\ub4dc\ub97c \uc9c0\uc6d0\ud558\uc9c0 \uc54a\uc2b5\ub2c8\ub2e4./,groups:["key"]}]}},{Pb:function(a){return a.key+" is not defined"},
Ab:{ReferenceError:[{regexp:/(.*) is not defined/,groups:["key"]},{regexp:/Can't find variable: (.*)/,groups:["key"]}]}}];var Xj={Ba:[],ya:[{Kc:Wj,weight:500}]};function Wj(a){if("JavaException"===a.name)return!0;a=a.stack;return a.includes("chrome://")||a.includes("chrome-extension://")||a.includes("moz-extension://")}
;function Yj(){this.ya=[];this.Ba=[]}
var Zj;function ak(){if(!Zj){var a=Zj=new Yj;a.Ba.length=0;a.ya.length=0;Xj.Ba&&a.Ba.push.apply(a.Ba,Xj.Ba);Xj.ya&&a.ya.push.apply(a.ya,Xj.ya)}return Zj}
;var bk=new L;function ck(a){function b(){return a.charCodeAt(d++)}
var c=a.length,d=0;do{var e=dk(b);if(Infinity===e)break;var f=e>>3;switch(e&7){case 0:e=dk(b);if(2===f)return e;break;case 1:if(2===f)return;d+=8;break;case 2:e=dk(b);if(2===f)return a.substr(d,e);d+=e;break;case 5:if(2===f)return;d+=4;break;default:return}}while(d<c)}
function dk(a){var b=a(),c=b&127;if(128>b)return c;b=a();c|=(b&127)<<7;if(128>b)return c;b=a();c|=(b&127)<<14;if(128>b)return c;b=a();return 128>b?c|(b&127)<<21:Infinity}
;function ek(a,b,c,d){if(a)if(Array.isArray(a)){var e=d;for(d=0;d<a.length&&!(a[d]&&(e+=fk(d,a[d],b,c),500<e));d++);d=e}else if("object"===typeof a)for(e in a){if(a[e]){var f=a[e];var g=b;var h=c;g="string"!==typeof f||"clickTrackingParams"!==e&&"trackingParams"!==e?0:(f=ck(atob(f.replace(/-/g,"+").replace(/_/g,"/"))))?fk(e+".ve",f,g,h):0;d+=g;d+=fk(e,a[e],b,c);if(500<d)break}}else c[b]=gk(a),d+=c[b].length;else c[b]=gk(a),d+=c[b].length;return d}
function fk(a,b,c,d){c+="."+a;a=gk(b);d[c]=a;return c.length+a.length}
function gk(a){try{return("string"===typeof a?a:String(JSON.stringify(a))).substr(0,500)}catch(b){return"unable to serialize "+typeof a+" ("+b.message+")"}}
;function hk(){}
;function ik(){if(!z.matchMedia)return"WEB_DISPLAY_MODE_UNKNOWN";try{return z.matchMedia("(display-mode: standalone)").matches?"WEB_DISPLAY_MODE_STANDALONE":z.matchMedia("(display-mode: minimal-ui)").matches?"WEB_DISPLAY_MODE_MINIMAL_UI":z.matchMedia("(display-mode: fullscreen)").matches?"WEB_DISPLAY_MODE_FULLSCREEN":z.matchMedia("(display-mode: browser)").matches?"WEB_DISPLAY_MODE_BROWSER":"WEB_DISPLAY_MODE_UNKNOWN"}catch(a){return"WEB_DISPLAY_MODE_UNKNOWN"}}
;B("ytglobal.prefsUserPrefsPrefs_",C("ytglobal.prefsUserPrefsPrefs_")||{});var jk={bluetooth:"CONN_DISCO",cellular:"CONN_CELLULAR_UNKNOWN",ethernet:"CONN_WIFI",none:"CONN_NONE",wifi:"CONN_WIFI",wimax:"CONN_CELLULAR_4G",other:"CONN_UNKNOWN",unknown:"CONN_UNKNOWN","slow-2g":"CONN_CELLULAR_2G","2g":"CONN_CELLULAR_2G","3g":"CONN_CELLULAR_3G","4g":"CONN_CELLULAR_4G"},kk={CONN_DEFAULT:0,CONN_UNKNOWN:1,CONN_NONE:2,CONN_WIFI:3,CONN_CELLULAR_2G:4,CONN_CELLULAR_3G:5,CONN_CELLULAR_4G:6,CONN_CELLULAR_UNKNOWN:7,CONN_DISCO:8,CONN_CELLULAR_5G:9,CONN_WIFI_METERED:10,CONN_CELLULAR_5G_SA:11,
CONN_CELLULAR_5G_NSA:12,CONN_INVALID:31},lk={EFFECTIVE_CONNECTION_TYPE_UNKNOWN:0,EFFECTIVE_CONNECTION_TYPE_OFFLINE:1,EFFECTIVE_CONNECTION_TYPE_SLOW_2G:2,EFFECTIVE_CONNECTION_TYPE_2G:3,EFFECTIVE_CONNECTION_TYPE_3G:4,EFFECTIVE_CONNECTION_TYPE_4G:5},mk={"slow-2g":"EFFECTIVE_CONNECTION_TYPE_SLOW_2G","2g":"EFFECTIVE_CONNECTION_TYPE_2G","3g":"EFFECTIVE_CONNECTION_TYPE_3G","4g":"EFFECTIVE_CONNECTION_TYPE_4G"};function nk(){var a=z.navigator;return a?a.connection:void 0}
;function ok(a){var b=La.apply(1,arguments);var c=Error.call(this,a);this.message=c.message;"stack"in c&&(this.stack=c.stack);this.args=[].concat(ka(b))}
v(ok,Error);function pk(){try{return qk(),!0}catch(a){return!1}}
function qk(){if(void 0!==U("DATASYNC_ID"))return U("DATASYNC_ID");throw new ok("Datasync ID not set","unknown");}
;function rk(){}
function sk(a,b){return tk(a,0,b)}
rk.prototype.ka=function(a,b){return tk(a,1,b)};function uk(){rk.apply(this,arguments)}
v(uk,rk);function vk(){uk.h||(uk.h=new uk);return uk.h}
function tk(a,b,c){void 0!==c&&Number.isNaN(Number(c))&&(c=void 0);var d=C("yt.scheduler.instance.addJob");return d?d(a,b,c):void 0===c?(a(),NaN):rj(a,c||0)}
uk.prototype.Qa=function(a){if(void 0===a||!Number.isNaN(Number(a))){var b=C("yt.scheduler.instance.cancelJob");b?b(a):window.clearTimeout(a)}};
uk.prototype.start=function(){var a=C("yt.scheduler.instance.start");a&&a()};
var Qe=vk();function wk(a){var b=new rg;if(b.h)try{b.h.setItem("__sak","1");b.h.removeItem("__sak");var c=!0}catch(d){c=!1}else c=!1;(b=c?a?new xg(b,a):b:null)||(a=new sg(a||"UserDataSharedStore"),b=a.h?a:null);this.h=(a=b)?new ng(a):null;this.i=document.domain||window.location.hostname}
wk.prototype.set=function(a,b,c,d){c=c||31104E3;this.remove(a);if(this.h)try{this.h.set(a,b,Date.now()+1E3*c);return}catch(f){}var e="";if(d)try{e=escape(Pf(b))}catch(f){return}else e=escape(b);b=this.i;be.set(""+a,e,{Nb:c,path:"/",domain:void 0===b?"youtube.com":b,secure:!1})};
wk.prototype.get=function(a,b){var c=void 0,d=!this.h;if(!d)try{c=this.h.get(a)}catch(e){d=!0}if(d&&(c=be.get(""+a,void 0))&&(c=unescape(c),b))try{c=JSON.parse(c)}catch(e){this.remove(a),c=void 0}return c};
wk.prototype.remove=function(a){this.h&&this.h.remove(a);var b=this.i;be.remove(""+a,"/",void 0===b?"youtube.com":b)};var xk=function(){var a;return function(){a||(a=new wk("ytidb"));return a}}();
function yk(){var a;return null==(a=xk())?void 0:a.get("LAST_RESULT_ENTRY_KEY",!0)}
;var zk=[],Ak=!1;function Bk(a){Ak||(zk.push({type:"ERROR",payload:a}),10<zk.length&&zk.shift())}
function Ck(a,b){Ak||(zk.push({type:"EVENT",eventType:a,payload:b}),10<zk.length&&zk.shift())}
;function Dk(a){if(0<=a.indexOf(":"))throw Error("Database name cannot contain ':'");}
function Ek(a){return a.substr(0,a.indexOf(":"))||a}
;var Fk={},Gk=(Fk.AUTH_INVALID="No user identifier specified.",Fk.EXPLICIT_ABORT="Transaction was explicitly aborted.",Fk.IDB_NOT_SUPPORTED="IndexedDB is not supported.",Fk.MISSING_INDEX="Index not created.",Fk.MISSING_OBJECT_STORES="Object stores not created.",Fk.DB_DELETED_BY_MISSING_OBJECT_STORES="Database is deleted because expected object stores were not created.",Fk.DB_REOPENED_BY_MISSING_OBJECT_STORES="Database is reopened because expected object stores were not created.",Fk.UNKNOWN_ABORT="Transaction was aborted for unknown reasons.",
Fk.QUOTA_EXCEEDED="The current transaction exceeded its quota limitations.",Fk.QUOTA_MAYBE_EXCEEDED="The current transaction may have failed because of exceeding quota limitations.",Fk.EXECUTE_TRANSACTION_ON_CLOSED_DB="Can't start a transaction on a closed database",Fk.INCOMPATIBLE_DB_VERSION="The binary is incompatible with the database version",Fk),Hk={},Ik=(Hk.AUTH_INVALID="ERROR",Hk.EXECUTE_TRANSACTION_ON_CLOSED_DB="WARNING",Hk.EXPLICIT_ABORT="IGNORED",Hk.IDB_NOT_SUPPORTED="ERROR",Hk.MISSING_INDEX=
"WARNING",Hk.MISSING_OBJECT_STORES="ERROR",Hk.DB_DELETED_BY_MISSING_OBJECT_STORES="WARNING",Hk.DB_REOPENED_BY_MISSING_OBJECT_STORES="WARNING",Hk.QUOTA_EXCEEDED="WARNING",Hk.QUOTA_MAYBE_EXCEEDED="WARNING",Hk.UNKNOWN_ABORT="WARNING",Hk.INCOMPATIBLE_DB_VERSION="WARNING",Hk),Jk={},Kk=(Jk.AUTH_INVALID=!1,Jk.EXECUTE_TRANSACTION_ON_CLOSED_DB=!1,Jk.EXPLICIT_ABORT=!1,Jk.IDB_NOT_SUPPORTED=!1,Jk.MISSING_INDEX=!1,Jk.MISSING_OBJECT_STORES=!1,Jk.DB_DELETED_BY_MISSING_OBJECT_STORES=!1,Jk.DB_REOPENED_BY_MISSING_OBJECT_STORES=
!1,Jk.QUOTA_EXCEEDED=!1,Jk.QUOTA_MAYBE_EXCEEDED=!0,Jk.UNKNOWN_ABORT=!0,Jk.INCOMPATIBLE_DB_VERSION=!1,Jk);function X(a,b,c,d,e){b=void 0===b?{}:b;c=void 0===c?Gk[a]:c;d=void 0===d?Ik[a]:d;e=void 0===e?Kk[a]:e;ok.call(this,c,Object.assign({},{name:"YtIdbKnownError",isSw:void 0===self.document,isIframe:self!==self.top,type:a},b));this.type=a;this.message=c;this.level=d;this.h=e;Object.setPrototypeOf(this,X.prototype)}
v(X,ok);function Lk(a,b){X.call(this,"MISSING_OBJECT_STORES",{expectedObjectStores:b,foundObjectStores:a},Gk.MISSING_OBJECT_STORES);Object.setPrototypeOf(this,Lk.prototype)}
v(Lk,X);function Mk(a,b){var c=Error.call(this);this.message=c.message;"stack"in c&&(this.stack=c.stack);this.index=a;this.objectStore=b;Object.setPrototypeOf(this,Mk.prototype)}
v(Mk,Error);var Nk=["The database connection is closing","Can't start a transaction on a closed database","A mutation operation was attempted on a database that did not allow mutations"];
function Ok(a,b,c,d){b=Ek(b);var e=a instanceof Error?a:Error("Unexpected error: "+a);if(e instanceof X)return e;a={objectStoreNames:c,dbName:b,dbVersion:d};if("QuotaExceededError"===e.name)return new X("QUOTA_EXCEEDED",a);if(wc&&"UnknownError"===e.name)return new X("QUOTA_MAYBE_EXCEEDED",a);if(e instanceof Mk)return new X("MISSING_INDEX",Object.assign({},a,{objectStore:e.objectStore,index:e.index}));if("InvalidStateError"===e.name&&Nk.some(function(f){return e.message.includes(f)}))return new X("EXECUTE_TRANSACTION_ON_CLOSED_DB",
a);
if("AbortError"===e.name)return new X("UNKNOWN_ABORT",a,e.message);e.args=[Object.assign({},a,{name:"IdbError",Qd:e.name})];e.level="WARNING";return e}
function Pk(a,b,c){var d=yk();return new X("IDB_NOT_SUPPORTED",{context:{caller:a,publicName:b,version:c,hasSucceededOnce:null==d?void 0:d.hasSucceededOnce}})}
;function Qk(a){if(!a)throw Error();throw a;}
function Rk(a){return a}
function Sk(a){this.h=a}
function Tk(a){function b(e){if("PENDING"===d.state.status){d.state={status:"REJECTED",reason:e};e=u(d.i);for(var f=e.next();!f.done;f=e.next())f=f.value,f()}}
function c(e){if("PENDING"===d.state.status){d.state={status:"FULFILLED",value:e};e=u(d.h);for(var f=e.next();!f.done;f=e.next())f=f.value,f()}}
var d=this;this.state={status:"PENDING"};this.h=[];this.i=[];a=a.h;try{a(c,b)}catch(e){b(e)}}
Tk.resolve=function(a){return new Tk(new Sk(function(b,c){a instanceof Tk?a.then(b,c):b(a)}))};
Tk.reject=function(a){return new Tk(new Sk(function(b,c){c(a)}))};
Tk.prototype.then=function(a,b){var c=this,d=null!=a?a:Rk,e=null!=b?b:Qk;return new Tk(new Sk(function(f,g){"PENDING"===c.state.status?(c.h.push(function(){Uk(c,c,d,f,g)}),c.i.push(function(){Vk(c,c,e,f,g)})):"FULFILLED"===c.state.status?Uk(c,c,d,f,g):"REJECTED"===c.state.status&&Vk(c,c,e,f,g)}))};
function Wk(a,b){a.then(void 0,b)}
function Uk(a,b,c,d,e){try{if("FULFILLED"!==a.state.status)throw Error("calling handleResolve before the promise is fulfilled.");var f=c(a.state.value);f instanceof Tk?Xk(a,b,f,d,e):d(f)}catch(g){e(g)}}
function Vk(a,b,c,d,e){try{if("REJECTED"!==a.state.status)throw Error("calling handleReject before the promise is rejected.");var f=c(a.state.reason);f instanceof Tk?Xk(a,b,f,d,e):d(f)}catch(g){e(g)}}
function Xk(a,b,c,d,e){b===c?e(new TypeError("Circular promise chain detected.")):c.then(function(f){f instanceof Tk?Xk(a,b,f,d,e):d(f)},function(f){e(f)})}
;function Yk(a,b,c){function d(){c(a.error);f()}
function e(){b(a.result);f()}
function f(){try{a.removeEventListener("success",e),a.removeEventListener("error",d)}catch(g){}}
a.addEventListener("success",e);a.addEventListener("error",d)}
function Zk(a){return new Promise(function(b,c){Yk(a,b,c)})}
function $k(a){return new Tk(new Sk(function(b,c){Yk(a,b,c)}))}
;function al(a,b){return new Tk(new Sk(function(c,d){function e(){var f=a?b(a):null;f?f.then(function(g){a=g;e()},d):c()}
e()}))}
;var Pl=window,Y=Pl.ytcsi&&Pl.ytcsi.now?Pl.ytcsi.now:Pl.performance&&Pl.performance.timing&&Pl.performance.now&&Pl.performance.timing.navigationStart?function(){return Pl.performance.timing.navigationStart+Pl.performance.now()}:function(){return(new Date).getTime()};function Ql(a,b){this.h=a;this.options=b;this.transactionCount=0;this.j=Math.round(Y());this.i=!1}
q=Ql.prototype;q.add=function(a,b,c){return Rl(this,[a],{mode:"readwrite",W:!0},function(d){return d.objectStore(a).add(b,c)})};
q.clear=function(a){return Rl(this,[a],{mode:"readwrite",W:!0},function(b){return b.objectStore(a).clear()})};
q.close=function(){this.h.close();var a;(null==(a=this.options)?0:a.closed)&&this.options.closed()};
function Sl(a,b,c){a=a.h.createObjectStore(b,c);return new Tl(a)}
q.delete=function(a,b){return Rl(this,[a],{mode:"readwrite",W:!0},function(c){return c.objectStore(a).delete(b)})};
q.get=function(a,b){return Rl(this,[a],{mode:"readonly",W:!0},function(c){return c.objectStore(a).get(b)})};
function Ul(a,b,c){return Rl(a,[b],{mode:"readwrite",W:!0},function(d){d=d.objectStore(b);return $k(d.h.put(c,void 0))})}
q.objectStoreNames=function(){return Array.from(this.h.objectStoreNames)};
function Rl(a,b,c,d){var e,f,g,h,k,l,m,n,r,p,y,A;return x(function(I){switch(I.h){case 1:var N={mode:"readonly",W:!1,tag:"IDB_TRANSACTION_TAG_UNKNOWN"};"string"===typeof c?N.mode=c:Object.assign(N,c);e=N;a.transactionCount++;f=e.W?3:1;g=0;case 2:if(h){I.D(3);break}g++;k=Math.round(Y());ya(I,4);l=a.h.transaction(b,e.mode);N=new Vl(l);N=Wl(N,d);return w(I,N,6);case 6:return m=I.i,n=Math.round(Y()),Xl(a,k,n,g,void 0,b.join(),e),I.return(m);case 4:r=Ba(I);p=Math.round(Y());y=Ok(r,a.h.name,b.join(),a.h.version);
if((A=y instanceof X&&!y.h)||g>=f)Xl(a,k,p,g,y,b.join(),e),h=y;I.D(2);break;case 3:return I.return(Promise.reject(h))}})}
function Xl(a,b,c,d,e,f,g){b=c-b;e?(e instanceof X&&("QUOTA_EXCEEDED"===e.type||"QUOTA_MAYBE_EXCEEDED"===e.type)&&Ck("QUOTA_EXCEEDED",{dbName:Ek(a.h.name),objectStoreNames:f,transactionCount:a.transactionCount,transactionMode:g.mode}),e instanceof X&&"UNKNOWN_ABORT"===e.type&&(c-=a.j,0>c&&c>=Math.pow(2,31)&&(c=0),Ck("TRANSACTION_UNEXPECTEDLY_ABORTED",{objectStoreNames:f,transactionDuration:b,transactionCount:a.transactionCount,dbDuration:c}),a.i=!0),Yl(a,!1,d,f,b,g.tag),Bk(e)):Yl(a,!0,d,f,b,g.tag)}
function Yl(a,b,c,d,e,f){Ck("TRANSACTION_ENDED",{objectStoreNames:d,connectionHasUnknownAbortedTransaction:a.i,duration:e,isSuccessful:b,tryCount:c,tag:void 0===f?"IDB_TRANSACTION_TAG_UNKNOWN":f})}
q.getName=function(){return this.h.name};
function Tl(a){this.h=a}
q=Tl.prototype;q.add=function(a,b){return $k(this.h.add(a,b))};
q.autoIncrement=function(){return this.h.autoIncrement};
q.clear=function(){return $k(this.h.clear()).then(function(){})};
function Zl(a,b,c){a.h.createIndex(b,c,{unique:!1})}
function $l(a,b){return am(a,{query:b},function(c){return c.delete().then(function(){return c.continue()})}).then(function(){})}
q.delete=function(a){return a instanceof IDBKeyRange?$l(this,a):$k(this.h.delete(a))};
q.get=function(a){return $k(this.h.get(a))};
q.index=function(a){try{return new bm(this.h.index(a))}catch(b){if(b instanceof Error&&"NotFoundError"===b.name)throw new Mk(a,this.h.name);throw b;}};
q.getName=function(){return this.h.name};
q.keyPath=function(){return this.h.keyPath};
function am(a,b,c){a=a.h.openCursor(b.query,b.direction);return cm(a).then(function(d){return al(d,c)})}
function Vl(a){var b=this;this.h=a;this.j=new Map;this.i=!1;this.done=new Promise(function(c,d){b.h.addEventListener("complete",function(){c()});
b.h.addEventListener("error",function(e){e.currentTarget===e.target&&d(b.h.error)});
b.h.addEventListener("abort",function(){var e=b.h.error;if(e)d(e);else if(!b.i){e=X;for(var f=b.h.objectStoreNames,g=[],h=0;h<f.length;h++){var k=f.item(h);if(null===k)throw Error("Invariant: item in DOMStringList is null");g.push(k)}e=new e("UNKNOWN_ABORT",{objectStoreNames:g.join(),dbName:b.h.db.name,mode:b.h.mode});d(e)}})})}
function Wl(a,b){var c=new Promise(function(d,e){try{Wk(b(a).then(function(f){d(f)}),e)}catch(f){e(f),a.abort()}});
return Promise.all([c,a.done]).then(function(d){return u(d).next().value})}
Vl.prototype.abort=function(){this.h.abort();this.i=!0;throw new X("EXPLICIT_ABORT");};
Vl.prototype.objectStore=function(a){a=this.h.objectStore(a);var b=this.j.get(a);b||(b=new Tl(a),this.j.set(a,b));return b};
function bm(a){this.h=a}
q=bm.prototype;q.delete=function(a){return dm(this,{query:a},function(b){return b.delete().then(function(){return b.continue()})})};
q.get=function(a){return $k(this.h.get(a))};
q.getKey=function(a){return $k(this.h.getKey(a))};
q.keyPath=function(){return this.h.keyPath};
q.unique=function(){return this.h.unique};
function dm(a,b,c){a=a.h.openCursor(void 0===b.query?null:b.query,void 0===b.direction?"next":b.direction);return cm(a).then(function(d){return al(d,c)})}
function em(a,b){this.request=a;this.cursor=b}
function cm(a){return $k(a).then(function(b){return b?new em(a,b):null})}
q=em.prototype;q.advance=function(a){this.cursor.advance(a);return cm(this.request)};
q.continue=function(a){this.cursor.continue(a);return cm(this.request)};
q.delete=function(){return $k(this.cursor.delete()).then(function(){})};
q.getKey=function(){return this.cursor.key};
q.la=function(){return this.cursor.value};
q.update=function(a){return $k(this.cursor.update(a))};function fm(a,b,c){return new Promise(function(d,e){function f(){r||(r=new Ql(g.result,{closed:n}));return r}
var g=void 0!==b?self.indexedDB.open(a,b):self.indexedDB.open(a);var h=c.Ic,k=c.Jc,l=c.nd,m=c.upgrade,n=c.closed,r;g.addEventListener("upgradeneeded",function(p){try{if(null===p.newVersion)throw Error("Invariant: newVersion on IDbVersionChangeEvent is null");if(null===g.transaction)throw Error("Invariant: transaction on IDbOpenDbRequest is null");p.dataLoss&&"none"!==p.dataLoss&&Ck("IDB_DATA_CORRUPTED",{reason:p.dataLossMessage||"unknown reason",dbName:Ek(a)});var y=f(),A=new Vl(g.transaction);m&&
m(y,function(I){return p.oldVersion<I&&p.newVersion>=I},A);
A.done.catch(function(I){e(I)})}catch(I){e(I)}});
g.addEventListener("success",function(){var p=g.result;k&&p.addEventListener("versionchange",function(){k(f())});
p.addEventListener("close",function(){Ck("IDB_UNEXPECTEDLY_CLOSED",{dbName:Ek(a),dbVersion:p.version});l&&l()});
d(f())});
g.addEventListener("error",function(){e(g.error)});
h&&g.addEventListener("blocked",function(){h()})})}
function gm(a,b,c){c=void 0===c?{}:c;return fm(a,b,c)}
function hm(a,b){b=void 0===b?{}:b;var c,d,e,f;return x(function(g){if(1==g.h)return ya(g,2),c=self.indexedDB.deleteDatabase(a),d=b,(e=d.Ic)&&c.addEventListener("blocked",function(){e()}),w(g,Zk(c),4);
if(2!=g.h)return Aa(g,0);f=Ba(g);throw Ok(f,a,"",-1);})}
;function im(a,b){this.name=a;this.options=b;this.j=!0;this.v=this.m=0}
im.prototype.i=function(a,b,c){c=void 0===c?{}:c;return gm(a,b,c)};
im.prototype.delete=function(a){a=void 0===a?{}:a;return hm(this.name,a)};
function jm(a,b){return new X("INCOMPATIBLE_DB_VERSION",{dbName:a.name,oldVersion:a.options.version,newVersion:b})}
function km(a,b){if(!b)throw Pk("openWithToken",Ek(a.name));return lm(a)}
function lm(a){function b(){var f,g,h,k,l,m,n,r,p,y;return x(function(A){switch(A.h){case 1:return g=null!=(f=Error().stack)?f:"",ya(A,2),w(A,a.i(a.name,a.options.version,d),4);case 4:h=A.i;for(var I=a.options,N=[],T=u(Object.keys(I.Za)),P=T.next();!P.done;P=T.next()){P=P.value;var wa=I.Za[P],Mc=void 0===wa.ld?Number.MAX_VALUE:wa.ld;!(h.h.version>=wa.eb)||h.h.version>=Mc||h.h.objectStoreNames.contains(P)||N.push(P)}k=N;if(0===k.length){A.D(5);break}l=Object.keys(a.options.Za);m=h.objectStoreNames();
if(a.v<gj("ytidb_reopen_db_retries",0))return a.v++,h.close(),Bk(new X("DB_REOPENED_BY_MISSING_OBJECT_STORES",{dbName:a.name,expectedObjectStores:l,foundObjectStores:m})),A.return(b());if(!(a.m<gj("ytidb_remake_db_retries",1))){A.D(6);break}a.m++;return w(A,a.delete(),7);case 7:return Bk(new X("DB_DELETED_BY_MISSING_OBJECT_STORES",{dbName:a.name,expectedObjectStores:l,foundObjectStores:m})),A.return(b());case 6:throw new Lk(m,l);case 5:return A.return(h);case 2:n=Ba(A);if(n instanceof DOMException?
"VersionError"!==n.name:"DOMError"in self&&n instanceof DOMError?"VersionError"!==n.name:!(n instanceof Object&&"message"in n)||"An attempt was made to open a database using a lower version than the existing version."!==n.message){A.D(8);break}return w(A,a.i(a.name,void 0,Object.assign({},d,{upgrade:void 0})),9);case 9:r=A.i;p=r.h.version;if(void 0!==a.options.version&&p>a.options.version+1)throw r.close(),a.j=!1,jm(a,p);return A.return(r);case 8:throw c(),n instanceof Error&&!W("ytidb_async_stack_killswitch")&&
(n.stack=n.stack+"\n"+g.substring(g.indexOf("\n")+1)),Ok(n,a.name,"",null!=(y=a.options.version)?y:-1);}})}
function c(){a.h===e&&(a.h=void 0)}
if(!a.j)throw jm(a);if(a.h)return a.h;var d={Jc:function(f){f.close()},
closed:c,nd:c,upgrade:a.options.upgrade};var e=b();a.h=e;return a.h}
;var mm=new im("YtIdbMeta",{Za:{databases:{eb:1}},upgrade:function(a,b){b(1)&&Sl(a,"databases",{keyPath:"actualName"})}});
function nm(a,b){var c;return x(function(d){if(1==d.h)return w(d,km(mm,b),2);c=d.i;return d.return(Rl(c,["databases"],{W:!0,mode:"readwrite"},function(e){var f=e.objectStore("databases");return f.get(a.actualName).then(function(g){if(g?a.actualName!==g.actualName||a.publicName!==g.publicName||a.userIdentifier!==g.userIdentifier:1)return $k(f.h.put(a,void 0)).then(function(){})})}))})}
function om(a,b){var c;return x(function(d){if(1==d.h)return a?w(d,km(mm,b),2):d.return();c=d.i;return d.return(c.delete("databases",a))})}
function pm(a,b){var c,d;return x(function(e){return 1==e.h?(c=[],w(e,km(mm,b),2)):3!=e.h?(d=e.i,w(e,Rl(d,["databases"],{W:!0,mode:"readonly"},function(f){c.length=0;return am(f.objectStore("databases"),{},function(g){a(g.la())&&c.push(g.la());return g.continue()})}),3)):e.return(c)})}
function qm(a){return pm(function(b){return"LogsDatabaseV2"===b.publicName&&void 0!==b.userIdentifier},a)}
;var rm,sm=new function(){}(new function(){});
function tm(){var a,b,c,d;return x(function(e){switch(e.h){case 1:a=yk();if(null==(b=a)?0:b.hasSucceededOnce)return e.return(!0);var f;if(f=Uj)f=/WebKit\/([0-9]+)/.exec(Nb()),f=!!(f&&600<=parseInt(f[1],10));f&&(f=/WebKit\/([0-9]+)/.exec(Nb()),f=!(f&&602<=parseInt(f[1],10)));if(f||ec)return e.return(!1);try{if(c=self,!(c.indexedDB&&c.IDBIndex&&c.IDBKeyRange&&c.IDBObjectStore))return e.return(!1)}catch(g){return e.return(!1)}if(!("IDBTransaction"in self&&"objectStoreNames"in IDBTransaction.prototype))return e.return(!1);
ya(e,2);d={actualName:"yt-idb-test-do-not-use",publicName:"yt-idb-test-do-not-use",userIdentifier:void 0};return w(e,nm(d,sm),4);case 4:return w(e,om("yt-idb-test-do-not-use",sm),5);case 5:return e.return(!0);case 2:return Ba(e),e.return(!1)}})}
function um(){if(void 0!==rm)return rm;Ak=!0;return rm=tm().then(function(a){Ak=!1;var b;if(null!=(b=xk())&&b.h){var c;b={hasSucceededOnce:(null==(c=yk())?void 0:c.hasSucceededOnce)||a};var d;null==(d=xk())||d.set("LAST_RESULT_ENTRY_KEY",b,2592E3,!0)}return a})}
function vm(){return C("ytglobal.idbToken_")||void 0}
function wm(){var a=vm();return a?Promise.resolve(a):um().then(function(b){(b=b?sm:void 0)&&B("ytglobal.idbToken_",b);return b})}
;new Qf;function xm(a){if(!pk())throw a=new X("AUTH_INVALID",{dbName:a}),Bk(a),a;var b=qk();return{actualName:a+":"+b,publicName:a,userIdentifier:b}}
function ym(a,b,c,d){var e,f,g,h,k,l;return x(function(m){switch(m.h){case 1:return f=null!=(e=Error().stack)?e:"",w(m,wm(),2);case 2:g=m.i;if(!g)throw h=Pk("openDbImpl",a,b),W("ytidb_async_stack_killswitch")||(h.stack=h.stack+"\n"+f.substring(f.indexOf("\n")+1)),Bk(h),h;Dk(a);k=c?{actualName:a,publicName:a,userIdentifier:void 0}:xm(a);ya(m,3);return w(m,nm(k,g),5);case 5:return w(m,gm(k.actualName,b,d),6);case 6:return m.return(m.i);case 3:return l=Ba(m),ya(m,7),w(m,om(k.actualName,g),9);case 9:Aa(m,
8);break;case 7:Ba(m);case 8:throw l;}})}
function zm(a,b,c){c=void 0===c?{}:c;return ym(a,b,!1,c)}
function Am(a,b,c){c=void 0===c?{}:c;return ym(a,b,!0,c)}
function Bm(a,b){b=void 0===b?{}:b;var c,d;return x(function(e){if(1==e.h)return w(e,wm(),2);if(3!=e.h){c=e.i;if(!c)return e.return();Dk(a);d=xm(a);return w(e,hm(d.actualName,b),3)}return w(e,om(d.actualName,c),0)})}
function Cm(a,b,c){a=a.map(function(d){return x(function(e){return 1==e.h?w(e,hm(d.actualName,b),2):w(e,om(d.actualName,c),0)})});
return Promise.all(a).then(function(){})}
function Dm(){var a=void 0===a?{}:a;var b,c;return x(function(d){if(1==d.h)return w(d,wm(),2);if(3!=d.h){b=d.i;if(!b)return d.return();Dk("LogsDatabaseV2");return w(d,qm(b),3)}c=d.i;return w(d,Cm(c,a,b),0)})}
function Em(a,b){b=void 0===b?{}:b;var c;return x(function(d){if(1==d.h)return w(d,wm(),2);if(3!=d.h){c=d.i;if(!c)return d.return();Dk(a);return w(d,hm(a,b),3)}return w(d,om(a,c),0)})}
;function Fm(a,b){im.call(this,a,b);this.options=b;Dk(a)}
v(Fm,im);function Gm(a,b){var c;return function(){c||(c=new Fm(a,b));return c}}
Fm.prototype.i=function(a,b,c){c=void 0===c?{}:c;return(this.options.Eb?Am:zm)(a,b,Object.assign({},c))};
Fm.prototype.delete=function(a){a=void 0===a?{}:a;return(this.options.Eb?Em:Bm)(this.name,a)};
function Hm(a,b){return Gm(a,b)}
;var Im={},Jm=Hm("ytGcfConfig",{Za:(Im.coldConfigStore={eb:1},Im.hotConfigStore={eb:1},Im),Eb:!1,upgrade:function(a,b){b(1)&&(Zl(Sl(a,"hotConfigStore",{keyPath:"key",autoIncrement:!0}),"hotTimestampIndex","timestamp"),Zl(Sl(a,"coldConfigStore",{keyPath:"key",autoIncrement:!0}),"coldTimestampIndex","timestamp"))},
version:1});function Km(a){return km(Jm(),a)}
function Lm(a,b,c){var d,e,f;return x(function(g){switch(g.h){case 1:return d={config:a,hashData:b,timestamp:Y()},w(g,Km(c),2);case 2:return e=g.i,w(g,e.clear("hotConfigStore"),3);case 3:return w(g,Ul(e,"hotConfigStore",d),4);case 4:return f=g.i,g.return(f)}})}
function Mm(a,b,c,d){var e,f,g;return x(function(h){switch(h.h){case 1:return e={config:a,hashData:b,configData:c,timestamp:Y()},w(h,Km(d),2);case 2:return f=h.i,w(h,f.clear("coldConfigStore"),3);case 3:return w(h,Ul(f,"coldConfigStore",e),4);case 4:return g=h.i,h.return(g)}})}
function Nm(a){var b,c;return x(function(d){return 1==d.h?w(d,Km(a),2):3!=d.h?(b=d.i,c=void 0,w(d,Rl(b,["coldConfigStore"],{mode:"readwrite",W:!0},function(e){return dm(e.objectStore("coldConfigStore").index("coldTimestampIndex"),{direction:"prev"},function(f){c=f.la()})}),3)):d.return(c)})}
function Om(a){var b,c;return x(function(d){return 1==d.h?w(d,Km(a),2):3!=d.h?(b=d.i,c=void 0,w(d,Rl(b,["hotConfigStore"],{mode:"readwrite",W:!0},function(e){return dm(e.objectStore("hotConfigStore").index("hotTimestampIndex"),{direction:"prev"},function(f){c=f.la()})}),3)):d.return(c)})}
;function Pm(){this.h=0}
function Qm(a,b,c){var d,e,f;return x(function(g){if(1==g.h){if(!W("update_log_event_config"))return g.D(0);c&&(a.i=c,B("yt.gcf.config.hotConfigGroup",a.i));a.hotHashData=b;B("yt.gcf.config.hotHashData",a.hotHashData);return(d=vm())?c?g.D(4):w(g,Om(d),5):g.D(0)}4!=g.h&&(e=g.i,c=null==(f=e)?void 0:f.config);return w(g,Lm(c,b,d),0)})}
function Rm(a,b,c){var d,e,f,g;return x(function(h){if(1==h.h){if(!W("update_log_event_config"))return h.D(0);a.coldHashData=b;B("yt.gcf.config.coldHashData",a.coldHashData);return(d=vm())?c?h.D(4):w(h,Nm(d),5):h.D(0)}4!=h.h&&(e=h.i,c=null==(f=e)?void 0:f.config);if(!c)return h.D(0);g=c.configData;return w(h,Mm(c,b,g,d),0)})}
;function Sm(){return"INNERTUBE_API_KEY"in Xi&&"INNERTUBE_API_VERSION"in Xi}
function Tm(){return{Wc:U("INNERTUBE_API_KEY"),Xc:U("INNERTUBE_API_VERSION"),Kb:U("INNERTUBE_CONTEXT_CLIENT_CONFIG_INFO"),kc:U("INNERTUBE_CONTEXT_CLIENT_NAME","WEB"),Yc:U("INNERTUBE_CONTEXT_CLIENT_NAME",1),lc:U("INNERTUBE_CONTEXT_CLIENT_VERSION"),nc:U("INNERTUBE_CONTEXT_HL"),mc:U("INNERTUBE_CONTEXT_GL"),Zc:U("INNERTUBE_HOST_OVERRIDE")||"",bd:!!U("INNERTUBE_USE_THIRD_PARTY_AUTH",!1),ad:!!U("INNERTUBE_OMIT_API_KEY_WHEN_AUTH_HEADER_IS_PRESENT",!1),appInstallData:U("SERIALIZED_CLIENT_CONFIG_DATA")}}
function Um(a){var b={client:{hl:a.nc,gl:a.mc,clientName:a.kc,clientVersion:a.lc,configInfo:a.Kb}};navigator.userAgent&&(b.client.userAgent=String(navigator.userAgent));var c=z.devicePixelRatio;c&&1!=c&&(b.client.screenDensityFloat=String(c));c=U("EXPERIMENTS_TOKEN","");""!==c&&(b.client.experimentsToken=c);c=hj();0<c.length&&(b.request={internalExperimentFlags:c});Vm(a,void 0,b);Wm(void 0,b);Xm(void 0,b);Ym(a,void 0,b);Zm(void 0,b);W("start_sending_config_hash")&&$m(void 0,b);U("DELEGATED_SESSION_ID")&&
!W("pageid_as_header_web")&&(b.user={onBehalfOfUser:U("DELEGATED_SESSION_ID")});a=Object;c=a.assign;for(var d=b.client,e={},f=u(Object.entries(zj(U("DEVICE","")))),g=f.next();!g.done;g=f.next()){var h=u(g.value);g=h.next().value;h=h.next().value;"cbrand"===g?e.deviceMake=h:"cmodel"===g?e.deviceModel=h:"cbr"===g?e.browserName=h:"cbrver"===g?e.browserVersion=h:"cos"===g?e.osName=h:"cosver"===g?e.osVersion=h:"cplatform"===g&&(e.platform=h)}b.client=c.call(a,d,e);return b}
function an(a){var b=new xi,c=new mi;H(c,1,a.nc);H(c,2,a.mc);H(c,16,a.Yc);H(c,17,a.lc);if(a.Kb){var d=a.Kb,e=new ii;d.coldConfigData&&H(e,1,d.coldConfigData);d.appInstallData&&H(e,6,d.appInstallData);d.coldHashData&&H(e,3,d.coldHashData);d.hotHashData&&H(e,5,d.hotHashData);J(c,ii,62,e)}if((d=z.devicePixelRatio)&&1!=d){if(null!=d&&"number"!==typeof d)throw Error("Value of float/double field must be a number|null|undefined, found "+typeof d+": "+d);H(c,65,d)}d=U("EXPERIMENTS_TOKEN","");""!==d&&H(c,
54,d);d=hj();if(0<d.length){e=new qi;for(var f=0;f<d.length;f++){var g=new oi;H(g,1,d[f].key);Yc(g,2,pi,d[f].value);gd(e,15,oi,g)}J(b,qi,5,e)}Vm(a,c);Wm(b);Xm(c);Ym(a,c);Zm(c);W("start_sending_config_hash")&&$m(c);U("DELEGATED_SESSION_ID")&&!W("pageid_as_header_web")&&(a=new vi,H(a,3,U("DELEGATED_SESSION_ID")));a=u(Object.entries(zj(U("DEVICE",""))));for(d=a.next();!d.done;d=a.next())e=u(d.value),d=e.next().value,e=e.next().value,"cbrand"===d?H(c,12,e):"cmodel"===d?H(c,13,e):"cbr"===d?H(c,87,e):"cbrver"===
d?H(c,88,e):"cos"===d?H(c,18,e):"cosver"===d?H(c,19,e):"cplatform"===d&&H(c,42,e);J(b,mi,1,c);return b}
function Vm(a,b,c){a=a.kc;if("WEB"===a||"MWEB"===a||1===a||2===a)if(b){c=$c(b,ji,96)||new ji;var d=ik();d=Object.keys(zi).indexOf(d);d=-1===d?null:d;null!==d&&H(c,3,d);J(b,ji,96,c)}else c&&(c.client.mainAppWebInfo=null!=(d=c.client.mainAppWebInfo)?d:{},c.client.mainAppWebInfo.webDisplayMode=ik())}
function Wm(a,b){var c=C("yt.embedded_player.embed_url");c&&(a?(b=$c(a,si,7)||new si,H(b,4,c),J(a,si,7,b)):b&&(b.thirdParty={embedUrl:c}))}
function Xm(a,b){var c;if(W("web_log_memory_total_kbytes")&&(null==(c=z.navigator)?0:c.deviceMemory)){var d;c=null==(d=z.navigator)?void 0:d.deviceMemory;a?H(a,95,1E6*c):b&&(b.client.memoryTotalKbytes=""+1E6*c)}}
function Ym(a,b,c){if(a.appInstallData)if(b){var d;c=null!=(d=$c(b,ii,62))?d:new ii;H(c,6,a.appInstallData);J(b,ii,62,c)}else c&&(c.client.configInfo=c.client.configInfo||{},c.client.configInfo.appInstallData=a.appInstallData)}
function Zm(a,b){a:{var c=nk();if(c){var d=jk[c.type||"unknown"]||"CONN_UNKNOWN";c=jk[c.effectiveType||"unknown"]||"CONN_UNKNOWN";"CONN_CELLULAR_UNKNOWN"===d&&"CONN_UNKNOWN"!==c&&(d=c);if("CONN_UNKNOWN"!==d)break a;if("CONN_UNKNOWN"!==c){d=c;break a}}d=void 0}d&&(a?H(a,61,kk[d]):b&&(b.client.connectionType=d));W("web_log_effective_connection_type")&&(d=nk(),d=null!=d&&d.effectiveType?mk.hasOwnProperty(d.effectiveType)?mk[d.effectiveType]:"EFFECTIVE_CONNECTION_TYPE_UNKNOWN":void 0,d&&(a?H(a,94,lk[d]):
b&&(b.client.effectiveConnectionType=d)))}
function bn(a,b,c){c=void 0===c?{}:c;var d={};U("EOM_VISITOR_DATA")?d={"X-Goog-EOM-Visitor-Id":U("EOM_VISITOR_DATA")}:d={"X-Goog-Visitor-Id":c.visitorData||U("VISITOR_DATA","")};if(b&&b.includes("www.youtube-nocookie.com"))return d;b=c.Kd||U("AUTHORIZATION");if(!b)if(a)b="Bearer "+C("gapi.auth.getToken")().Jd;else{hk.h||(hk.h=new hk);a={};if(c=ee([]))a.Authorization=c,c=void 0,void 0===c&&(c=Number(U("SESSION_INDEX",0)),c=isNaN(c)?0:c),W("voice_search_auth_header_removal")||(a["X-Goog-AuthUser"]=
c.toString()),"INNERTUBE_HOST_OVERRIDE"in Xi||(a["X-Origin"]=window.location.origin),"DELEGATED_SESSION_ID"in Xi&&(a["X-Goog-PageId"]=U("DELEGATED_SESSION_ID"));W("pageid_as_header_web")||delete a["X-Goog-PageId"];d=Object.assign({},d,a)}b&&(d.Authorization=b);return d}
function $m(a,b){Pm.h||(Pm.h=new Pm);var c=Pm.h;var d=Y()-c.h;if(0!==c.h&&d<gj("send_config_hash_timer"))c=void 0;else{d=C("yt.gcf.config.coldConfigData");var e=C("yt.gcf.config.hotHashData"),f=C("yt.gcf.config.coldHashData");d&&e&&f&&(c.h=Y());c={coldConfigData:d,hotHashData:e,coldHashData:f}}if(e=c)if(c=e.coldConfigData,d=e.coldHashData,e=e.hotHashData,c&&d&&e)if(a){var g;b=null!=(g=$c(a,ii,62))?g:new ii;H(b,1,c);H(b,3,d);H(b,5,e);J(a,ii,62,b)}else b&&(b.client.configInfo=b.client.configInfo||{},
b.client.configInfo.coldConfigData=c,b.client.configInfo.coldHashData=d,b.client.configInfo.hotHashData=e)}
;var cn=C("ytPubsub2Pubsub2Instance")||new L;L.prototype.subscribe=L.prototype.subscribe;L.prototype.unsubscribeByKey=L.prototype.nb;L.prototype.publish=L.prototype.ab;L.prototype.clear=L.prototype.clear;B("ytPubsub2Pubsub2Instance",cn);B("ytPubsub2Pubsub2SubscribedKeys",C("ytPubsub2Pubsub2SubscribedKeys")||{});B("ytPubsub2Pubsub2TopicToKeys",C("ytPubsub2Pubsub2TopicToKeys")||{});B("ytPubsub2Pubsub2IsAsync",C("ytPubsub2Pubsub2IsAsync")||{});B("ytPubsub2Pubsub2SkipSubKey",null);
function dn(a,b){var c=C("ytPubsub2Pubsub2Instance");c&&c.publish.call(c,a.toString(),a,b)}
;var en=gj("max_body_size_to_compress",5E5),fn=gj("min_body_size_to_compress",500),gn=!0,hn=gj("compression_performance_threshold",250);
function jn(a,b,c,d){var e={startTime:Y()};if(gn)try{var f=(new Blob(b.split(""))).size;if(!(f>en||f<fn)){var g=ge(b);var h=h||{};h.Vc=!0;var k=new di(h);k.push(g,!0);if(k.err)throw k.msg||hh[k.err];var l=k.result;e.endTime=Y();W("disable_compression_due_to_performance_degredation")&&e.endTime-e.startTime>=hn&&(gn=!1);!W("gel_compression_csi_killswitch")&&W("log_gel_compression_latency")&&.01>=Math.random()&&dn("gel_compression_latency_payload",e);if(window.Blob){var m=l.length<(new Blob(b.split(""))).size;
m||dj(new ok("Compressed req body is larger than uncompressed","original size: "+(new Blob(b.split(""))).size,"compressed size: "+l.length));var n=m}else n=!0;if(n||!W("only_compress_gel_if_smaller"))c.headers||(c.headers={}),c.headers["Content-Encoding"]="gzip",c.postBody=l,c.postParams=void 0}d(a,c)}catch(r){ej(r),d(a,c)}else d(a,c)}
;function kn(a){a=Object.assign({},a);delete a.Authorization;var b=ee();if(b){var c=new Bf;c.update(U("INNERTUBE_API_KEY"));c.update(b);a.hash=zc(c.digest(),3)}return a}
;var ln;function mn(){ln||(ln=new wk("yt.innertube"));return ln}
function nn(a,b,c,d){if(d)return null;d=mn().get("nextId",!0)||1;var e=mn().get("requests",!0)||{};e[d]={method:a,request:b,authState:kn(c),requestTime:Math.round(Y())};mn().set("nextId",d+1,86400,!0);mn().set("requests",e,86400,!0);return d}
function on(a){var b=mn().get("requests",!0)||{};delete b[a];mn().set("requests",b,86400,!0)}
function pn(a){var b=mn().get("requests",!0);if(b){for(var c in b){var d=b[c];if(!(6E4>Math.round(Y())-d.requestTime)){var e=d.authState,f=kn(bn(!1));pb(e,f)&&(e=d.request,"requestTimeMs"in e&&(e.requestTimeMs=Math.round(Y())),qn(a,d.method,e,{}));delete b[c]}}mn().set("requests",b,86400,!0)}}
;function rn(a){this.rb=this.h=!1;this.potentialEsfErrorCounter=this.i=0;this.handleError=function(){};
this.Ta=function(){};
this.now=Date.now;this.hb=!1;var b;this.Bc=null!=(b=a.Bc)?b:100;var c;this.zc=null!=(c=a.zc)?c:1;var d;this.xc=null!=(d=a.xc)?d:2592E6;var e;this.wc=null!=(e=a.wc)?e:12E4;var f;this.yc=null!=(f=a.yc)?f:5E3;var g;this.I=null!=(g=a.I)?g:void 0;this.wb=!!a.wb;var h;this.ub=null!=(h=a.ub)?h:.1;var k;this.Bb=null!=(k=a.Bb)?k:10;a.handleError&&(this.handleError=a.handleError);a.Ta&&(this.Ta=a.Ta);a.hb&&(this.hb=a.hb);a.rb&&(this.rb=a.rb);this.K=a.K;this.ia=a.ia;this.O=a.O;this.R=a.R;this.xa=a.xa;this.Sb=
a.Sb;this.Rb=a.Rb;sn(this)&&(!this.K||this.K("networkless_logging"))&&tn(this)}
function tn(a){sn(a)&&!a.hb&&(a.h=!0,a.wb&&Math.random()<=a.ub&&a.O.Lc(a.I),un(a),a.R.da()&&a.mb(),a.R.Ka(a.Sb,a.mb.bind(a)),a.R.Ka(a.Rb,a.Zb.bind(a)))}
q=rn.prototype;q.writeThenSend=function(a,b){var c=this;b=void 0===b?{}:b;if(sn(this)&&this.h){var d={url:a,options:b,timestamp:this.now(),status:"NEW",sendCount:0};this.O.set(d,this.I).then(function(e){d.id=e;c.R.da()&&vn(c,d)}).catch(function(e){vn(c,d);
wn(c,e)})}else this.xa(a,b)};
q.sendThenWrite=function(a,b,c){var d=this;b=void 0===b?{}:b;if(sn(this)&&this.h){var e={url:a,options:b,timestamp:this.now(),status:"NEW",sendCount:0};this.K&&this.K("nwl_skip_retry")&&(e.skipRetry=c);if(this.R.da()||this.K&&this.K("nwl_aggressive_send_then_write")&&!e.skipRetry){if(!e.skipRetry){var f=b.onError?b.onError:function(){};
b.onError=function(g,h){return x(function(k){if(1==k.h)return w(k,d.O.set(e,d.I).catch(function(l){wn(d,l)}),2);
f(g,h);k.h=0})}}this.xa(a,b,e.skipRetry)}else this.O.set(e,this.I).catch(function(g){d.xa(a,b,e.skipRetry);
wn(d,g)})}else this.xa(a,b,this.K&&this.K("nwl_skip_retry")&&c)};
q.sendAndWrite=function(a,b){var c=this;b=void 0===b?{}:b;if(sn(this)&&this.h){var d={url:a,options:b,timestamp:this.now(),status:"NEW",sendCount:0},e=!1,f=b.onSuccess?b.onSuccess:function(){};
d.options.onSuccess=function(g,h){void 0!==d.id?c.O.Ra(d.id,c.I):e=!0;c.R.Ma&&c.K&&c.K("vss_network_hint")&&c.R.Ma(!0);f(g,h)};
this.xa(d.url,d.options);this.O.set(d,this.I).then(function(g){d.id=g;e&&c.O.Ra(d.id,c.I)}).catch(function(g){wn(c,g)})}else this.xa(a,b)};
q.mb=function(){var a=this;if(!sn(this))throw Pk("throttleSend");this.i||(this.i=this.ia.ka(function(){var b;return x(function(c){if(1==c.h)return w(c,a.O.hc("NEW",a.I),2);if(3!=c.h)return b=c.i,b?w(c,vn(a,b),3):(a.Zb(),c.return());a.i&&(a.i=0,a.mb());c.h=0})},this.Bc))};
q.Zb=function(){this.ia.Qa(this.i);this.i=0};
function vn(a,b){var c,d;return x(function(e){switch(e.h){case 1:if(!sn(a))throw c=Pk("immediateSend"),c;if(void 0===b.id){e.D(2);break}return w(e,a.O.dd(b.id,a.I),3);case 3:(d=e.i)||a.Ta(Error("The request cannot be found in the database."));case 2:if(xn(a,b,a.xc)){e.D(4);break}a.Ta(Error("Networkless Logging: Stored logs request expired age limit"));if(void 0===b.id){e.D(5);break}return w(e,a.O.Ra(b.id,a.I),5);case 5:return e.return();case 4:b.skipRetry||(b=yn(a,b));if(!b){e.D(0);break}if(!b.skipRetry||
void 0===b.id){e.D(8);break}return w(e,a.O.Ra(b.id,a.I),8);case 8:a.xa(b.url,b.options,!!b.skipRetry),e.h=0}})}
function yn(a,b){if(!sn(a))throw Pk("updateRequestHandlers");var c=b.options.onError?b.options.onError:function(){};
b.options.onError=function(e,f){var g,h,k,l;return x(function(m){switch(m.h){case 1:g=zn(f);h=An(f);if(!(a.K&&a.K("nwl_consider_error_code")&&g||a.K&&!a.K("nwl_consider_error_code")&&a.potentialEsfErrorCounter<=a.Bb)){m.D(2);break}if(!a.R.Db){m.D(3);break}return w(m,a.R.Db(),3);case 3:if(a.R.da()){m.D(2);break}c(e,f);if(!a.K||!a.K("nwl_consider_error_code")||void 0===(null==(k=b)?void 0:k.id)){m.D(6);break}return w(m,a.O.Tb(b.id,a.I,!1),6);case 6:return m.return();case 2:if(a.K&&a.K("nwl_consider_error_code")&&
!g&&a.potentialEsfErrorCounter>a.Bb)return m.return();a.potentialEsfErrorCounter++;if(void 0===(null==(l=b)?void 0:l.id)){m.D(8);break}return b.sendCount<a.zc?w(m,a.O.Tb(b.id,a.I,!0,h?!1:void 0),12):w(m,a.O.Ra(b.id,a.I),8);case 12:a.ia.ka(function(){a.R.da()&&a.mb()},a.yc);
case 8:c(e,f),m.h=0}})};
var d=b.options.onSuccess?b.options.onSuccess:function(){};
b.options.onSuccess=function(e,f){var g;return x(function(h){if(1==h.h)return void 0===(null==(g=b)?void 0:g.id)?h.D(2):w(h,a.O.Ra(b.id,a.I),2);a.R.Ma&&a.K&&a.K("vss_network_hint")&&a.R.Ma(!0);d(e,f);h.h=0})};
return b}
function xn(a,b,c){b=b.timestamp;return a.now()-b>=c?!1:!0}
function un(a){if(!sn(a))throw Pk("retryQueuedRequests");a.O.hc("QUEUED",a.I).then(function(b){b&&!xn(a,b,a.wc)?a.ia.ka(function(){return x(function(c){if(1==c.h)return void 0===b.id?c.D(2):w(c,a.O.Tb(b.id,a.I),2);un(a);c.h=0})}):a.R.da()&&a.mb()})}
function wn(a,b){a.Ec&&!a.R.da()?a.Ec(b):a.handleError(b)}
function sn(a){return!!a.I||a.rb}
function zn(a){var b;return(a=null==a?void 0:null==(b=a.error)?void 0:b.code)&&400<=a&&599>=a?!1:!0}
function An(a){var b;a=null==a?void 0:null==(b=a.error)?void 0:b.code;return!(400!==a&&415!==a)}
;var Bn;
function Cn(){if(Bn)return Bn();var a={};Bn=Hm("LogsDatabaseV2",{Za:(a.LogsRequestsStore={eb:2},a),Eb:!1,upgrade:function(b,c,d){c(2)&&Sl(b,"LogsRequestsStore",{keyPath:"id",autoIncrement:!0});c(3);c(5)&&(d=d.objectStore("LogsRequestsStore"),d.h.indexNames.contains("newRequest")&&d.h.deleteIndex("newRequest"),Zl(d,"newRequestV2",["status","interface","timestamp"]));c(7)&&b.h.objectStoreNames.contains("sapisid")&&b.h.deleteObjectStore("sapisid");c(9)&&b.h.objectStoreNames.contains("SWHealthLog")&&b.h.deleteObjectStore("SWHealthLog")},
version:9});return Bn()}
;function Dn(a){return km(Cn(),a)}
function En(a,b){var c,d,e,f;return x(function(g){if(1==g.h)return c={startTime:Y(),transactionType:"YT_IDB_TRANSACTION_TYPE_WRITE"},w(g,Dn(b),2);if(3!=g.h)return d=g.i,e=Object.assign({},a,{options:JSON.parse(JSON.stringify(a.options)),interface:U("INNERTUBE_CONTEXT_CLIENT_NAME",0)}),w(g,Ul(d,"LogsRequestsStore",e),3);f=g.i;c.od=Y();Fn(c);return g.return(f)})}
function Gn(a,b){var c,d,e,f,g,h,k;return x(function(l){if(1==l.h)return c={startTime:Y(),transactionType:"YT_IDB_TRANSACTION_TYPE_READ"},w(l,Dn(b),2);if(3!=l.h)return d=l.i,e=U("INNERTUBE_CONTEXT_CLIENT_NAME",0),f=[a,e,0],g=[a,e,Y()],h=IDBKeyRange.bound(f,g),k=void 0,w(l,Rl(d,["LogsRequestsStore"],{mode:"readwrite",W:!0},function(m){return dm(m.objectStore("LogsRequestsStore").index("newRequestV2"),{query:h,direction:"prev"},function(n){n.la()&&(k=n.la(),"NEW"===a&&(k.status="QUEUED",n.update(k)))})}),
3);
c.od=Y();Fn(c);return l.return(k)})}
function Hn(a,b){var c;return x(function(d){if(1==d.h)return w(d,Dn(b),2);c=d.i;return d.return(Rl(c,["LogsRequestsStore"],{mode:"readwrite",W:!0},function(e){var f=e.objectStore("LogsRequestsStore");return f.get(a).then(function(g){if(g)return g.status="QUEUED",$k(f.h.put(g,void 0)).then(function(){return g})})}))})}
function In(a,b,c,d){c=void 0===c?!0:c;var e;return x(function(f){if(1==f.h)return w(f,Dn(b),2);e=f.i;return f.return(Rl(e,["LogsRequestsStore"],{mode:"readwrite",W:!0},function(g){var h=g.objectStore("LogsRequestsStore");return h.get(a).then(function(k){return k?(k.status="NEW",c&&(k.sendCount+=1),void 0!==d&&(k.options.compress=d),$k(h.h.put(k,void 0)).then(function(){return k})):Tk.resolve(void 0)})}))})}
function Jn(a,b){var c;return x(function(d){if(1==d.h)return w(d,Dn(b),2);c=d.i;return d.return(c.delete("LogsRequestsStore",a))})}
function Kn(a){var b,c;return x(function(d){if(1==d.h)return w(d,Dn(a),2);b=d.i;c=Y()-2592E6;return w(d,Rl(b,["LogsRequestsStore"],{mode:"readwrite",W:!0},function(e){return am(e.objectStore("LogsRequestsStore"),{},function(f){if(f.la().timestamp<=c)return f.delete().then(function(){return f.continue()})})}),0)})}
function Ln(){x(function(a){return w(a,Dm(),0)})}
function Fn(a){W("nwl_csi_killswitch")||.01>=Math.random()&&dn("nwl_transaction_latency_payload",a)}
;var Mn={},Nn=Hm("ServiceWorkerLogsDatabase",{Za:(Mn.SWHealthLog={eb:1},Mn),Eb:!0,upgrade:function(a,b){b(1)&&Zl(Sl(a,"SWHealthLog",{keyPath:"id",autoIncrement:!0}),"swHealthNewRequest",["interface","timestamp"])},
version:1});function On(a){return km(Nn(),a)}
function Pn(a){var b,c;x(function(d){if(1==d.h)return w(d,On(a),2);b=d.i;c=Y()-2592E6;return w(d,Rl(b,["SWHealthLog"],{mode:"readwrite",W:!0},function(e){return am(e.objectStore("SWHealthLog"),{},function(f){if(f.la().timestamp<=c)return f.delete().then(function(){return f.continue()})})}),0)})}
function Qn(a){var b;return x(function(c){if(1==c.h)return w(c,On(a),2);b=c.i;return w(c,b.clear("SWHealthLog"),0)})}
;var Rn={},Sn=0;function Tn(a){var b=new Image,c=""+Sn++;Rn[c]=b;b.onload=b.onerror=function(){delete Rn[c]};
b.src=a}
;function Un(){this.h=new Map;this.i=!1}
function Vn(){if(!Un.h){var a=C("yt.networkRequestMonitor.instance")||new Un;B("yt.networkRequestMonitor.instance",a);Un.h=a}return Un.h}
Un.prototype.requestComplete=function(a,b){b&&(this.i=!0);a=this.removeParams(a);this.h.get(a)||this.h.set(a,b)};
Un.prototype.isEndpointCFR=function(a){a=this.removeParams(a);return(a=this.h.get(a))?!1:!1===a&&this.i?!0:null};
Un.prototype.removeParams=function(a){return a.split("?")[0]};
Un.prototype.removeParams=Un.prototype.removeParams;Un.prototype.isEndpointCFR=Un.prototype.isEndpointCFR;Un.prototype.requestComplete=Un.prototype.requestComplete;Un.getInstance=Vn;var Wn;function Xn(){Wn||(Wn=new wk("yt.offline"));return Wn}
function Yn(a){if(W("offline_error_handling")){var b=Xn().get("errors",!0)||{};b[a.message]={name:a.name,stack:a.stack};a.level&&(b[a.message].level=a.level);Xn().set("errors",b,2592E3,!0)}}
;function Z(){Je.call(this);var a=this;this.m=!1;this.i=Pe();this.i.Ka("networkstatus-online",function(){if(a.m&&W("offline_error_handling")){var b=Xn().get("errors",!0);if(b){for(var c in b)if(b[c]){var d=new ok(c,"sent via offline_errors");d.name=b[c].name;d.stack=b[c].stack;d.level=b[c].level;dj(d)}Xn().set("errors",{},2592E3,!0)}}})}
v(Z,Je);function Zn(){if(!Z.h){var a=C("yt.networkStatusManager.instance")||new Z;B("yt.networkStatusManager.instance",a);Z.h=a}return Z.h}
q=Z.prototype;q.da=function(){return this.i.da()};
q.Ma=function(a){this.i.i=a};
q.Tc=function(){var a=window.navigator.onLine;return void 0===a?!0:a};
q.Oc=function(){this.m=!0};
q.Ka=function(a,b){return this.i.Ka(a,b)};
q.Db=function(a){a=Ne(this.i,a);a.then(function(b){W("use_cfr_monitor")&&Vn().requestComplete("generate_204",b)});
return a};
Z.prototype.sendNetworkCheckRequest=Z.prototype.Db;Z.prototype.listen=Z.prototype.Ka;Z.prototype.enableErrorFlushing=Z.prototype.Oc;Z.prototype.getWindowStatus=Z.prototype.Tc;Z.prototype.networkStatusHint=Z.prototype.Ma;Z.prototype.isNetworkAvailable=Z.prototype.da;Z.getInstance=Zn;function $n(a){a=void 0===a?{}:a;Je.call(this);var b=this;this.i=this.H=0;this.m=Zn();var c=C("yt.networkStatusManager.instance.listen").bind(this.m);c&&(a.Cb?(this.Cb=a.Cb,c("networkstatus-online",function(){ao(b,"publicytnetworkstatus-online")}),c("networkstatus-offline",function(){ao(b,"publicytnetworkstatus-offline")})):(c("networkstatus-online",function(){Ke(b,"publicytnetworkstatus-online")}),c("networkstatus-offline",function(){Ke(b,"publicytnetworkstatus-offline")})))}
v($n,Je);$n.prototype.da=function(){var a=C("yt.networkStatusManager.instance.isNetworkAvailable");return a?a.bind(this.m)():!0};
$n.prototype.Ma=function(a){var b=C("yt.networkStatusManager.instance.networkStatusHint").bind(this.m);b&&b(a)};
$n.prototype.Db=function(a){var b=this,c;return x(function(d){c=C("yt.networkStatusManager.instance.sendNetworkCheckRequest").bind(b.m);return W("skip_network_check_if_cfr")&&Vn().isEndpointCFR("generate_204")?d.return(new Promise(function(e){var f;b.Ma((null==(f=window.navigator)?void 0:f.onLine)||!0);e(b.da())})):c?d.return(c(a)):d.return(!0)})};
function ao(a,b){a.Cb?a.i?(Qe.Qa(a.H),a.H=Qe.ka(function(){a.s!==b&&(Ke(a,b),a.s=b,a.i=Y())},a.Cb-(Y()-a.i))):(Ke(a,b),a.s=b,a.i=Y()):Ke(a,b)}
;var bo;function co(){var a=rn.call;bo||(bo=new $n({Od:!0,Nd:!0}));a.call(rn,this,{O:{Lc:Kn,Ra:Jn,hc:Gn,dd:Hn,Tb:In,set:En},R:bo,handleError:dj,Ta:ej,xa:eo,now:Y,Ec:Yn,ia:vk(),Sb:"publicytnetworkstatus-online",Rb:"publicytnetworkstatus-offline",wb:!0,ub:.1,Bb:gj("potential_esf_error_limit",10),K:W,hb:!(pk()&&"www.youtube-nocookie.com"!==Ub(document.location.toString()))});this.j=new Qf;W("networkless_immediately_drop_all_requests")&&Ln();Em("LogsDatabaseV2")}
v(co,rn);function fo(){var a=C("yt.networklessRequestController.instance");a||(a=new co,B("yt.networklessRequestController.instance",a),W("networkless_logging")&&wm().then(function(b){a.I=b;tn(a);a.j.resolve();a.wb&&Math.random()<=a.ub&&a.I&&Pn(a.I);W("networkless_immediately_drop_sw_health_store")&&go(a)}));
return a}
co.prototype.writeThenSend=function(a,b){b||(b={});pk()||(this.h=!1);rn.prototype.writeThenSend.call(this,a,b)};
co.prototype.sendThenWrite=function(a,b,c){b||(b={});pk()||(this.h=!1);rn.prototype.sendThenWrite.call(this,a,b,c)};
co.prototype.sendAndWrite=function(a,b){b||(b={});pk()||(this.h=!1);rn.prototype.sendAndWrite.call(this,a,b)};
co.prototype.awaitInitialization=function(){return this.j.promise};
function go(a){var b;x(function(c){if(!a.I)throw b=Pk("clearSWHealthLogsDb"),b;return c.return(Qn(a.I).catch(function(d){a.handleError(d)}))})}
function eo(a,b,c){W("use_cfr_monitor")&&ho(a,b);if(W("use_request_time_ms_header"))b.headers&&(b.headers["X-Goog-Request-Time"]=JSON.stringify(Math.round(Y())));else{var d;if(null==(d=b.postParams)?0:d.requestTimeMs)b.postParams.requestTimeMs=Math.round(Y())}if(c&&0===Object.keys(b).length){var e=void 0===e?"":e;var f=void 0===f?!1:f;if(a)if(e)Oj(a,void 0,"POST",e);else if(U("USE_NET_AJAX_FOR_PING_TRANSPORT",!1))Oj(a,void 0,"GET","",void 0,void 0,f);else{b:{try{var g=new bb({url:a});if(g.j&&g.i||
g.m){var h=Tb(a.match(Sb)[5]||null);var k=!(!h||!h.endsWith("/aclk")||"1"!==Zb(a,"ri"));break b}}catch(m){}k=!1}if(k){b:{try{if(window.navigator&&window.navigator.sendBeacon&&window.navigator.sendBeacon(a,"")){var l=!0;break b}}catch(m){}l=!1}c=l?!0:!1}else c=!1;c||Tn(a)}}else b.compress?b.postBody?("string"!==typeof b.postBody&&(b.postBody=JSON.stringify(b.postBody)),jn(a,b.postBody,b,Lj)):jn(a,JSON.stringify(b.postParams),b,Tj):Lj(a,b)}
function ho(a,b){var c=b.onError?b.onError:function(){};
b.onError=function(e,f){Vn().requestComplete(a,!1);c(e,f)};
var d=b.onSuccess?b.onSuccess:function(){};
b.onSuccess=function(e,f){Vn().requestComplete(a,!0);d(e,f)}}
;var io=z.ytNetworklessLoggingInitializationOptions||{isNwlInitialized:!1};B("ytNetworklessLoggingInitializationOptions",io);function jo(a){var b=this;this.config_=null;a?this.config_=a:Sm()&&(this.config_=Tm());sk(function(){pn(b)},5E3)}
jo.prototype.isReady=function(){!this.config_&&Sm()&&(this.config_=Tm());return!!this.config_};
function qn(a,b,c,d){function e(y){y=void 0===y?!1:y;var A;if(d.retry&&"www.youtube-nocookie.com"!=h&&(y||W("skip_ls_gel_retry")||"application/json"!==g.headers["Content-Type"]||(A=nn(b,c,l,k)),A)){var I=g.onSuccess,N=g.onFetchSuccess;g.onSuccess=function(P,wa){on(A);I(P,wa)};
c.onFetchSuccess=function(P,wa){on(A);N(P,wa)}}try{if(y&&d.retry&&!d.sc.bypassNetworkless)g.method="POST",d.sc.writeThenSend?fo().writeThenSend(p,g):fo().sendAndWrite(p,g);
else if(d.compress)if(g.postBody){var T=g.postBody;"string"!==typeof T&&(T=JSON.stringify(g.postBody));jn(p,T,g,Lj)}else jn(p,JSON.stringify(g.postParams),g,Tj);else W("web_all_payloads_via_jspb")?Lj(p,g):Tj(p,g)}catch(P){if("InvalidAccessError"==P.name)A&&(on(A),A=0),ej(Error("An extension is blocking network request."));else throw P;}A&&sk(function(){pn(a)},5E3)}
!U("VISITOR_DATA")&&"visitor_id"!==b&&.01>Math.random()&&ej(new ok("Missing VISITOR_DATA when sending innertube request.",b,c,d));if(!a.isReady()){var f=new ok("innertube xhrclient not ready",b,c,d);dj(f);throw f;}var g={headers:d.headers||{},method:"POST",postParams:c,postBody:d.postBody,postBodyFormat:d.postBodyFormat||"JSON",onTimeout:function(){d.onTimeout()},
onFetchTimeout:d.onTimeout,onSuccess:function(y,A){if(d.onSuccess)d.onSuccess(A)},
onFetchSuccess:function(y){if(d.onSuccess)d.onSuccess(y)},
onError:function(y,A){if(d.onError)d.onError(A)},
onFetchError:function(y){if(d.onError)d.onError(y)},
timeout:d.timeout,withCredentials:!0,compress:d.compress};g.headers["Content-Type"]||(g.headers["Content-Type"]="application/json");var h="";(f=a.config_.Zc)&&(h=f);var k=a.config_.bd||!1,l=bn(k,h,d);Object.assign(g.headers,l);(f=g.headers.Authorization)&&!h&&k&&(g.headers["x-origin"]=window.location.origin);var m="/youtubei/"+a.config_.Xc+"/"+b,n={alt:"json"},r=a.config_.ad&&f;r=r&&f.startsWith("Bearer");r||(n.key=a.config_.Wc);var p=Aj(""+h+m,n||{},!0);C("ytNetworklessLoggingInitializationOptions")&&
io.isNwlInitialized?um().then(function(y){e(y)}):e(!1)}
;function ko(a){this.X=a;this.h=null;this.s=0;this.N=null;this.H=0;this.i=[];for(a=0;4>a;a++)this.i.push(0);this.m=0;this.Fb=qj(window,"mousemove",Za(this.Gb,this));this.Hb=sj(Za(this.Ga,this),25)}
D(ko,he);ko.prototype.Gb=function(a){void 0===a.h&&lj(a);var b=a.h;void 0===a.i&&lj(a);this.h=new xd(b,a.i)};
ko.prototype.Ga=function(){if(this.h){var a=Y();if(0!=this.s){var b=this.N,c=this.h,d=b.x-c.x;b=b.y-c.y;d=Math.sqrt(d*d+b*b)/(a-this.s);this.i[this.m]=.5<Math.abs((d-this.H)/this.H)?1:0;for(c=b=0;4>c;c++)b+=this.i[c]||0;3<=b&&this.X();this.H=d}this.s=a;this.N=this.h;this.m=(this.m+1)%4}};
ko.prototype.Sa=function(){window.clearInterval(this.Hb);oj(this.Fb)};var lo={};
function mo(){var a={},b=void 0===a.kd?!1:a.kd;a=void 0===a.Pc?!0:a.Pc;if(null==C("_lact",window)){var c=parseInt(U("LACT"),10);c=isFinite(c)?Date.now()-Math.max(c,0):-1;B("_lact",c,window);B("_fact",c,window);-1==c&&no();qj(document,"keydown",no);qj(document,"keyup",no);qj(document,"mousedown",no);qj(document,"mouseup",no);b?qj(window,"touchmove",function(){oo("touchmove",200)},{passive:!0}):(qj(window,"resize",function(){oo("resize",200)}),a&&qj(window,"scroll",function(){oo("scroll",200)}));
new ko(function(){oo("mouse",100)});
qj(document,"touchstart",no,{passive:!0});qj(document,"touchend",no,{passive:!0})}}
function oo(a,b){lo[a]||(lo[a]=!0,Qe.ka(function(){no();lo[a]=!1},b))}
function no(){null==C("_lact",window)&&mo();var a=Date.now();B("_lact",a,window);-1==C("_fact",window)&&B("_fact",a,window);(a=C("ytglobal.ytUtilActivityCallback_"))&&a()}
function po(){var a=C("_lact",window);return null==a?-1:Math.max(Date.now()-a,0)}
;var qo=z.ytPubsubPubsubInstance||new L,ro=z.ytPubsubPubsubSubscribedKeys||{},so=z.ytPubsubPubsubTopicToKeys||{},to=z.ytPubsubPubsubIsSynchronous||{};L.prototype.subscribe=L.prototype.subscribe;L.prototype.unsubscribeByKey=L.prototype.nb;L.prototype.publish=L.prototype.ab;L.prototype.clear=L.prototype.clear;B("ytPubsubPubsubInstance",qo);B("ytPubsubPubsubTopicToKeys",so);B("ytPubsubPubsubIsSynchronous",to);B("ytPubsubPubsubSubscribedKeys",ro);var uo=Symbol("injectionDeps");function vo(){this.key=Pm}
function wo(){this.i=new Map;this.h=new Map}
wo.prototype.resolve=function(a){return a instanceof vo?xo(this,a.key,[],!0):xo(this,a,[])};
function xo(a,b,c,d){d=void 0===d?!1:d;if(-1<c.indexOf(b))throw Error("Deps cycle for: "+b);if(a.h.has(b))return a.h.get(b);if(!a.i.has(b)){if(d)return;throw Error("No provider for: "+b);}d=a.i.get(b);c.push(b);if(d.rd)var e=d.rd;else if(d.qd)e=d[uo]?yo(a,d[uo],c):[],e=d.qd.apply(d,ka(e));else if(d.pd){e=d.pd;var f=e[uo]?yo(a,e[uo],c):[];e=new (Function.prototype.bind.apply(e,[null].concat(ka(f))))}else throw Error("Could not resolve providers for: "+b);c.pop();d.Ud||a.h.set(b,e);return e}
function yo(a,b,c){return b?b.map(function(d){return d instanceof vo?xo(a,d.key,c,!0):xo(a,d,c)}):[]}
;var zo;function Ao(){this.store={};this.h={}}
Ao.prototype.storePayload=function(a,b){a=Bo(a);this.store[a]?this.store[a].push(b):(this.h={},this.store[a]=[b]);return a};
Ao.prototype.extractMatchingEntries=function(a){a=Co(this,a);for(var b=[],c=0;c<a.length;c++)this.store[a[c]]&&(b.push.apply(b,ka(this.store[a[c]])),delete this.store[a[c]]);return b};
Ao.prototype.getSequenceCount=function(a){a=Co(this,a);for(var b=0,c=0;c<a.length;c++)b+=this.store[a[c]].length||0;return b};
function Co(a,b){var c=Bo(b);if(a.h[c])return a.h[c];var d=Object.keys(a.store)||[];if(1>=d.length&&Bo(b)===d[0])return d;for(var e=[],f=0;f<d.length;f++){var g=d[f].split("/");if(Do(b.auth,g[0])){var h=b.isJspb;Do(void 0===h?"undefined":h?"true":"false",g[1])&&Do(b.cttAuthInfo,g[2])&&e.push(d[f])}}return a.h[c]=e}
function Do(a,b){return void 0===a||"undefined"===a?!0:a===b}
Ao.prototype.getSequenceCount=Ao.prototype.getSequenceCount;Ao.prototype.extractMatchingEntries=Ao.prototype.extractMatchingEntries;Ao.prototype.storePayload=Ao.prototype.storePayload;function Bo(a){return[void 0===a.auth?"undefined":a.auth,void 0===a.isJspb?"undefined":a.isJspb,void 0===a.cttAuthInfo?"undefined":a.cttAuthInfo].join("/")}
;var Eo=gj("initial_gel_batch_timeout",2E3),Fo=gj("gel_queue_timeout_max_ms",6E4),Go=Math.pow(2,16)-1,Ho=void 0;function Io(){this.j=this.h=this.i=0}
var Jo=new Io,Ko=new Io,Lo,Mo=!0,No=z.ytLoggingTransportTokensToCttTargetIds_||{};B("ytLoggingTransportTokensToCttTargetIds_",No);var Oo=z.ytLoggingTransportTokensToJspbCttTargetIds_||{};B("ytLoggingTransportTokensToJspbCttTargetIds_",Oo);var Po={};function Qo(){var a=C("yt.logging.ims");a||(a=new Ao,B("yt.logging.ims",a));return a}
function Ro(a,b){W("web_all_payloads_via_jspb")&&ej(new ok("transport.log called for JSON in JSPB only experiment"));if("log_event"===a.endpoint){So(a);var c=To(a);Po[c]=!0;var d={cttAuthInfo:c,isJspb:!1};Qo().storePayload(d,a.payload);Uo(b,c,!1,d)}}
function Vo(a,b){if("log_event"===a.endpoint){So(void 0,a);var c=To(a,!0);Po[c]=!0;var d={cttAuthInfo:c,isJspb:!0};Qo().storePayload(d,a.payload.toJSON());Uo(b,c,!0,d)}}
function Uo(a,b,c,d){c=void 0===c?!1:c;a&&(Ho=new a);a=gj("tvhtml5_logging_max_batch_ads_fork")||gj("tvhtml5_logging_max_batch")||gj("web_logging_max_batch")||100;var e=Y(),f=c?Ko.j:Jo.j,g=0;d&&(g=Qo().getSequenceCount(d));g>=a?Lo||(Lo=Wo(function(){Xo({writeThenSend:!0},W("flush_only_full_queue")?b:void 0,c);Lo=void 0},0)):10<=e-f&&(Yo(c),c?Ko.j=e:Jo.j=e)}
function Zo(a,b){W("web_all_payloads_via_jspb")&&ej(new ok("transport.logIsolatedGelPayload called in JSPB only experiment"));if("log_event"===a.endpoint){So(a);var c=To(a),d=new Map;d.set(c,[a.payload]);b&&(Ho=new b);return new Rf(function(e,f){Ho&&Ho.isReady()?$o(d,Ho,e,f,{bypassNetworkless:!0},!0):e()})}}
function ap(a,b){if("log_event"===a.endpoint){So(void 0,a);var c=To(a,!0),d=new Map;d.set(c,[a.payload.toJSON()]);b&&(Ho=new b);return new Rf(function(e){Ho&&Ho.isReady()?bp(d,Ho,e,{bypassNetworkless:!0},!0):e()})}}
function To(a,b){var c="";if(a.dangerousLogToVisitorSession)c="visitorOnlyApprovedKey";else if(a.cttAuthInfo){if(void 0===b?0:b){b=a.cttAuthInfo.token;c=a.cttAuthInfo;var d=new Ri;c.videoId?Yc(d,1,jd,c.videoId):c.playlistId&&Yc(d,2,jd,c.playlistId);Oo[b]=d}else b=a.cttAuthInfo,c={},b.videoId?c.videoId=b.videoId:b.playlistId&&(c.playlistId=b.playlistId),No[a.cttAuthInfo.token]=c;c=a.cttAuthInfo.token}return c}
function Xo(a,b,c){a=void 0===a?{}:a;c=void 0===c?!1:c;!c&&W("web_all_payloads_via_jspb")&&ej(new ok("transport.flushLogs called for JSON in JSPB only experiment"));new Rf(function(d,e){c?(cp(Ko.i),cp(Ko.h),Ko.h=0):(cp(Jo.i),cp(Jo.h),Jo.h=0);if(Ho&&Ho.isReady()){var f=a,g=c,h=Ho;f=void 0===f?{}:f;g=void 0===g?!1:g;var k=new Map,l=new Map;if(void 0!==b)g?(e=Qo().extractMatchingEntries({isJspb:g,cttAuthInfo:b}),k.set(b,e),bp(k,h,d,f)):(k=Qo().extractMatchingEntries({isJspb:g,cttAuthInfo:b}),l.set(b,
k),$o(l,h,d,e,f));else if(g){e=u(Object.keys(Po));for(g=e.next();!g.done;g=e.next())l=g.value,g=Qo().extractMatchingEntries({isJspb:!0,cttAuthInfo:l}),0<g.length&&k.set(l,g),delete Po[l];bp(k,h,d,f)}else{k=u(Object.keys(Po));for(g=k.next();!g.done;g=k.next()){g=g.value;var m=Qo().extractMatchingEntries({isJspb:!1,cttAuthInfo:g});0<m.length&&l.set(g,m);delete Po[g]}$o(l,h,d,e,f)}}else Yo(c),d()})}
function Yo(a){a=void 0===a?!1:a;if(W("web_gel_timeout_cap")&&(!a&&!Jo.h||a&&!Ko.h)){var b=Wo(function(){Xo({writeThenSend:!0},void 0,a)},Fo);
a?Ko.h=b:Jo.h=b}cp(a?Ko.i:Jo.i);b=U("LOGGING_BATCH_TIMEOUT",gj("web_gel_debounce_ms",1E4));W("shorten_initial_gel_batch_timeout")&&Mo&&(b=Eo);b=Wo(function(){Xo({writeThenSend:!0},void 0,a)},b);
a?Ko.i=b:Jo.i=b}
function $o(a,b,c,d,e,f){e=void 0===e?{}:e;var g=Math.round(Y()),h=a.size,k={};a=u(a);for(var l=a.next();!l.done;k={ob:k.ob,Fa:k.Fa,cb:k.cb,qb:k.qb,pb:k.pb},l=a.next()){var m=u(l.value);l=m.next().value;m=m.next().value;k.Fa=qb({context:Um(b.config_||Tm())});if(!Qa(m)&&!W("throw_err_when_logevent_malformed_killswitch")){d();break}k.Fa.events=m;(m=No[l])&&dp(k.Fa,l,m);delete No[l];k.cb="visitorOnlyApprovedKey"===l;ep(k.Fa,g,k.cb);fp(e);k.qb=function(n){W("update_log_event_config")&&Qe.ka(function(){return x(function(r){return w(r,
gp(n),0)})});
h--;h||c()};
k.ob=0;k.pb=function(n){return function(){n.ob++;if(e.bypassNetworkless&&1===n.ob)try{qn(b,"log_event",n.Fa,hp({writeThenSend:!0},n.cb,n.qb,n.pb,f)),Mo=!1}catch(r){dj(r),d()}h--;h||c()}}(k);
try{qn(b,"log_event",k.Fa,hp(e,k.cb,k.qb,k.pb,f)),Mo=!1}catch(n){dj(n),d()}}}
function bp(a,b,c,d,e){d=void 0===d?{}:d;var f=Math.round(Y()),g=a.size,h=new Map([].concat(ka(a)));h=u(h);for(var k=h.next();!k.done;k=h.next()){var l=u(k.value).next().value,m=a.get(l);k=new Si;var n=an(b.config_||Tm());J(k,xi,1,n);m=m?ip(m):[];m=u(m);for(n=m.next();!n.done;n=m.next())gd(k,3,Oi,n.value);(m=Oo[l])&&jp(k,l,m);delete Oo[l];l="visitorOnlyApprovedKey"===l;kp(k,f,l);fp(d);k=ud(k);l=hp(d,l,function(r){W("update_log_event_config")&&Qe.ka(function(){return x(function(p){return w(p,gp(r),
0)})});
g--;g||c()},function(){g--;
g||c()},e);
l.headers["Content-Type"]="application/json+protobuf";l.postBodyFormat="JSPB";l.postBody=k;qn(b,"log_event","",l);Mo=!1}}
function fp(a){W("always_send_and_write")&&(a.writeThenSend=!1)}
function hp(a,b,c,d,e){a={retry:!0,onSuccess:c,onError:d,sc:a,dangerousLogToVisitorSession:b,Md:!!e,headers:{},postBodyFormat:"",postBody:"",compress:W("compress_gel")};lp()&&(a.headers["X-Goog-Request-Time"]=JSON.stringify(Math.round(Y())));return a}
function ep(a,b,c){lp()||(a.requestTimeMs=String(b));W("unsplit_gel_payloads_in_logs")&&(a.unsplitGelPayloadsInLogs=!0);!c&&(b=U("EVENT_ID"))&&(c=mp(),a.serializedClientEventId={serializedEventId:b,clientCounter:String(c)})}
function kp(a,b,c){lp()||H(a,2,b);if(!c&&(b=U("EVENT_ID"))){c=mp();var d=new Qi;H(d,1,b);H(d,2,c);J(a,Qi,5,d)}}
function mp(){var a=U("BATCH_CLIENT_COUNTER")||0;a||(a=Math.floor(Math.random()*Go/2));a++;a>Go&&(a=1);Yi("BATCH_CLIENT_COUNTER",a);return a}
function dp(a,b,c){if(c.videoId)var d="VIDEO";else if(c.playlistId)d="PLAYLIST";else return;a.credentialTransferTokenTargetId=c;a.context=a.context||{};a.context.user=a.context.user||{};a.context.user.credentialTransferTokens=[{token:b,scope:d}]}
function jp(a,b,c){if(id(c,1))var d=1;else if(c.getPlaylistId())d=2;else return;J(a,Ri,4,c);a=$c(a,xi,1)||new xi;c=$c(a,vi,3)||new vi;var e=new ui;H(e,2,b);H(e,1,d);gd(c,12,ui,e);J(a,vi,3,c)}
function ip(a){for(var b=[],c=0;c<a.length;c++)try{b.push(new Oi(a[c]))}catch(d){dj(new ok("Transport failed to deserialize "+String(a[c])))}return b}
function So(a,b){if(C("yt.logging.transport.enableScrapingForTest")){var c=C("yt.logging.transport.scrapedPayloadsForTesting"),d=C("yt.logging.transport.payloadToScrape");b&&(b=C("yt.logging.transport.getScrapedPayloadFromClientEventsFunction").bind(b.payload)())&&c.push(b);if(d&&1<=d.length)for(b=0;b<d.length;b++)if(a&&a.payload[d[b]]){var e=void 0;c.push((null==(e=a)?void 0:e.payload)[d[b]])}B("yt.logging.transport.scrapedPayloadsForTesting",c)}}
function lp(){return W("use_request_time_ms_header")||W("lr_use_request_time_ms_header")}
function Wo(a,b){return W("transport_use_scheduler")?sk(a,b):rj(a,b)}
function cp(a){W("transport_use_scheduler")?Qe.Qa(a):window.clearTimeout(a)}
function gp(a){var b,c,d,e,f,g,h,k,l,m;return x(function(n){if(1==n.h){d=null==(b=a)?void 0:null==(c=b.responseContext)?void 0:c.globalConfigGroup;var r=d?d[hi.name]:void 0;e=r;g=null==(f=d)?void 0:f.hotHashData;r=d?d[gi.name]:void 0;h=r;l=null==(k=d)?void 0:k.coldHashData;zo||(zo=new wo);r=zo;return(m=r.resolve.call(r,new vo))?g?e?w(n,Qm(m,g,e),2):w(n,Qm(m,g),2):n.D(2):n.return()}return l?h?w(n,Rm(m,l,h),0):w(n,Rm(m,l),0):n.D(0)})}
;var np=z.ytLoggingGelSequenceIdObj_||{};B("ytLoggingGelSequenceIdObj_",np);function op(a){Xo(void 0,void 0,void 0===a?!1:a)}
function pp(a){np[a]=a in np?np[a]+1:0;return np[a]}
;var qp=[];
function rp(a,b){var c=void 0===c?{}:c;var d=jo;U("ytLoggingEventsDefaultDisabled",!1)&&jo===jo&&(d=null);if(W("web_all_payloads_via_jspb"))qp.push({Sd:a,payload:b,options:c});else{c=void 0===c?{}:c;var e={},f=Math.round(c.timestamp||Y());e.eventTimeMs=f<Number.MAX_SAFE_INTEGER?f:0;e[a]=b;W("enable_unknown_lact_fix_on_html5")&&mo();a=po();e.context={lastActivityMs:String(c.timestamp||!isFinite(a)?-1:a)};W("log_sequence_info_on_gel_web")&&c.sequenceGroup&&(a=e.context,b=c.sequenceGroup,b={index:pp(b),
groupKey:b},a.sequence=b,c.endOfSequence&&delete np[c.sequenceGroup]);(c.sendIsolatedPayload?Zo:Ro)({endpoint:"log_event",payload:e,cttAuthInfo:c.cttAuthInfo,dangerousLogToVisitorSession:c.dangerousLogToVisitorSession},d)}}
;var sp=z.ytLoggingGelSequenceIdObj_||{};B("ytLoggingGelSequenceIdObj_",sp);function tp(a){var b=void 0;b=void 0===b?{}:b;var c=!1;U("ytLoggingEventsDefaultDisabled",!1)&&(c=!0);c=c?null:jo;b=void 0===b?{}:b;var d=Math.round(b.timestamp||Y());H(a,1,d<Number.MAX_SAFE_INTEGER?d:0);var e=po();d=new Ni;H(d,1,b.timestamp||!isFinite(e)?-1:e);if(W("log_sequence_info_on_gel_web")&&b.sequenceGroup){e=b.sequenceGroup;var f=pp(e),g=new Mi;H(g,2,f);H(g,1,e);J(d,Mi,3,g);b.endOfSequence&&delete sp[b.sequenceGroup]}J(a,Ni,33,d);(b.sendIsolatedPayload?ap:Vo)({endpoint:"log_event",payload:a,
cttAuthInfo:b.cttAuthInfo,dangerousLogToVisitorSession:b.dangerousLogToVisitorSession},c)}
;var up=new Set,vp=0,wp=0,xp=0,yp=[],zp=["PhantomJS","Googlebot","TO STOP THIS SECURITY SCAN go/scan"];function Ap(){for(var a=u(zp),b=a.next();!b.done;b=a.next()){var c=Nb();if(c&&0<=c.toLowerCase().indexOf(b.value.toLowerCase()))return!0}return!1}
;function Bp(){var a;return x(function(b){return(a=kf())?b.return(a.then(function(c){c=ud(c);for(var d=[],e=0,f=0;f<c.length;f++){var g=c.charCodeAt(f);255<g&&(d[e++]=g&255,g>>=8);d[e++]=g}return zc(d,3)})):b.return(Promise.resolve(null))})}
;var Cp={};function Dp(a){return Cp[a]||(Cp[a]=String(a).replace(/\-([a-z])/g,function(b,c){return c.toUpperCase()}))}
;var Ep={},Fp=[],hg=new L,Gp={};function Hp(){for(var a=u(Fp),b=a.next();!b.done;b=a.next())b=b.value,b()}
function Ip(a,b){var c;"yt:"===a.tagName.toLowerCase().substr(0,3)?c=a.getAttribute(b):c=a?a.dataset?a.dataset[Dp(b)]:a.getAttribute("data-"+b):null;return c}
function Jp(a){hg.ab.apply(hg,arguments)}
;function Kp(a){this.h=a||{};a=[this.h,window.YTConfig||{}];for(var b=0;b<a.length;b++)a[b].host&&(a[b].host=a[b].host.toString().replace("http://","https://"))}
function Lp(a,b){a=[a.h,window.YTConfig||{}];for(var c=0;c<a.length;c++){var d=a[c][b];if(void 0!==d)return d}return null}
function Mp(a,b,c){Np||(Np={},qj(window,"message",function(d){a:{if(d.origin===Lp(a,"host")){try{var e=JSON.parse(d.data)}catch(f){e=void 0;break a}if(d=Np[e.id])d.H=!0,d.H&&(E(d.s,d.sendMessage,d),d.s.length=0),d.Xb(e)}e=void 0}return e}));
Np[c]=b}
var Np=null;var Op=window;
function Pp(a,b,c){this.v=this.h=this.i=null;this.j=0;this.H=!1;this.s=[];this.m=null;this.X={};if(!a)throw Error("YouTube player element ID required.");this.id=Sa(this);this.N=c;c=document;if(a="string"===typeof a?c.getElementById(a):a)if(c="iframe"===a.tagName.toLowerCase(),b.host||(b.host=c?Vb(a.src):"https://www.youtube.com"),this.i=new Kp(b),c||(b=Qp(this,a),this.v=a,(c=a.parentNode)&&c.replaceChild(b,a),a=b),this.h=a,this.h.id||(this.h.id="widget"+Sa(this.h)),Ep[this.h.id]=this,window.postMessage){this.m=
new L;Rp(this);b=Lp(this.i,"events");for(var d in b)b.hasOwnProperty(d)&&this.addEventListener(d,b[d]);for(var e in Gp)Gp.hasOwnProperty(e)&&Sp(this,e)}}
q=Pp.prototype;q.setSize=function(a,b){this.h.width=a.toString();this.h.height=b.toString();return this};
q.getIframe=function(){return this.h};
q.Xb=function(a){Tp(this,a.event,a)};
q.addEventListener=function(a,b){var c=b;"string"===typeof b&&(c=function(){window[b].apply(window,arguments)});
if(!c)return this;this.m.subscribe(a,c);Up(this,a);return this};
function Sp(a,b){b=b.split(".");if(2===b.length){var c=b[1];a.N===b[0]&&Up(a,c)}}
q.destroy=function(){this.h&&this.h.id&&(Ep[this.h.id]=null);var a=this.m;a&&"function"==typeof a.dispose&&a.dispose();if(this.v){a=this.h;var b=a.parentNode;b&&b.replaceChild(this.v,a)}else(a=this.h)&&a.parentNode&&a.parentNode.removeChild(a);Np&&(Np[this.id]=null);this.i=null;a=this.h;for(var c in ob)ob[c][0]==a&&oj(c);this.v=this.h=null};
q.cc=function(){return{}};
function Vp(a,b,c){c=c||[];c=Array.prototype.slice.call(c);b={event:"command",func:b,args:c};a.H?a.sendMessage(b):a.s.push(b)}
function Tp(a,b,c){a.m.j||(c={target:a,data:c},a.m.ab(b,c),Jp(a.N+"."+b,c))}
function Qp(a,b){var c=document.createElement("iframe");b=b.attributes;for(var d=0,e=b.length;d<e;d++){var f=b[d].value;null!=f&&""!==f&&"null"!==f&&c.setAttribute(b[d].name,f)}c.setAttribute("frameBorder","0");c.setAttribute("allowfullscreen","1");c.setAttribute("allow","accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share");c.setAttribute("title","YouTube "+Lp(a.i,"title"));(b=Lp(a.i,"width"))&&c.setAttribute("width",b.toString());(b=Lp(a.i,"height"))&&
c.setAttribute("height",b.toString());var g=a.cc();g.enablejsapi=window.postMessage?1:0;window.location.host&&(g.origin=window.location.protocol+"//"+window.location.host);g.widgetid=a.id;window.location.href&&E(["debugjs","debugcss"],function(k){var l=Zb(window.location.href,k);null!==l&&(g[k]=l)});
var h=""+Lp(a.i,"host")+("/embed/"+Lp(a.i,"videoId"))+"?"+Xb(g);Op.yt_embedsEnableUaChProbe?Bp().then(function(k){var l=new URL(h),m=Number(l.searchParams.get("reloads"));isNaN(m)&&(m=0);l.searchParams.set("reloads",(m+1).toString());k&&l.searchParams.set("uach",k);l.searchParams.set("uats",Math.floor(window.performance.timeOrigin).toString());k=Ld(l.href).toString();Hd(c,Md(k));c.sandbox.add("allow-presentation","allow-top-navigation");return k}):Op.yt_embedsEnableIframeSrcWithIntent?(Hd(c,Md(h)),
c.sandbox.add("allow-presentation","allow-top-navigation")):c.src=h;
return c}
q.uc=function(){this.h&&this.h.contentWindow?this.sendMessage({event:"listening"}):window.clearInterval(this.j)};
function Rp(a){Mp(a.i,a,a.id);a.j=sj(a.uc.bind(a),250);qj(a.h,"load",function(){window.clearInterval(a.j);a.j=sj(a.uc.bind(a),250)})}
function Up(a,b){a.X[b]||(a.X[b]=!0,Vp(a,"addEventListener",[b]))}
q.sendMessage=function(a){a.id=this.id;a.channel="widget";var b=JSON.stringify(a),c=[Vb(this.h.src||"").replace("http:","https:")];if(this.h.contentWindow)for(var d=0;d<c.length;d++)try{this.h.contentWindow.postMessage(b,c[d])}catch(jc){if(jc.name&&"SyntaxError"===jc.name){if(!(jc.message&&0<jc.message.indexOf("target origin ''"))){var e=void 0,f=jc;e=void 0===e?{}:e;e.name=U("INNERTUBE_CONTEXT_CLIENT_NAME",1);e.version=U("INNERTUBE_CONTEXT_CLIENT_VERSION");var g=e||{},h="WARNING";h=void 0===h?"ERROR":
h;if(f){f.hasOwnProperty("level")&&f.level&&(h=f.level);if(W("console_log_js_exceptions")){var k=f,l=[];l.push("Name: "+k.name);l.push("Message: "+k.message);k.hasOwnProperty("params")&&l.push("Error Params: "+JSON.stringify(k.params));k.hasOwnProperty("args")&&l.push("Error args: "+JSON.stringify(k.args));l.push("File name: "+k.fileName);l.push("Stacktrace: "+k.stack);window.console.log(l.join("\n"),k)}if(!(5<=vp)){var m=void 0,n=void 0,r=f,p=g,y=Bd(r),A=y.message||"Unknown Error",I=y.name||"UnknownError",
N=y.stack||r.i||"Not available";if(N.startsWith(I+": "+A)){var T=N.split("\n");T.shift();N=T.join("\n")}var P=y.lineNumber||"Not available",wa=y.fileName||"Not available",Mc=N,Ha=0;if(r.hasOwnProperty("args")&&r.args&&r.args.length)for(var xa=0;xa<r.args.length&&!(Ha=ek(r.args[xa],"params."+xa,p,Ha),500<=Ha);xa++);else if(r.hasOwnProperty("params")&&r.params){var ba=r.params;if("object"===typeof r.params)for(n in ba){if(ba[n]){var ia="params."+n,ja=gk(ba[n]);p[ia]=ja;Ha+=ia.length+ja.length;if(500<
Ha)break}}else p.params=gk(ba)}if(yp.length)for(var aa=0;aa<yp.length&&!(Ha=ek(yp[aa],"params.context."+aa,p,Ha),500<=Ha);aa++);navigator.vendor&&!p.hasOwnProperty("vendor")&&(p["device.vendor"]=navigator.vendor);var V={message:A,name:I,lineNumber:P,fileName:wa,stack:Mc,params:p,sampleWeight:1},bl=Number(r.columnNumber);isNaN(bl)||(V.lineNumber=V.lineNumber+":"+bl);if("IGNORED"===r.level)m=0;else a:{for(var cl=ak(),dl=u(cl.Ba),Gg=dl.next();!Gg.done;Gg=dl.next()){var el=Gg.value;if(V.message&&V.message.match(el.Pd)){m=
el.weight;break a}}for(var fl=u(cl.ya),Hg=fl.next();!Hg.done;Hg=fl.next()){var gl=Hg.value;if(gl.Kc(V)){m=gl.weight;break a}}m=1}V.sampleWeight=m;for(var hl=u(Vj),Ig=hl.next();!Ig.done;Ig=hl.next()){var Jg=Ig.value;if(Jg.Ab[V.name])for(var il=u(Jg.Ab[V.name]),Kg=il.next();!Kg.done;Kg=il.next()){var jl=Kg.value,ne=V.message.match(jl.regexp);if(ne){V.params["params.error.original"]=ne[0];for(var Lg=jl.groups,kl={},kc=0;kc<Lg.length;kc++)kl[Lg[kc]]=ne[kc+1],V.params["params.error."+Lg[kc]]=ne[kc+1];
V.message=Jg.Pb(kl);break}}}V.params||(V.params={});var ll=ak();V.params["params.errorServiceSignature"]="msg="+ll.Ba.length+"&cb="+ll.ya.length;V.params["params.serviceWorker"]="false";z.document&&z.document.querySelectorAll&&(V.params["params.fscripts"]=String(document.querySelectorAll("script:not([nonce])").length));vb("sample").constructor!==ub&&(V.params["params.fconst"]="true");var za=V;window.yterr&&"function"===typeof window.yterr&&window.yterr(za);if(0!==za.sampleWeight&&!up.has(za.message)){if("ERROR"===
h){bk.ab("handleError",za);if(W("record_app_crashed_web")&&0===xp&&1===za.sampleWeight)if(xp++,W("errors_via_jspb")){var Mg=new Li;H(Mg,1,1);if(!W("report_client_error_with_app_crash_ks")){var ml=new Gi;H(ml,1,za.message);var nl=new Hi;J(nl,Gi,3,ml);var ol=new Ii;J(ol,Hi,5,nl);var pl=new Ji;J(pl,Ii,9,ol);J(Mg,Ji,4,pl)}var aq=Mg,ql=new Oi;bd(ql,Li,20,Pi,aq);tp(ql)}else{var rl={appCrashType:"APP_CRASH_TYPE_BREAKPAD"};W("report_client_error_with_app_crash_ks")||(rl.systemHealth={crashData:{clientError:{logMessage:{message:za.message}}}});
rp("appCrashed",rl)}wp++}else"WARNING"===h&&bk.ab("handleWarning",za);if(W("kevlar_gel_error_routing"))a:{var Ng=void 0,Og=void 0,dd=h,S=za;if(W("errors_via_jspb")){if(Ap())Og=void 0;else{var lc=new Di;H(lc,1,S.stack);S.fileName&&H(lc,4,S.fileName);var Wa=S.lineNumber&&S.lineNumber.split?S.lineNumber.split(":"):[];0!==Wa.length&&(1!==Wa.length||isNaN(Number(Wa[0]))?2!==Wa.length||isNaN(Number(Wa[0]))||isNaN(Number(Wa[1]))||(H(lc,2,Number(Wa[0])),H(lc,3,Number(Wa[1]))):H(lc,2,Number(Wa[0])));var Eb=
new Gi;H(Eb,1,S.message);H(Eb,3,S.name);H(Eb,6,S.sampleWeight);"ERROR"===dd?H(Eb,2,2):"WARNING"===dd?H(Eb,2,1):H(Eb,2,0);var Pg=new Ei;H(Pg,1,!0);bd(Pg,Di,3,Fi,lc);var Fb=new Bi;H(Fb,3,window.location.href);for(var sl=U("FEXP_EXPERIMENTS",[]),Qg=0;Qg<sl.length;Qg++){var tl=Fb,bq=sl[Qg];Pc(tl);Xc(tl,5,2,!1,!1).push(bq)}var Rg=Zi();if(!$i()&&Rg)for(var ul=u(Object.keys(Rg)),Gb=ul.next();!Gb.done;Gb=ul.next()){var vl=Gb.value,Sg=new Ai;H(Sg,1,vl);H(Sg,2,String(Rg[vl]));gd(Fb,4,Ai,Sg)}var Tg=S.params;
if(Tg){var wl=u(Object.keys(Tg));for(Gb=wl.next();!Gb.done;Gb=wl.next()){var xl=Gb.value,Ug=new Ai;H(Ug,1,"client."+xl);H(Ug,2,String(Tg[xl]));gd(Fb,4,Ai,Ug)}}var yl=U("SERVER_NAME"),zl=U("SERVER_VERSION");if(yl&&zl){var Vg=new Ai;H(Vg,1,"server.name");H(Vg,2,yl);gd(Fb,4,Ai,Vg);var Wg=new Ai;H(Wg,1,"server.version");H(Wg,2,zl);gd(Fb,4,Ai,Wg)}var oe=new Hi;J(oe,Bi,1,Fb);J(oe,Ei,2,Pg);J(oe,Gi,3,Eb);Og=oe}var Al=Og;if(!Al)break a;var Bl=new Oi;bd(Bl,Hi,163,Pi,Al);tp(Bl)}else{if(Ap())Ng=void 0;else{var ed=
{stackTrace:S.stack};S.fileName&&(ed.filename=S.fileName);var Xa=S.lineNumber&&S.lineNumber.split?S.lineNumber.split(":"):[];0!==Xa.length&&(1!==Xa.length||isNaN(Number(Xa[0]))?2!==Xa.length||isNaN(Number(Xa[0]))||isNaN(Number(Xa[1]))||(ed.lineNumber=Number(Xa[0]),ed.columnNumber=Number(Xa[1])):ed.lineNumber=Number(Xa[0]));var Xg={level:"ERROR_LEVEL_UNKNOWN",message:S.message,errorClassName:S.name,sampleWeight:S.sampleWeight};"ERROR"===dd?Xg.level="ERROR_LEVEL_ERROR":"WARNING"===dd&&(Xg.level="ERROR_LEVEL_WARNNING");
var cq={isObfuscated:!0,browserStackInfo:ed},mc={pageUrl:window.location.href,kvPairs:[]};U("FEXP_EXPERIMENTS")&&(mc.experimentIds=U("FEXP_EXPERIMENTS"));var Yg=Zi();if(!$i()&&Yg)for(var Cl=u(Object.keys(Yg)),Hb=Cl.next();!Hb.done;Hb=Cl.next()){var Dl=Hb.value;mc.kvPairs.push({key:Dl,value:String(Yg[Dl])})}var Zg=S.params;if(Zg){var El=u(Object.keys(Zg));for(Hb=El.next();!Hb.done;Hb=El.next()){var Fl=Hb.value;mc.kvPairs.push({key:"client."+Fl,value:String(Zg[Fl])})}}var Gl=U("SERVER_NAME"),Hl=U("SERVER_VERSION");
Gl&&Hl&&(mc.kvPairs.push({key:"server.name",value:Gl}),mc.kvPairs.push({key:"server.version",value:Hl}));Ng={errorMetadata:mc,stackTrace:cq,logMessage:Xg}}var Il=Ng;if(!Il)break a;rp("clientError",Il)}if("ERROR"===dd||W("errors_flush_gel_always_killswitch"))b:{if(W("web_fp_via_jspb")&&(op(!0),!W("web_fp_via_jspb_and_json")))break b;op()}}if(!W("suppress_error_204_logging")){var Ib=za,fd=Ib.params||{},lb={urlParams:{a:"logerror",t:"jserror",type:Ib.name,msg:Ib.message.substr(0,250),line:Ib.lineNumber,
level:h,"client.name":fd.name},postParams:{url:U("PAGE_NAME",window.location.href),file:Ib.fileName},method:"POST"};fd.version&&(lb["client.version"]=fd.version);if(lb.postParams){Ib.stack&&(lb.postParams.stack=Ib.stack);for(var Jl=u(Object.keys(fd)),$g=Jl.next();!$g.done;$g=Jl.next()){var Kl=$g.value;lb.postParams["client."+Kl]=fd[Kl]}var ah=Zi();if(ah)for(var Ll=u(Object.keys(ah)),bh=Ll.next();!bh.done;bh=Ll.next()){var Ml=bh.value;lb.postParams[Ml]=ah[Ml]}var Nl=U("SERVER_NAME"),Ol=U("SERVER_VERSION");
Nl&&Ol&&(lb.postParams["server.name"]=Nl,lb.postParams["server.version"]=Ol)}Lj(U("ECATCHER_REPORT_HOST","")+"/error_204",lb)}try{up.add(za.message)}catch(hq){}vp++}}}}}else throw jc;}else console&&console.warn&&console.warn("The YouTube player is not attached to the DOM. API calls should be made after the onReady event. See more: https://developers.google.com/youtube/iframe_api_reference#Events")};function Wp(a){return(0===a.search("cue")||0===a.search("load"))&&"loadModule"!==a}
function Xp(a){return 0===a.search("get")||0===a.search("is")}
;function Yp(a,b){Pp.call(this,a,Object.assign({title:"video player",videoId:"",width:640,height:360},b||{}),"player");this.na={};this.playerInfo={};this.videoTitle=""}
v(Yp,Pp);q=Yp.prototype;q.cc=function(){var a=Lp(this.i,"playerVars");if(a){var b={},c;for(c in a)b[c]=a[c];a=b}else a={};window!==window.top&&document.referrer&&(a.widget_referrer=document.referrer.substring(0,256));if(c=Lp(this.i,"embedConfig")){if(Ra(c))try{c=JSON.stringify(c)}catch(d){console.error("Invalid embed config JSON",d)}a.embed_config=c}return a};
q.Xb=function(a){var b=a.event;a=a.info;switch(b){case "apiInfoDelivery":if(Ra(a))for(var c in a)a.hasOwnProperty(c)&&(this.na[c]=a[c]);break;case "infoDelivery":Zp(this,a);break;case "initialDelivery":Ra(a)&&(window.clearInterval(this.j),this.playerInfo={},this.na={},$p(this,a.apiInterface),Zp(this,a));break;default:Tp(this,b,a)}};
function Zp(a,b){if(Ra(b)){for(var c in b)b.hasOwnProperty(c)&&(a.playerInfo[c]=b[c]);a.playerInfo.hasOwnProperty("videoData")&&(b=a.playerInfo.videoData,b.hasOwnProperty("title")&&b.title?(b=b.title,b!==a.videoTitle&&(a.videoTitle=b,a.h.setAttribute("title",b))):(a.videoTitle="",a.h.setAttribute("title","YouTube "+Lp(a.i,"title"))))}}
function $p(a,b){E(b,function(c){this[c]||("getCurrentTime"===c?this[c]=function(){var d=this.playerInfo.currentTime;if(1===this.playerInfo.playerState){var e=(Date.now()/1E3-this.playerInfo.currentTimeLastUpdated_)*this.playerInfo.playbackRate;0<e&&(d+=Math.min(e,1))}return d}:Wp(c)?this[c]=function(){this.playerInfo={};
this.na={};Vp(this,c,arguments);return this}:Xp(c)?this[c]=function(){var d=0;
0===c.search("get")?d=3:0===c.search("is")&&(d=2);return this.playerInfo[c.charAt(d).toLowerCase()+c.substr(d+1)]}:this[c]=function(){Vp(this,c,arguments);
return this})},a)}
q.getVideoEmbedCode=function(){var a=Lp(this.i,"host")+("/embed/"+Lp(this.i,"videoId")),b=Number(Lp(this.i,"width")),c=Number(Lp(this.i,"height"));if(isNaN(b)||isNaN(c))throw Error("Invalid width or height property");b=Math.floor(b);c=Math.floor(c);var d=this.videoTitle;a=Rb(a);d=Rb(null!=d?d:"YouTube video player");return'<iframe width="'+b+'" height="'+c+'" src="'+a+'" title="'+(d+'" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" allowfullscreen></iframe>')};
q.getOptions=function(a){return this.na.namespaces?a?this.na[a]?this.na[a].options||[]:[]:this.na.namespaces||[]:[]};
q.getOption=function(a,b){if(this.na.namespaces&&a&&b&&this.na[a])return this.na[a][b]};
function dq(a){if("iframe"!==a.tagName.toLowerCase()){var b=Ip(a,"videoid");b&&(b={videoId:b,width:Ip(a,"width"),height:Ip(a,"height")},new Yp(a,b))}}
;B("YT.PlayerState.UNSTARTED",-1);B("YT.PlayerState.ENDED",0);B("YT.PlayerState.PLAYING",1);B("YT.PlayerState.PAUSED",2);B("YT.PlayerState.BUFFERING",3);B("YT.PlayerState.CUED",5);B("YT.get",function(a){return Ep[a]});
B("YT.scan",Hp);B("YT.subscribe",function(a,b,c){hg.subscribe(a,b,c);Gp[a]=!0;for(var d in Ep)Ep.hasOwnProperty(d)&&Sp(Ep[d],a)});
B("YT.unsubscribe",function(a,b,c){gg(a,b,c)});
B("YT.Player",Yp);Pp.prototype.destroy=Pp.prototype.destroy;Pp.prototype.setSize=Pp.prototype.setSize;Pp.prototype.getIframe=Pp.prototype.getIframe;Pp.prototype.addEventListener=Pp.prototype.addEventListener;Yp.prototype.getVideoEmbedCode=Yp.prototype.getVideoEmbedCode;Yp.prototype.getOptions=Yp.prototype.getOptions;Yp.prototype.getOption=Yp.prototype.getOption;
Fp.push(function(a){var b=a;b||(b=document);a=jb(b.getElementsByTagName("yt:player"));var c=b||document;if(c.querySelectorAll&&c.querySelector)b=c.querySelectorAll(".yt-player");else{var d;c=document;b=b||c;if(b.querySelectorAll&&b.querySelector)b=b.querySelectorAll(".yt-player");else if(b.getElementsByClassName){var e=b.getElementsByClassName("yt-player");b=e}else{e=b.getElementsByTagName("*");var f={};for(c=d=0;b=e[c];c++){var g=b.className,h;if(h="function"==typeof g.split)h=0<=eb(g.split(/\s+/),
"yt-player");h&&(f[d++]=b)}f.length=d;b=f}}b=jb(b);E(ib(a,b),dq)});
"undefined"!=typeof YTConfig&&YTConfig.parsetags&&"onload"!=YTConfig.parsetags||Hp();var eq=z.onYTReady;eq&&eq();var fq=z.onYouTubeIframeAPIReady;fq&&fq();var gq=z.onYouTubePlayerAPIReady;gq&&gq();}).call(this);
