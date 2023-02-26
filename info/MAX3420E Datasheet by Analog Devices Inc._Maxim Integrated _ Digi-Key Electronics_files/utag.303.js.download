//~~tv:3158.20180305
//~~tc: Added clickTaleTagInjected initialization check

window.clickTaleTagInjected = window.clickTaleTagInjected || false;

//tealium universal tag - utag.sender.3158 ut4.0.202010282035, Copyright 2020 Tealium.com Inc. All Rights Reserved.
try {
  (function (id, loader) {
    var u = {"id" : id};
    utag.o[loader].sender[id] = u;
    // Please do not modify
    if (utag.ut === undefined) { utag.ut = {}; }
    // Start Tealium loader 4.41
     /* utag.js version 4.26 or above is required to avoid errors with this loader function */
    var match = /ut\d\.(\d*)\..*/.exec(utag.cfg.v);
    if (utag.ut.loader === undefined || !match || parseInt(match[1]) < 41) { u.loader = function(o, a, b, c, l, m) { utag.DB(o); a = document; if (o.type == "iframe") { m = a.getElementById(o.id); if (m && m.tagName == "IFRAME") { b = m; } else { b = a.createElement("iframe"); } o.attrs = o.attrs || {}; utag.ut.merge(o.attrs, { "height": "1", "width": "1", "style": "display:none" }, 0); } else if (o.type == "img") { utag.DB("Attach img: " + o.src); b = new Image(); } else { b = a.createElement("script"); b.language = "javascript"; b.type = "text/javascript"; b.async = 1; b.charset = "utf-8"; } if (o.id) { b.id = o.id; } for (l in utag.loader.GV(o.attrs)) { b.setAttribute(l, o.attrs[l]); } b.setAttribute("src", o.src); if (typeof o.cb == "function") { if (b.addEventListener) { b.addEventListener("load", function() { o.cb(); }, false); } else { b.onreadystatechange = function() { if (this.readyState == "complete" || this.readyState == "loaded") { this.onreadystatechange = null; o.cb(); } }; } } if (o.type != "img" && !m) { l = o.loc || "head"; c = a.getElementsByTagName(l)[0]; if (c) { utag.DB("Attach to " + l + ": " + o.src); if (l == "script") { c.parentNode.insertBefore(b, c); } else { c.appendChild(b); } } } }; } else { u.loader = utag.ut.loader; }
    // End Tealium loader
    // Start Tealium typeOf 4.35
    if (utag.ut.typeOf === undefined) { u.typeOf = function(e) {return ({}).toString.call(e).match(/\s([a-zA-Z]+)/)[1].toLowerCase();};} else { u.typeOf = utag.ut.typeOf; }
    // End Tealium typeOf

    u.ev = {"view" : 1};

    // Start Loader Callback
    u.loader_cb = function () {
      utag.DB("send:303:CALLBACK");
      window.clickTaleTagInjected = true;
      // Add code here
      utag.DB("send:303:CALLBACK:COMPLETE");
    };
    // End Loader Callback

      u.map={"clicktail_project_guid":"project_guid"};
  u.extend=[];


    u.send = function (a, b) {
      if (u.ev[a] || u.ev.all !== undefined) {
        utag.DB("send:303");
        utag.DB(b);

        var d, e, f;

        u.data = {
          "qsp_delim" : "&",
          "kvp_delim" : "=",
          "base_url" : "https://cdnssl.clicktale.net/##utag_partition##/ptc/##utag_project_guid##.js",
          "partition" : "www",
          "project_guid" : ""
        };

        // Start tag-scoped extensions
        
        utag.DB("send:303:EXTENSIONS");
        utag.DB(b);
        // End tag-scoped extensions

        // Start Mapping
        for (d in utag.loader.GV(u.map)) {
          if (b[d] !== undefined && b[d] !== "") {
            e = u.map[d].split(",");
            for (f = 0; f < e.length; f++) {
              u.data[e[f]] = b[d];
            }
          }
        }
        utag.DB("send:303:MAPPINGS");
        utag.DB(u.data);
        // End Mapping

        // Report required config is missing, and stop tag from firing.
        if (!u.data.partition || !u.data.project_guid) {
          utag.DB(u.id + ": Tag not fired: Required attribute not populated");
          return;
        }

        u.data.base_url = u.data.base_url.replace("##utag_partition##",u.data.partition);
        u.data.base_url = u.data.base_url.replace("##utag_project_guid##",u.data.project_guid);

        if (!window.clickTaleTagInjected) {
          u.loader({
            "type" : "script",
            "src" : u.data.base_url,
            "cb" : u.loader_cb,
            "loc" : "script",
            "id" : "utag_303",
            "attrs" : {
              "crossOrigin" : "anonymous"
            }
          });
        }
        

        utag.DB("send:303:COMPLETE");
      }
    };
    utag.o[loader].loader.LOAD(id);
  }("303", "digikey.main"));
} catch (error) {
  utag.DB(error);
}
//end tealium universal tag
