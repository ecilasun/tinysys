/*global Webtrends,videojs,isReady_,on,error_,muted,paused,isFillscreen,duration,obj,currentTime,utag_data*/
/**
 * Created by clarkj on 12/29/2015.
 * Customized for DigiKey SOW 3933
 * Customized to pick up Digi-Key utag data
 */
/*
 * Modified by LunaMetrics to bypass WebTrends (8/17)
 */
/*
 * TAKEN FROM -- http://www.digikey.com/webtrends.bc.js
  Requirements
 *
 *  Brightcove - new Player (vidoejs v4.2)
 *
 * ---------------------------------------------------------------------------------------------------
 ** NOTE: the playerId must be set to reflect that the developer used for the site.
 * ---------------------------------------------------------------------------------------------------
 * Configuration Options
 *
 *  load: 			load event tracking valid values {true:false} default true
 *  loadMeta: 	    loaded meta data event tracking valid values {true:false} default false
 *  pause: 		    pause/resume event tracking valid values {true:false} default true
 *  quartile: 	    quartile event tracking valid values {true:false} default true
 *  beacon: 		beacon tracking valid values {true:false} default true
 *  seek: 			seek event tracking valid values {true:false} default true
 *  beaconRate:     the number of seconds between beacons in seconds values {0-65,000} default 60
 *  pctInc: 		percentage increments for quartile tracking defaule 25
 *  volume: 		volume event and level tracking valid values {true:false} default true
 *  bins: 			bin range for duration tracking in seconds. valid values [0-65000 {default 15})
 *  dcsid: 		    override dcsid
 *  playerId:       the 'id' attribute of the player in the markup.  Default is "[id^='vjs_video']:not([id$='_api'])"
 *  transformCallback:  adds in a transform so the data can be customized before sending (standard Webtrends transform)
 *  trackCallback:  callback function to override standard multiTrack call
 *  nameCallback:   callback for custom video name function
 *  preProcess :    callback to pre-preocess the data before its sent to the tracking call
 *  beaconType:     the type of beaconing - auto: for streaming will use the beaconCurve, and for fix duration the beacon rate
 *  postMessage:    flag to send data via postMessage instead of the tracking call -- default = false
 *  beaconType      auto - for stream media use the beacon curve, curve - always use curve, other values use the beacon ratel
 *  fixed:          uses the beacon duration  for all
 *  beaconCurve: an array of value to use for curvilinear beaconing.  The default table looks like:
 *  {
 *      60:10,
 *      120:20,
 *      300:30,
 *      420:45,
 *      600:60,
 *      1800:150
 *  }
 *
 *
 *  Parameters generated
 *  clip_n: 				clip name
 *  clip_id: 				clip identifier
 *  clip_secs: 				playhead positon in seconds
 *  clip_mins: 				playhead positon in minutes
 *  clip_ct: 				clip content (mp4,mov,avi,...)
 *  clip_perc: 				percentage played
 *  clip_ev: 				event identifier
 *  clip_duration: 		    clips duration
 *  clip_t: 				player media type
 *  clip_player: 			clip player name
 *  clip_vol: 				clip volume level (0 - 100)
 *  clip_res: 				clip fesolugion hxw
 *  clip_player_res: 	    player resolution hxw
 *  clip_duration_n: 	    clip duration bin
 *  clip_tv: 				clip tag version
 *  clip_mode: 				streaming or fixed duration
 *  player_id               BrightCove id for the player
 *  player_playlist_id      BrightCove playlist id
 *  clip_video_id           Brightcove Video id;
 *  clip_accountId          BrightCove account Id;
 *  dl: 					event type 41 - load, 40 for event
 *  clip_tag_*              meta data from BrighCove tag meta attribute
 *
 *
 *
 *
 *  Mod History
 *  v1.0.0BETA    CLARK     Created    02/15/2016
 *  V1.01 CLARK Added tag attribute from bc meta data
 *  V1.01 CLARK Added tag attribute from bc meta data
 *
 *  Implementation example
 *  plugins:{
 *      bc:{src:"webtrends.bc.js",beaconRate:45,load:false,playerId:"[id^='vjs_video']:not([id$='_api'])"},
 *  }
 *
 *
 *
 */
