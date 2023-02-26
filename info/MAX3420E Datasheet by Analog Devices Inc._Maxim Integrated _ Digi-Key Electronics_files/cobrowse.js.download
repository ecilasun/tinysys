!function(e) {
    var t = {};
    function n(o) {
        if (t[o])
            return t[o].exports;
        var s = t[o] = {
            i: o,
            l: !1,
            exports: {}
        };
        return e[o].call(s.exports, s, s.exports, n),
        s.l = !0,
        s.exports
    }
    n.m = e,
    n.c = t,
    n.d = function(e, t, o) {
        n.o(e, t) || Object.defineProperty(e, t, {
            enumerable: !0,
            get: o
        })
    }
    ,
    n.r = function(e) {
        "undefined" != typeof Symbol && Symbol.toStringTag && Object.defineProperty(e, Symbol.toStringTag, {
            value: "Module"
        }),
        Object.defineProperty(e, "__esModule", {
            value: !0
        })
    }
    ,
    n.t = function(e, t) {
        if (1 & t && (e = n(e)),
        8 & t)
            return e;
        if (4 & t && "object" == typeof e && e && e.__esModule)
            return e;
        var o = Object.create(null);
        if (n.r(o),
        Object.defineProperty(o, "default", {
            enumerable: !0,
            value: e
        }),
        2 & t && "string" != typeof e)
            for (var s in e)
                n.d(o, s, function(t) {
                    return e[t]
                }
                .bind(null, s));
        return o
    }
    ,
    n.n = function(e) {
        var t = e && e.__esModule ? function() {
            return e.default
        }
        : function() {
            return e
        }
        ;
        return n.d(t, "a", t),
        t
    }
    ,
    n.o = function(e, t) {
        return Object.prototype.hasOwnProperty.call(e, t)
    }
    ,
    n.p = "",
    n(n.s = 73)
}({
    73: function(e, t, n) {
        "use strict";
        var o = "https://10.10.174.103/dkcfx/session.php";
        window.getAssistantSDK = function() {
            var e, t;
            e = function(e) {
                AssistSDK.onScreenshareRequest = function() {
                    return !0
                }
                ,
                AssistSDK.onInSupport = function() {
                    var e;
                    (e = document.body.querySelector("#button_stop_assistant")).className = "active",
                    e.addEventListener("click", function() {
                        e.className = "hidden",
                        AssistSDK.endSupport()
                    })
                }
                ,
                AssistSDK.startSupport({
                    correlationId: e.cid,
                    sessionToken: e.token,
                    sdkPath: "https://cobrowse.digikey.us:8443/assistserver/sdk/web/consumer"
                }),
                function(e) {
                    var t = window.__cobrowse
                      , n = document.createElement("div");
                    n.innerHTML = '\n    <div class="dk_modal">\n        <div class="dk_modal--wrapper">\n            <div class="dk_modal--titlebar">\n                <span class="dk_modal--close">&#xf00d;</span>\n                <span class="dk_modal--title"></span>\n            </div>\n            <div class="dk_modal--content">\n                <p id="cobrowse__message"></p>\n                <p id="cobrowse__code"></p>\n            </div>\n            <div class="dk_modal--buttons">\n                <span class="button primary"></span>\n            </div>\n        </div>\n    </div>';
                    var o = n.children[0];
                    function s(e) {
                        document.body.removeChild(o),
                        e.preventDefault()
                    }
                    n.querySelector(".dk_modal--title").textContent = t.title,
                    n.querySelector("#cobrowse__message").textContent = t.message,
                    n.querySelector(".button").textContent = t.close,
                    n.querySelector("#cobrowse__code").textContent = e,
                    n.querySelector(".dk_modal--close").addEventListener("click", s),
                    n.querySelector(".button").addEventListener("click", s),
                    o.addEventListener("click", function(e) {
                        e.target === o && s(e)
                    }),
                    document.body.appendChild(o)
                }(e.shortcode)
            }
            ,
            (t = new XMLHttpRequest).open("POST", o, !0),
            t.setRequestHeader("Content-Type", "application/x-www-form-urlencoded"),
            t.onload = function() {
                if (this.status >= 200 && this.status <= 400) {
                    var t = JSON.parse(this.response);
                    e(t)
                }
            }
            ,
            t.send("action=getScCid")
        }
    }
});
//# sourceMappingURL=cobrowse.js.map
