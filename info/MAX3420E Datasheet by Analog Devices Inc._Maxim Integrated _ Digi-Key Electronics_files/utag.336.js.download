//~~tv:12047.20170514
//~~tc: Adding tag LinkedIn Insight Tag

var _linkedin = _linkedin || {};
var _linkedin_data_partner_id   = _linkedin_data_partner_id  || "";

//tealium universal tag - utag.sender.12047 ut4.0.202010282035, Copyright 2020 Tealium.com Inc. All Rights Reserved.
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


    u.map_func = function (arr, obj, item) {
      var i = arr.shift();
      obj[i] = obj[i] || {};
      if (arr.length > 0) {
        u.map_func(arr,obj[i], item);
      } else {
        obj[i] = item;
      }
    };

    u.clearEmptyKeys = function (object) {
      for (var key in object) {
        if (object[key] === "" || object[key] === undefined) {
          delete object[key];
        }
      }
      return object;
    };

      u.map={"linkedin_conversion_id":"conversionId"};
  u.extend=[];



    u.send = function (a, b) {
      if (u.ev[a] || u.ev.all !== undefined) {
        utag.DB("send:336");
        utag.DB(b);

        var c, d, e, f;

        u.data = {
          //##UTVARconfig_<id from config>##
          "qsp_delim" : "&",
          "kvp_delim" : "=",
          "base_url" : "https://snap.licdn.com/li.lms-analytics/insight.min.js",
          "partner_id" : "966017",
          //LinkedIn Mappings
          "linkedin" : {
            "custom_channel_id": "",
            "custom_group_id": "",
            "custom_user_id": "",
            "zoom_info_id": "",
            "title": "",
            "domain": "",
            "company": "",
            "gender": "",
            "location": "",
            "education": "",
            //"email": "data_partner_email",
            "email_sha256": "",
            "email_sha512": "",
            "raw_data": "",
            "raw_data_overwrite": "",
            "encrypted_data": "",
            "partner_data": "",
            "sic_codes": "",
            "employee_range": "",
            "default_keywords": "",
            //"order_id": "order_id",
            "async_target": "",
            "use_iframe": "",
            "use_callback": "",
            "test_url": ""
            //"conv_value": "conversion_value",
            //"conv_currency": "conversion_currency"
          }

        };

        // Start tag-scoped extensions
        
        utag.DB("send:336:EXTENSIONS");
        utag.DB(b);
        // End tag-scoped extensions

        c = [];

        // Start Mapping
        for (d in utag.loader.GV(u.map)) {
          if (b[d] !== undefined && b[d] !== "") {
            e = u.map[d].split(",");
            for (f = 0; f < e.length; f++) {
              // Advanced
              u.map_func(e[f].split("."), u.data, b[d]);
            }
          }
        }
        utag.DB("send:336:MAPPINGS");
        utag.DB(u.data);
        // End Mapping

        // Pull E-Commerce extension values
        // Mappings override E-Commerce extension values
        u.data.order_id = u.data.order_id || b._corder || "";
        u.data.order_total = u.data.order_total || b._ctotal || "";
        u.data.order_currency = u.data.order_currency || b._ccurrency || "";
        u.data.customer_id = u.data.customer_id || b._ccustid || "";

        // Report required config is missing, and stop tag from firing.
        if (!u.data.partner_id) {
          utag.DB(u.id + ": Tag not fired: Required attribute not populated");
          return;
        }

        _linkedin_data_partner_id = u.data.partner_id;

        u.data.linkedin.partner_id = u.data.partner_id;
        u.data.linkedin.order_id = u.data.order_id;
        u.data.linkedin.conv_value = u.data.order_total;
        u.data.linkedin.conv_currency = u.data.order_currency;

        _linkedin = u.clearEmptyKeys(u.data.linkedin);

        u.loader({
          "type" : "script",
          "src" : u.data.base_url,
          "cb" : null,
          "loc" : "script",
          "id" : "utag_336",
          "attrs" : {}
        });

        utag.DB("send:336:COMPLETE");
      }
    };
    utag.o[loader].loader.LOAD(id);
  }("336", "digikey.main"));
} catch (error) {
  utag.DB(error);
}
//end tealium universal tag