(function() {
    var cache = [],
        config = {
            load: true,
            loadMeta: false,
            pause: true,
            quartile: true,
            beacon: true,
            beaconType: 'auto',
            seek: true,
            mute: true,
            beaconRate: 30,
            pctInc: 25,
            volume: true,
            bins: 15,
            fullscreen: true,
            dcsid: null,
            //playerId: "[id^='vjs_video']:not([id$='_api'])",
            //playerId: "[id='vjs_video_3'],[id='popup-player']",
            playerId: "[id='popup-player'],[id^='vjs_video']:not([id$='_api'])",
            //playerId: [id='vjs_video_3','popup-player'],
            //playerId: "[id^='vjs_video'],[id^='popup-player']:not([id$='_api'])",
            trackCallback: function(o) {

              o.transform(null, o);

              var obj = {};
              var key;
              var val;
              var i;

              for (i = 0; i < o.argsa.length; i = i + 2) {

                key = o.argsa[i];
                val = o.argsa[i + 1];

                obj[key] = val;

              }

              //utag.link({
                //event_name: 'bc_video',
                //video_title: obj['WT.clip_n'],
                //video_event: obj['WT.z_event_type'],
                //video_source: obj['WT.z_video_source'],
                //playlist_id: obj['WT.z_playlist_id'],
                //video_id: obj['WT.z_video_id'],
                //supplier_id: obj['WT.z_supplier_id']
              //});
              
              //utag.dkVideo(config.playerId,obj);
              //utag.dkVideo("[id^='vjs_video'],[id^='popup-player']:not([id$='_api'])",obj);
              utag.dkVideo("[id^='vjs_video'],[id^='popup-player']",obj);
              //utag.dkVideo("[id^='vjs_video']:not([id$='_api'])",obj);

            },
            nameCallback: null,
            preProcess: function(state) {
                state.data.WT.z_video_id = state.data.WT.clip_video_id;
                state.data.WT.z_en_title = state.data.WT.clip_n;
                if (state.data.WT.z_clip_tag_supplierid !== undefined) {
                    state.data.WT.z_supplier_id = state.data.WT.z_clip_tag_supplierid
                }
                state.data.WT.z_clip_tag_supplierid = null;
                state.data.WT.player_id = null;
                state.data.WT.player_playlist_id = null;
                state.data.WT.clip_video_id = null;
                state.data.WT.clip_accountId = null;
                state.data.WT.ti = state.data.WT.clip_n;
                state.data.WT.z_page_id = state.data.WT.clip_video_id;
                if (utag_data !== undefined) {
                    if (utag_data.page_site !== undefined) {
                        state.data.WT.site = utag_data.page_site;
                        state.data.WT.z_site_id = utag_data.page_site
                    }
                    if (utag_data.page_language !== undefined) {
                        state.data.WT.z_lang = utag_data.page_language
                    }
                }
            },
            transformCallback: function(d, o) {
                var n;
                for (n = o.argsa.length - 2; n > 0; n -= 2) {
                    switch (o.argsa[n]) {
                        case 'WT.clip_ev':
                            switch (o.argsa[n + 1]) {
                                case 'Play':
                                    o.argsa.push('WT.z_event_type', "mediaBegin");
                                    break;
                                case 'Pause':
                                    o.argsa.push('WT.z_event_type', "mediaStop");
                                    break;
                                case 'Resume':
                                    o.argsa.push('WT.z_event_type', "mediaPlay");
                                    break;
                                case 'Complete':
                                    o.argsa.push('WT.z_event_type', "mediaComplete");
                                    break;
                                case 'Seek':
                                    o.argsa.push('WT.z_event_type', "mediaSeek");
                                    break;
                                case 'Load':
                                    o.argsa.push('WT.z_event_type', "playerLoad");
                                    break
                            }
                            break
                    }
                }
                o.argsa.push('WT.cg_n', 'Video');
                o.argsa.push('WT.cg_s', 'Brightcove');
                o.argsa.push('DCS.dcssip', 'www.digikey.com');
                o.argsa.push('DCS.dcsuri', 'brightcove/video.html');
                o.argsa.push('WT.z_page_type', 'VI');
                o.argsa.push('WT.z_page_sub_type', 'BC')
            },
            beaconCurve: {
                60: 10,
                120: 20,
                300: 30,
                420: 45,
                600: 60,
                1800: 150
            },
            postMessage: false,
            basic: false
        },
        H5v_states = function() {
            this.data = {
                WT: {
                    clip_n: null,
                    clip_id: null,
                    clip_secs: null,
                    clip_mins: null,
                    clip_ct: null,
                    clip_perc: null,
                    clip_ev: null,
                    clip_duration: null,
                    clip_t: 'Flash',
                    clip_player: "Brightcove vjs",
                    clip_provider: null,
                    clip_vol: null,
                    clip_res: null,
                    clip_player_res: null,
                    clip_q: null,
                    clip_duration_n: null,
                    clip_tv: "1.0.1",
                    clip_mode: "FixedDuration",
                    clip_player_ver: null,
                    player_id: null,
                    player_playlist_id: null,
                    clip_video_id: null,
                    clip_accountId: null,
                    dl: 41
                },
                _state: 0,
                _lastBeacon: 0,
                _lastQuartile: 0,
                _volSettle: -1,
                _seekSettle: 0,
                _loaded: false,
                _duration: null,
                _timer: null,
                _start: false,
                _sentLoad: false,
                _myBeaconRate: 0,
                _isMuted: false,
                player: null,
                _fullScreen: false,
                _isValidMeta: false,
                _waitMeta: false
            }
        },
        bc_callback = function(states, ev, playerId) {
            var e = document.getElementById(playerId),
                obj = videojs(playerId).player(),
                clipTrack = false,
                currentTime = obj.currentTime(),
                beaconAtInterval, btime, bn, delta, mediaData, playerdata, tags = [],
                tag, tagName, tagValue, n;
            if (config.basic) {
                states.data.WT.clip_tv += "basic"
            }
            if (currentTime && states.data._duration && states.data._duration > 0) {
                states.data.WT.clip_perc = Math.floor((currentTime / states.data._duration) * 100)
            } else {
                states.data.WT.clip_perc = null
            }
            switch (ev) {
                case 'play':
                    if (states.data._state === 'play' || !states.data._loaded || states.data._waitMeta || states.data._seekSettle > 0) {
                        break
                    }
                    if (!states.data._isValidMeta && obj.isReady_) {
                        bc_callback(states, 'meta2', playerId)
                    }
                    if (states.data._state === 'pause' || states.data._state === 'stop') {
                        if (config.pause) {
                            states.data.WT.clip_ev = 'Resume';
                            clipTrack = true;
                            states.data._state = 'play'
                        }
                    } else {
                        states.data.WT.clip_ev = 'Play';
                        states.data._loaded = true;
                        clipTrack = true;
                        states.data._state = 'play';
                        states.data._lastBeacon = 0;
                        states.data._lastQuartile = 0;
                        states.data.WT.clip_perc = '0';
                        states.data.WT.clip_secs = null;
                        states.data.WT.clip_mins = null;
                        states.data.WT.clip_res = obj.height() + "x" + obj.width();
                        states.data._seekSettle = 55
                    }
                    states.data._start = true;
                    break;
                case 'loadstart3':
                case 'meta':
                case 'meta2':
                    mediaData = obj.mediainfo;
                    if (mediaData === undefined) {
                        break
                    }
                    playerdata = obj.options();
                    if (obj.currentType()) {
                        states.data.WT.clip_ct = obj.currentType()
                    }
                    if (typeof obj.techName_ === 'string') {
                        states.data.WT.clip_t = obj.techName_.toUpperCase()
                    }
                    states.data.WT.clip_id = mediaData.id;
                    states.data.WT.player_id = playerdata['data-player'];
                    if (!config.basic) {
                        states.data.WT.clip_player_ver = videojs.VERSION;
                        states.data.WT.player_playlist_id = playerdata['data-playlist-id'];
                        states.data.WT.clip_video_id = mediaData.id;
                        states.data.WT.clip_accountId = mediaData.account_id
                    }
                    if (mediaData.duration !== undefined) {
                        states.data.WT.clip_duration = Math.floor(mediaData.duration * 100) / 100;
                        states.data._duration = mediaData.duration
                    } else {
                        states.data.WT.clip_duration = null;
                        states.data._duration = null
                    }
                    if (!states.data._duration) {
                        states.data.WT.clip_mode = 'Stream';
                        states.data.WT.clip_duration = null;
                        states.data._duration = null
                    }
                    states.data._myBeaconRate = config.beaconRate;
                    states.data.WT.clip_n = mediaData.name !== undefined ? mediaData.name : mediaData.src.split('/')[mediaData.src.split('/').length - 1].split('.')[0];
                    //states.data.WT.clip_n = mediaData.name !== undefined && mediaData.src !== undefined ? mediaData.name : mediaData.src.split('/')[mediaData.src.split('/').length - 1].split('.')[0];
                    if (typeof config.nameCallback === 'function') {
                        states.data.WT.clip_n = config.nameCallback(obj)
                    }
                    if (config.bins && states.data._duration && states.data._duration > 0) {
                        bn = Math.floor((states.data._duration + config.bins) / config.bins);
                        states.data.WT.clip_durration_n = (bn - 1) * config.bins + '-' + bn * config.bins
                    }
                    try {
                        if (mediaData.tags !== undefined && mediaData.tags.length > 0) {
                            for (n = 0; n < mediaData.tags.length; n++) {
                                tagName = mediaData.tags[n].split('=')[0];
                                tagValue = mediaData.tags[n].split('=').length > 1 ? mediaData.tags[n].split('=')[1] : "0";
                                states.data.WT['z_clip_tag_' + tagName] = tagValue
                            }
                        }
                    } catch (ignore) {}
                    states.data._fullScreen = obj.isFullscreen();
                    if (!states.data._sentLoad) {
                        bc_callback(states, 'load', playerId)
                    } else {
                        states.data._loaded = true
                    }
                    states.data._isValidMeta = true;
                    states.data._waitMeta = false;
                    if (config.loadMeta) {
                        clipTrack = true;
                        states.data.WT.clip_ev = 'Meta'
                    }
                    break;
                case 'pause':
                    if (states.data._seekSettle > 0) {
                        return
                    }
                    if (states.data.WT.clip_perc > 97) {
                        bc_callback(states, 'complete', playerId)
                    } else {
                        if (!states.data._loaded || !states.data._start || !(states.data._state !== 'start' || states.data._state !== 'begin')) {
                            break
                        }
                        if (config.pause) {
                            clipTrack = true
                        }
                        states.data._state = ev;
                        states.data.WT.clip_ev = 'Pause'
                    }
                    break;
                case 'end':
                case 'complete':
                    states.data._state = ev;
                    bc_callback(states, 'timeupdate', playerId);
                    if (states.data.WT.clip_ev !== 'Complete') {
                        clipTrack = true
                    }
                    states.data.WT.clip_ev = 'Complete';
                    states.data.WT.clip_perc = 100;
                    states.data._start = false;
                    states.data._loaded = false;
                    states.data._isValidMeta = false;
                    break;
                case 'error':
                    clipTrack = true;
                    states.data.WT.clip_ev = 'Error ';
                    states.data.WT.clip_ev += obj.error_.type;
                    break;
                case 'timeupdate':
                case 'progress':
                    if (states.data._seekSettle > 0) {
                        states.data._seekSettle -= 1
                    }
                    delta = (Math.floor(currentTime * 100) / 100) - states.data.WT.clip_secs;
                    if (Math.abs(delta) > 5) {
                        bc_callback(states, 'seek', playerId)
                    }
                    states.data.WT.clip_secs = Math.floor(currentTime * 100) / 100;
                    if (!config.basic) {
                        states.data.WT.clip_mins = Math.floor((currentTime / 60) * 100) / 100
                    }
                    if (config.quartile) {
                        if (states.data.WT.clip_perc >= states.data._lastQuartile + config.pctInc) {
                            states.data.WT.clip_perc = Math.floor(states.data.WT.clip_perc / config.pctInc) * config.pctInc;
                            states.data._lastQuartile = states.data.WT.clip_perc;
                            states.data.WT.clip_ev = 'Quartile';
                            clipTrack = true;
                            break
                        }
                        if (states.data._state === 'complete' && states.data._lastQuartile !== 100) {
                            states.data.WT.clip_perc = 100;
                            states.data._lastQuartile = states.data.WT.clip_perc;
                            states.data.WT.clip_ev = 'Quartile';
                            clipTrack = true
                        }
                    }
                    if (config.beacon && !config.basic) {
                        try {
                            if ((config.beaconType.toLowerCase() === 'auto' && states.data.WT.clip_mode !== null && states.data.WT.clip_mode.toLowerCase() === "stream") || config.beaconType === 'curve') {
                                beaconAtInterval = config.beaconRate;
                                for (btime in config.beaconCurve) {
                                    if (currentTime <= btime) {
                                        beaconAtInterval = config.beaconCurve[btime];
                                        break
                                    }
                                }
                                if (currentTime === 0) {
                                    states.data._lastBeacon = 0
                                }
                                states.data._myBeaconRate = beaconAtInterval
                            } else {
                                states.data._myBeaconRate = config.beaconRate
                            }
                        } catch (err) {
                            states.data._myBeaconRate = config.beaconRate
                        }
                        if (currentTime > states.data._lastBeacon + states.data._myBeaconRate) {
                            states.data.WT.clip_ev = 'Beacon';
                            clipTrack = true;
                            states.data._lastBeacon += states.data._myBeaconRate;
                            break
                        }
                    }
                    if (states.data._volSettle > 0) {
                        states.data._volSettle -= 1
                    }
                    if (states.data._volSettle === 0) {
                        states.data.WT.clip_ev = 'Volume';
                        clipTrack = true
                    }
                    if (states.data._state === 'ended') {
                        states.data.WT.clip_ev = 'complete';
                        states.data.WT.clip_secs = '0';
                        if (!config.basic) {
                            states.data.WT.clip_mins = '0'
                        }
                        states.data.WT.clip_perc = null;
                        clipTrack = false;
                        states.data._state = 'ended'
                    }
                    if (config.bins && states.data._duration > 0) {
                        bn = Math.floor((states.data._duration + config.bins) / config.bins);
                        states.data.WT.clip_durration_n = (bn - 1) * config.bins + '-' + bn * config.bins
                    }
                    if (obj.muted() !== states.data._isMuted) {
                        bc_callback(states, 'mute', playerId)
                    }
                    if (!states.data._start && !obj.paused()) {
                        bc_callback(states, 'play', playerId)
                    }
                    if (states.data._fullScreen !== obj.isFullscreen() && obj.readyState() > 0) {
                        states.data._fullScreen = obj.isFullscreen();
                        if (obj.isFullscreen()) {
                            bc_callback(states, 'fullscreen', playerId)
                        } else {
                            bc_callback(states, 'fullscreenexit', playerId)
                        }
                    }
                    if (obj.volume() !== undefined && !config.basic) {
                        states.data.WT.clip_vol = obj.volume() * 100
                    }
                    break;
                case 'seek':
                    if (currentTime === 0 && states.data._state === 'play') {
                        states.data.WT.clip_secs = '0';
                        if (!config.basic) {
                            states.data.WT.clip_mins = '0'
                        }
                        states.data.WT.clip_perc = null;
                        clipTrack = false;
                        states.data._state = 'ended';
                        states.data._isValidMeta = false;
                        states.data._waitMeta = true;
                        states.data._start = false
                    }
                    if (currentTime === 0 && states.data._state !== 'play') {
                        states.data._state = 'Ended';
                        break
                    }
                    states.data._lastBeacon = currentTime;
                    if (states.data._state !== 'ended' && config.seek && states.data._seekSettle === 0 && currentTime !== 0) {
                        states.data.WT.clip_ev = 'Seek';
                        clipTrack = true;
                        states.data._seekSettle = 50
                    }
                    states.data._state = ev;
                    break;
                case 'loadstart':
                    if (config.load) {
                        states.data._sentLoad = false;
                        clipTrack = false
                    }
                    break;
                case 'load':
                    if (config.load && !states.data._sentLoad) {
                        states.data.WT.clip_ev = 'Load';
                        clipTrack = true
                    }
                    states.data._sentLoad = true;
                    states.data._loaded = true;
                    break;
                case 'fullscreen':
                    if (config.fullscreen && obj.isFullscreen() && !config.basic) {
                        states.data.WT.clip_ev = 'Fullscreen';
                        clipTrack = true
                    }
                    break;
                case 'fullscreenexit':
                    if (config.fullscreen && !obj.isFullscreen() && !config.basic) {
                        states.data.WT.clip_ev = 'Fullscreenexit';
                        clipTrack = true
                    }
                    break;
                case 'mute':
                    if (config.mute && !config.basic) {
                        if (obj.muted()) {
                            states.data.WT.clip_ev = 'Muted'
                        } else {
                            states.data.WT.clip_ev = 'UnMuted'
                        }
                        states.data._isMuted = obj.muted();
                        clipTrack = true
                    }
                    break;
                default:
                    break
            }
            if (clipTrack) {
                if (typeof config.preProcess === 'function') {
                    config.preProcess(states)
                }
                for (tag in states.data.WT) {
                    tags.push('WT.' + tag);
                    tags.push(states.data.WT[tag] === "null" ? '' : states.data.WT[tag])
                }
                cache.push({
                    element: this,
                    argsa: tags,
                    transform: config.transformCallback,
                    data: states.data.WT,
                    states: states.data,
                    videoEle: e
                });
                if (config.postMessage) {
                    window.parent.postMessage([tags], "*")
                }
                if (typeof config.trackCallback === 'function') {
                    config.trackCallback(cache.pop())
                } else if (typeof Webtrends !== 'undefined') {
                    Webtrends.multiTrack(cache.pop())
                }
            }
            states.data.WT.dl = 40
        },
        player_bind = function(playerId) {
            var eventBinds = {
                'begin': {
                    v4: 'onBegin'
                },
                'pause': {
                    v4: 'pause'
                },
                'play': {
                    v4: 'play'
                },
                'end': {
                    v4: 'ended'
                },
                'mute': {
                    v4: 'mute'
                },
                'unmute': {
                    v4: 'onUnmute'
                },
                'complete': {
                    v4: 'onLastSecond'
                },
                'loadalldata': {
                    v4: 'loadelldata'
                },
                'loadstart': {
                    v4: 'loadstart'
                },
                'loadstart3': {
                    v4: 'loadeddata'
                },
                'loadedalldata': {
                    v4: 'loadedalldata'
                },
                'resize': {
                    v4: 'resize'
                },
                'progress': {
                    v4: 'progress'
                },
                'wait': {
                    v4: 'waiting'
                },
                'seek': {
                    v4: 'onSeek'
                },
                'beforeSeek': {
                    v4: 'onBeforeSeek'
                },
                'error': {
                    v4: 'error'
                },
                'meta': {
                    v4: 'loadedmetadata'
                },
                'timeupdate': {
                    v4: 'timeupdate'
                }
            };
            try {
                videojs(playerId).ready(function() {
                    var s = new H5v_states(),
                        myPlayer = this,
                        action;
                    s.data.player = this;
                    for (action in eventBinds) {
                        if (eventBinds[action].v4 !== null) {
                            (function(action, eventBinds, playerId, s, myPlayer) {
                                myPlayer.on(eventBinds[action].v4, function() {
                                    bc_callback(s, action, playerId)
                                })
                            }(action, eventBinds, playerId, s, myPlayer))
                        }
                    }
                    if (videojs(playerId).isReady_) {
                        bc_callback(s, 'meta2', playerId)
                    }
                })
            } catch (ignore) {}
        },
        waitForPlayer = function(pollTimeOut) {
            setTimeout(function() {
                var players = document.querySelectorAll(config.playerId),
                    n;
                if (window.videojs !== undefined && document.querySelectorAll(config.playerId + ':not([data-wt="ytBound"])').length > 0) {
                    for (n = 0; n < players.length; n++) {
                        players[n].setAttribute("data-wt", "Bound");
                        player_bind(players[n].id)
                    }
                } else {
                    if (pollTimeOut === "infinite" || pollTimeOut-- > 0) {
                        waitForPlayer(pollTimeOut)
                    }
                }
            }, 500)
        },
        Init = function(t, p) {
            var i, pollTimeOut = 20;
            for (i in config) {
                if (p !== undefined && p[i] !== undefined) {
                    config[i] = p[i]
                }
            }
            if (this.dcsid !== undefined) {
                this.dcsid = p.dcsid || t.dcsid
            }
            if (config.pollTime !== undefined) {
                pollTimeOut = config.pollTime
            }
            waitForPlayer(pollTimeOut)
        };
    Init();
}());