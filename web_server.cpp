#include "web_server.h"
#include "macro_store.h"
#include "ui.h"
#include "config.h"
#include "energy_save.h"
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "esp_system.h"
#include "soc/rtc_cntl_reg.h"

// ── HTTP server instance ──────────────────────────────────────────────────────
static WebServer server(WEB_SERVER_PORT);

// ── Embedded HTML page ────────────────────────────────────────────────────────
static const char HTML_PAGE[] = R"rawliteral(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>m5Macro</title>
<style>
*{box-sizing:border-box;margin:0;padding:0}
body{background:#0a0a0a;color:#e0e0e0;font-family:'Courier New',monospace;font-size:13px;display:flex;flex-direction:column;height:100vh;overflow:hidden}
header{background:#111111;padding:10px 14px;display:flex;align-items:center;gap:10px;border-bottom:1px solid #222222;flex-shrink:0}
h1{color:#ffffff;font-size:16px;letter-spacing:2px}
.sub{color:#444444;font-size:11px}
.conn{width:7px;height:7px;border-radius:50%;background:#aaaaaa;flex-shrink:0;transition:background .3s}
.conn.off{background:#444444}
.ip-badge{color:#cccccc;font-size:11px;margin-left:auto;background:#1a1a1a;padding:3px 8px;border-radius:3px;border:1px solid #333333}
.hbg{display:none;flex-direction:column;gap:4px;background:none;border:1px solid #222222;border-radius:4px;padding:6px 7px;cursor:pointer;flex-shrink:0}
.hbg span{display:block;width:18px;height:2px;background:#606060;border-radius:1px;transition:background .1s}
.hbg:hover span{background:#a0a0a0}
.settings-btn{background:none;border:1px solid #222222;border-radius:4px;color:#888888;cursor:pointer;font-size:16px;padding:5px 9px;line-height:1;flex-shrink:0}
.settings-btn:hover{color:#e0e0e0;border-color:#444444}
/* Settings Modal */
#settings-modal{display:none;position:fixed;inset:0;background:rgba(0,0,0,.85);z-index:200;align-items:center;justify-content:center}
#settings-modal.show{display:flex}
.modal-box{background:#111111;border:1px solid #333333;border-radius:6px;width:320px;max-width:95vw;max-height:90vh;overflow-y:auto;padding:20px;display:flex;flex-direction:column;gap:14px}
.modal-hdr{display:flex;align-items:center;justify-content:space-between}
.modal-ttl{color:#ffffff;font-size:14px;letter-spacing:1px}
.modal-x{background:none;border:none;color:#666666;font-size:18px;cursor:pointer;padding:0 4px;line-height:1}
.modal-x:hover{color:#e0e0e0}
.modal-fg{display:flex;flex-direction:column;gap:5px}
.modal-lbl{color:#666666;font-size:10px;text-transform:uppercase;letter-spacing:1px}
.modal-inp{background:#0a0a0a;border:1px solid #333333;color:#e0e0e0;padding:9px 10px;border-radius:4px;width:100%;font-family:inherit;font-size:13px}
.modal-inp:focus{outline:none;border-color:#888888}
.modal-sep{border:none;border-top:1px solid #222222}
.modal-row{display:flex;gap:8px}
.btn-modal-save{padding:9px;background:#333333;color:#ffffff;border:none;border-radius:4px;cursor:pointer;font-family:inherit;font-size:12px;font-weight:bold;text-transform:uppercase;letter-spacing:.5px}
.btn-modal-save:hover{background:#444444}
.btn-restart{flex:1;padding:9px;background:#1a1a1a;color:#cccccc;border:1px solid #333333;border-radius:4px;cursor:pointer;font-family:inherit;font-size:11px;font-weight:bold;text-transform:uppercase;letter-spacing:.5px}
.btn-restart:hover{background:#222222;color:#ffffff;border-color:#555555}
.btn-boot{flex:1;padding:9px;background:#1a1a1a;color:#888888;border:1px solid #2a2a2a;border-radius:4px;cursor:pointer;font-family:inherit;font-size:11px;font-weight:bold;text-transform:uppercase;letter-spacing:.5px}
.btn-boot:hover{background:#222222;color:#cccccc;border-color:#444444}
.modal-st{font-size:11px;min-height:14px;color:#aaaaaa;text-align:center}
/* Overlay */
#ov{display:none;position:fixed;inset:0;background:rgba(0,0,0,.65);z-index:90}
#ov.show{display:block}
/* Layout */
.layout{display:flex;flex:1;overflow:hidden}
/* Sidebar */
.sidebar{width:190px;background:#0d0d0d;border-right:1px solid #222222;display:flex;flex-direction:column;flex-shrink:0;transition:transform .22s ease}
.sb-hdr{display:flex;align-items:center;gap:6px;padding:11px 10px 5px;border-bottom:1px solid #1a1a1a}
.sb-ttl{color:#444444;font-size:10px;text-transform:uppercase;letter-spacing:1.5px;flex:1}
.mc{background:#1a1a1a;color:#555555;font-size:10px;padding:1px 6px;border-radius:10px}
.sb-x{display:none;background:none;border:none;color:#555555;font-size:18px;cursor:pointer;padding:0 4px;line-height:1}
.sb-x:hover{color:#a0a0a0}
.macro-list{flex:1;overflow-y:auto;padding:4px 4px 0;-webkit-overflow-scrolling:touch}
.mi{padding:10px;border-radius:4px;cursor:pointer;color:#666666;font-size:12px;white-space:nowrap;overflow:hidden;text-overflow:ellipsis;border-left:2px solid transparent;display:flex;align-items:center;gap:5px;min-height:44px;transition:background .1s,color .1s}
.mi:hover{background:#151515;color:#aaaaaa}
.mi.active{background:#1a1a1a;color:#e0e0e0;border-left-color:#888888}
.mi-num{color:#333333;font-size:10px;min-width:14px;text-align:right;flex-shrink:0}
.mi-name{overflow:hidden;text-overflow:ellipsis;flex:1}
.ud{color:#aaaaaa;font-size:8px;flex-shrink:0;opacity:0}
.ud.show{opacity:1}
.sb-foot{padding:8px}
/* Main */
.main{flex:1;display:flex;flex-direction:column;overflow:hidden;min-width:0}
.editor{flex:1;padding:14px 16px;display:flex;flex-direction:column;gap:10px;overflow-y:auto;-webkit-overflow-scrolling:touch}
.ph{display:flex;flex-direction:column;align-items:center;justify-content:center;flex:1;gap:8px;color:#222222}
.ph-i{font-size:26px}.ph-t{font-size:12px}
/* Form */
.fg{display:flex;flex-direction:column;gap:5px}
.fh{display:flex;align-items:center;justify-content:space-between}
label{color:#555555;font-size:10px;text-transform:uppercase;letter-spacing:1px}
.sc{color:#333333;font-size:10px}
input[type=text]{background:#0d0d0d;border:1px solid #222222;color:#e0e0e0;padding:9px 10px;border-radius:4px;width:100%;font-family:inherit;font-size:16px;transition:border-color .15s;-webkit-appearance:none}
input[type=text]:focus{outline:none;border-color:#888888}
/* Step rows */
.step-list{display:flex;flex-direction:column;gap:5px}
.sr{display:flex;flex-wrap:wrap;align-items:center;gap:6px;background:#0d0d0d;border:1px solid #1a1a1a;border-radius:5px;padding:8px 9px}
.sr:hover{border-color:#2a2a2a}
.type-sel{flex:1;min-width:0}
.sr-acts{display:flex;gap:4px;margin-left:auto;flex-shrink:0}
.sr-cnt{width:100%;display:flex;align-items:center;gap:6px;flex-wrap:wrap}
.si{background:#0e0e0e;border:1px solid #222222;color:#cccccc;padding:9px 10px;border-radius:4px;font-family:inherit;font-size:16px;cursor:pointer;min-height:44px;-webkit-appearance:none}
.si:focus{outline:none;border-color:#888888}
.key-sel{flex:1;min-width:0}
.text-in{flex:1;min-width:0;background:#0e0e0e;border:1px solid #222222;color:#e0e0e0;padding:9px 10px;border-radius:4px;font-family:inherit;font-size:16px;min-height:44px;-webkit-appearance:none}
.text-in:focus{outline:none;border-color:#888888}
.delay-in{width:90px;background:#0e0e0e;border:1px solid #222222;color:#cccccc;padding:9px 10px;border-radius:4px;font-family:inherit;font-size:16px;min-height:44px;-webkit-appearance:none}
.delay-in:focus{outline:none;border-color:#888888}
input[type=number]::-webkit-outer-spin-button,input[type=number]::-webkit-inner-spin-button{-webkit-appearance:none}
input[type=number]{-moz-appearance:textfield}
.ms-lbl{color:#444444;font-size:12px;flex-shrink:0}
.mods{display:flex;gap:4px;flex-wrap:wrap}
.mb{padding:9px 12px;background:#141414;border:1px solid #222222;color:#555555;border-radius:4px;cursor:pointer;font-size:13px;font-family:inherit;user-select:none;min-height:44px;display:flex;align-items:center;transition:background .1s,color .1s,border-color .1s}
.mb.on{background:#1a1a1a;color:#e0e0e0;border-color:#555555}
.plus-lbl{color:#444444;font-size:15px;flex-shrink:0}
.ob{width:40px;height:40px;font-size:13px;background:#141414;border:1px solid #222222;color:#444444;border-radius:4px;cursor:pointer;display:flex;align-items:center;justify-content:center;flex-shrink:0;font-family:inherit}
.ob:hover:not([disabled]){color:#888888;border-color:#333333;background:#1a1a1a}
.ob[disabled]{opacity:.3;cursor:default}
.rm{width:44px;height:40px;background:#1a0a0a;border:1px solid #2a1010;color:#664444;border-radius:4px;cursor:pointer;font-size:16px;display:flex;align-items:center;justify-content:center;flex-shrink:0}
.rm:hover{color:#cc6060;border-color:#441515;background:#220a0a}
.add-btn{padding:10px 16px;background:#0f0f0f;border:1px solid #222222;color:#444444;border-radius:5px;cursor:pointer;font-family:inherit;font-size:13px;margin-top:2px;min-height:44px;width:100%;text-align:center}
.add-btn:hover{color:#888888;border-color:#333333}
.toolbar{display:flex;gap:8px;align-items:center;flex-wrap:wrap;padding-top:4px}
.btn-save{padding:10px 18px;background:#333333;color:#ffffff;border:none;border-radius:5px;cursor:pointer;font-family:inherit;font-size:13px;font-weight:bold;letter-spacing:.5px;text-transform:uppercase;min-height:44px}
.btn-save:hover{background:#444444}
.btn-del{padding:10px 16px;background:#1a0a0a;color:#aa4444;border:1px solid #2a1010;border-radius:5px;cursor:pointer;font-family:inherit;font-size:13px;font-weight:bold;letter-spacing:.5px;text-transform:uppercase;min-height:44px}
.btn-del:hover{color:#dd6060}
.btn-new{padding:10px 16px;background:#141414;color:#666666;border:1px solid #222222;border-radius:5px;cursor:pointer;font-family:inherit;font-size:13px;font-weight:bold;letter-spacing:.5px;text-transform:uppercase;min-height:44px}
.btn-new:hover{color:#aaaaaa}
.kh{font-size:10px;color:#222222;margin-left:auto}
.status{font-size:11px;min-height:16px;padding:2px 0;transition:opacity .5s}
::-webkit-scrollbar{width:4px}::-webkit-scrollbar-track{background:#0a0a0a}::-webkit-scrollbar-thumb{background:#222222;border-radius:2px}
.es-sw{position:relative;display:inline-block;width:36px;height:20px;flex-shrink:0}
.es-sw input{opacity:0;width:0;height:0}
.es-sl{position:absolute;cursor:pointer;inset:0;background:#2a2a2a;border-radius:20px;transition:.2s}
.es-sl:before{content:"";position:absolute;width:14px;height:14px;left:3px;bottom:3px;background:#555;border-radius:50%;transition:.2s}
input:checked+.es-sl{background:#444}
input:checked+.es-sl:before{transform:translateX(16px);background:#e0e0e0}
.es-row{display:flex;align-items:center;gap:8px}
.es-rng{flex:1;accent-color:#888;cursor:pointer}
.es-val{color:#666;font-size:11px;min-width:26px;text-align:right;flex-shrink:0}
@media(min-width:600px){
  .hbg{display:none}
  .sb-x{display:none!important}
  .sub{display:inline}
  .sr{flex-wrap:nowrap}
  .type-sel{width:95px;flex:0 0 95px}
  .sr-acts{margin-left:0}
  .sr-cnt{width:auto;flex:1}
  .si,.text-in,.delay-in{font-size:13px;min-height:34px;padding:5px 8px}
  input[type=text]{font-size:13px;padding:8px 10px}
  .ob{width:30px;height:30px;font-size:11px}
  .rm{width:34px;height:30px;font-size:13px}
  .mb{min-height:32px;padding:4px 9px;font-size:11px}
  .btn-save,.btn-del,.btn-new{min-height:34px;font-size:11px;padding:6px 14px}
  .add-btn{min-height:34px;font-size:11px;padding:6px 12px;width:auto}
  .mi{min-height:32px;padding:7px 10px}
}
@media(max-width:599px){
  .hbg{display:flex}
  .sub{display:none}
  .sb-x{display:block}
  .ip-badge{font-size:10px;padding:3px 6px}
  .sidebar{position:fixed;top:0;left:0;bottom:0;z-index:100;transform:translateX(-100%);width:220px;box-shadow:4px 0 24px rgba(0,0,0,.8)}
  .sidebar.open{transform:translateX(0)}
  .editor{padding:12px}
}
</style>
</head>
<body>
<div id="ov" onclick="closeSB()"></div>
<div id="settings-modal">
  <div class="modal-box">
    <div class="modal-hdr">
      <span class="modal-ttl">&#9881; Settings</span>
      <button class="modal-x" onclick="closeSettings()">&#10005;</button>
    </div>
    <div class="modal-fg">
      <label class="modal-lbl">WiFi SSID</label>
      <input type="text" class="modal-inp" id="cfg-ssid" maxlength="32" placeholder="m5Macro" autocomplete="off">
    </div>
    <div class="modal-fg">
      <label class="modal-lbl">WiFi Password (min. 8 characters)</label>
      <input type="password" class="modal-inp" id="cfg-pass" maxlength="63" placeholder="(leave unchanged)" autocomplete="new-password">
    </div>
    <hr class="modal-sep">
    <button class="btn-modal-save" onclick="saveSettings()" style="width:100%">&#10003; Save &amp; Restart</button>
    <div class="modal-row">
      <button class="btn-restart" onclick="doRestart()">&#8635; Restart</button>
      <button class="btn-boot" onclick="doBootloader()">&#9660; Bootloader</button>
    </div>
    <hr class="modal-sep">
    <div class="modal-fg">
      <div style="display:flex;align-items:center;justify-content:space-between">
        <label class="modal-lbl" style="margin:0">&#9889; Energy Saving</label>
        <label class="es-sw"><input type="checkbox" id="cfg-es-en"><span class="es-sl"></span></label>
      </div>
    </div>
    <div class="modal-fg">
      <label class="modal-lbl">Inactivity until dim (seconds)</label>
      <input type="number" class="modal-inp" id="cfg-es-to" min="5" max="3600" value="30">
    </div>
    <div class="modal-fg">
      <label class="modal-lbl">Dim brightness (0–255)</label>
      <div class="es-row">
        <input type="range" class="es-rng" id="cfg-es-db" min="0" max="255" value="10" oninput="document.getElementById('cfg-es-dbv').textContent=this.value">
        <span class="es-val" id="cfg-es-dbv">10</span>
      </div>
    </div>
    <div class="modal-fg">
      <label class="modal-lbl">Normal brightness (10–255)</label>
      <div class="es-row">
        <input type="range" class="es-rng" id="cfg-es-ab" min="10" max="255" value="128" oninput="document.getElementById('cfg-es-abv').textContent=this.value">
        <span class="es-val" id="cfg-es-abv">128</span>
      </div>
    </div>
    <button class="btn-modal-save" onclick="saveEnergy()" style="width:100%">&#9889; Save energy settings</button>
    <div class="modal-st" id="modal-st"></div>
  </div>
</div>
<header>
  <button class="hbg" onclick="openSB()" aria-label="Menu"><span></span><span></span><span></span></button>
  <div class="conn" id="conn"></div>
  <h1>m5Macro</h1><span class="sub">Editor</span>
  <span class="ip-badge" id="ip-badge">192.168.4.1</span>
  <button class="settings-btn" onclick="openSettings()" title="Settings">&#9881;</button>
</header>
<div class="layout">
  <div class="sidebar" id="sb">
    <div class="sb-hdr">
      <span class="sb-ttl">Macros</span>
      <span class="mc" id="mc">0</span>
      <button class="sb-x" onclick="closeSB()">&#10005;</button>
    </div>
    <div class="macro-list" id="ml"></div>
    <div class="sb-foot">
      <button class="btn-new" onclick="newMacro()" style="width:100%;text-align:center">+ New</button>
    </div>
  </div>
  <div class="main">
    <div class="editor" id="editor">
      <div class="ph"><div class="ph-i">&#9000;</div><div class="ph-t">Select a macro or create a new one</div></div>
    </div>
  </div>
</div>
<script>
var macros=[],cur=-1,dirty=false,steps=[],stTimer=null;
var KEYS=['ENTER','ESC','BACKSPACE','TAB','SPACE','DELETE','INSERT','UP','DOWN','LEFT','RIGHT','HOME','END','PGUP','PGDN','F1','F2','F3','F4','F5','F6','F7','F8','F9','F10','F11','F12','CAPSLOCK','PRINTSCREEN','SCROLLLOCK','PAUSE','NUMLOCK','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','0','1','2','3','4','5','6','7','8','9'];
var MODS=['CTRL','SHIFT','ALT','WIN'];

function esc(s){return(s||'').replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;').replace(/"/g,'&quot;');}
function setOnline(on){document.getElementById('conn').classList.toggle('off',!on);}
function openSB(){document.getElementById('sb').classList.add('open');document.getElementById('ov').classList.add('show');}
function closeSB(){document.getElementById('sb').classList.remove('open');document.getElementById('ov').classList.remove('show');}

function openSettings(){
  Promise.all([
    fetch('/api/settings').then(function(r){return r.json();}).catch(function(){return{};}),
    fetch('/api/energy').then(function(r){return r.json();}).catch(function(){return{};})
  ]).then(function(res){
    var j=res[0],e=res[1];
    document.getElementById('cfg-ssid').value=j.ssid||'';
    document.getElementById('cfg-pass').value='';
    document.getElementById('modal-st').textContent='';
    if(e.enabled!==undefined)document.getElementById('cfg-es-en').checked=!!e.enabled;
    if(e.timeout_s!==undefined)document.getElementById('cfg-es-to').value=e.timeout_s;
    if(e.dim_br!==undefined){document.getElementById('cfg-es-db').value=e.dim_br;document.getElementById('cfg-es-dbv').textContent=e.dim_br;}
    if(e.active_br!==undefined){document.getElementById('cfg-es-ab').value=e.active_br;document.getElementById('cfg-es-abv').textContent=e.active_br;}
    document.getElementById('settings-modal').classList.add('show');
  });
}
function closeSettings(){document.getElementById('settings-modal').classList.remove('show');}

function mst(msg){document.getElementById('modal-st').textContent=msg;}

async function saveSettings(){
  var ssid=(document.getElementById('cfg-ssid').value||'').trim();
  var pass=document.getElementById('cfg-pass').value||'';
  if(!ssid){mst('SSID cannot be empty.');return;}
  if(pass.length>0&&pass.length<8){mst('Password must be at least 8 characters.');return;}
  mst('Saving...');
  var body=new URLSearchParams({ssid:ssid,pass:pass});
  try{
    var r=await fetch('/api/settings',{method:'POST',body:body});
    var j=await r.json();
    if(j.ok){mst('Saved! Device is restarting...');}
    else mst('Error: '+(j.err||'?'));
  }catch(e){mst('Connection lost – restarting.');}
}

async function doRestart(){
  if(!confirm('Restart device now?'))return;
  mst('Restarting...');
  try{await fetch('/api/restart',{method:'POST'});}catch(e){}
  mst('Device is restarting...');
}

async function doBootloader(){
  if(!confirm('Switch to bootloader mode?'))return;
  mst('Bootloader...');
  try{await fetch('/api/restart-bootloader',{method:'POST'});}catch(e){}
  mst('Device is switching to bootloader mode...');
}

function parseSteps(raw){
  var out=[];
  (raw||'').split('\n').forEach(function(ln){
    ln=ln.trim();
    if(!ln||ln[0]==='#')return;
    if(ln.indexOf('TEXT:')=== 0){out.push({type:'text',text:ln.substring(5)});return;}
    if(ln.indexOf('DELAY:')=== 0){out.push({type:'delay',ms:parseInt(ln.substring(6))||100});return;}
    var p=ln.split('+');
    if(p.length>1){
      var mods=p.slice(0,-1).map(function(m){
        m=m.trim().toUpperCase();
        if(m==='LCTRL'||m==='RCTRL')return'CTRL';
        if(m==='LSHIFT'||m==='RSHIFT')return'SHIFT';
        if(m==='LALT'||m==='RALT')return'ALT';
        if(m==='RWIN'||m==='GUI')return'WIN';
        return m;
      }).filter(function(m){return MODS.indexOf(m)>=0;});
      if(mods.length){out.push({type:'combo',mods:mods,key:p[p.length-1].trim().toUpperCase()});return;}
    }
    out.push({type:'key',key:ln.toUpperCase()});
  });
  return out;
}

function serializeSteps(arr){
  return arr.map(function(s){
    if(s.type==='text')return'TEXT:'+s.text;
    if(s.type==='delay')return'DELAY:'+s.ms;
    if(s.type==='combo')return(s.mods.length?s.mods.join('+')+'+':'')+s.key;
    return s.key;
  }).join('\n');
}

function kOpts(sel){
  var list=KEYS.slice();
  if(sel&&list.indexOf(sel)<0)list.unshift(sel);
  return list.map(function(k){return'<option value="'+k+'"'+(k===sel?' selected':'')+'>'+k+'</option>';}).join('');
}

function renderRow(s,i){
  var n=steps.length;
  var r='<div class="sr">'
    +'<select class="si type-sel" onchange="chType('+i+',this.value)">'
    +'<option value="key"'+(s.type==='key'?' selected':'')+'>Key</option>'
    +'<option value="combo"'+(s.type==='combo'?' selected':'')+'>Combo</option>'
    +'<option value="text"'+(s.type==='text'?' selected':'')+'>Text</option>'
    +'<option value="delay"'+(s.type==='delay'?' selected':'')+'>Delay</option>'
    +'</select>'
    +'<div class="sr-acts">'
    +'<button class="ob" onclick="mv('+i+',-1)"'+(i===0?' disabled':'')+' title="Up">&#9650;</button>'
    +'<button class="ob" onclick="mv('+i+',1)"'+(i===n-1?' disabled':'')+' title="Down">&#9660;</button>'
    +'<button class="rm" onclick="rmStep('+i+')" title="Remove">&#10007;</button>'
    +'</div>'
    +'<div class="sr-cnt">';
  if(s.type==='key'){
    r+='<select class="si key-sel" onchange="steps['+i+'].key=this.value;markDirty()">'+kOpts(s.key||'ENTER')+'</select>';
  }else if(s.type==='combo'){
    r+='<div class="mods">';
    MODS.forEach(function(m){r+='<button class="mb'+(s.mods&&s.mods.indexOf(m)>=0?' on':'')+'" onclick="togMod('+i+',\''+m+'\')">'+m+'</button>';});
    r+='</div><span class="plus-lbl">+</span><select class="si" style="min-width:80px" onchange="steps['+i+'].key=this.value;markDirty()">'+kOpts(s.key||'C')+'</select>';
  }else if(s.type==='text'){
    r+='<input type="text" class="text-in" placeholder="Enter text..." value="'+esc(s.text||'')+'" oninput="steps['+i+'].text=this.value;markDirty()" autocomplete="off">';
  }else{
    r+='<input type="number" class="delay-in" min="1" max="9999" value="'+(s.ms||100)+'" oninput="steps['+i+'].ms=parseInt(this.value)||1;markDirty()"><span class="ms-lbl">ms</span>';
  }
  r+='</div></div>';
  return r;
}

function renderSteps(){
  var sc=document.getElementById('sc');
  if(sc)sc.textContent=steps.length+(steps.length===1?' step':' steps');
  var el=document.getElementById('step-list');
  if(el)el.innerHTML=steps.map(renderRow).join('');
}

function addStep(){
  steps.push({type:'key',key:'ENTER'});
  markDirty();renderSteps();
  var el=document.getElementById('step-list');
  if(el&&el.lastElementChild)el.lastElementChild.scrollIntoView({block:'nearest',behavior:'smooth'});
}
function rmStep(i){steps.splice(i,1);markDirty();renderSteps();}
function mv(i,d){var j=i+d;if(j<0||j>=steps.length)return;var t=steps[i];steps[i]=steps[j];steps[j]=t;markDirty();renderSteps();}
function chType(i,t){var s=steps[i];steps[i]=t==='key'?{type:'key',key:s.key||'ENTER'}:t==='combo'?{type:'combo',mods:['CTRL'],key:s.key||'C'}:t==='text'?{type:'text',text:s.text||''}:{type:'delay',ms:s.ms||500};markDirty();renderSteps();}
function togMod(i,mod){var s=steps[i];if(!s.mods)s.mods=[];var x=s.mods.indexOf(mod);if(x>=0)s.mods.splice(x,1);else s.mods.push(mod);markDirty();renderSteps();}

async function load(){
  try{
    var r=await fetch('/api/macros');macros=await r.json();setOnline(true);
    document.getElementById('mc').textContent=macros.length;renderList();
    var ip=r.headers.get('X-Device-IP');if(ip)document.getElementById('ip-badge').textContent=ip;
  }catch(e){setOnline(false);st('Connection error','#cc4444');}
}

function renderList(){
  document.getElementById('ml').innerHTML=macros.map(function(m,i){
    return'<div class="mi'+(i===cur?' active':'')+'" onclick="selMacro('+i+')" title="'+esc(m.name)+'">'
      +'<span class="mi-num">'+(i+1)+'</span>'
      +'<span class="mi-name">'+esc(m.name||'(no name)')+'</span>'
      +'<span class="ud'+(i===cur&&dirty?' show':'')+'">&#9679;</span></div>';
  }).join('');
}

function markDirty(){if(!dirty){dirty=true;renderList();}}

function edHTML(m,isNew){
  return'<div class="fg"><label>Name</label>'
    +'<input type="text" id="m-name" maxlength="32" value="'+(isNew?'':esc(m.name))+'" placeholder="'+(isNew?'Macro name':'')+'" oninput="markDirty()" autocomplete="off"></div>'
    +'<div class="fg"><div class="fh"><label>Steps</label><span class="sc" id="sc"></span></div>'
    +'<div class="step-list" id="step-list"></div>'
    +'<button class="add-btn" onclick="addStep()">+ Add step</button></div>'
    +'<div class="toolbar">'
    +(isNew?'<button class="btn-save" onclick="save()">+ Create</button>'
           :'<button class="btn-save" onclick="save()">&#10003; Save</button><button class="btn-del" onclick="del()">&#10007; Delete</button>')
    +'<span class="kh">Strg+S</span></div><div class="status" id="st"></div>';
}

function selMacro(i){
  if(dirty&&cur>=0&&!confirm('Discard changes?'))return;
  dirty=false;cur=i;renderList();closeSB();
  steps=parseSteps(macros[i].steps);
  document.getElementById('editor').innerHTML=edHTML(macros[i],false);
  renderSteps();
}

function newMacro(){
  if(dirty&&cur>=0&&!confirm('Discard changes?'))return;
  dirty=false;cur=-1;steps=[];renderList();closeSB();
  document.getElementById('editor').innerHTML=edHTML(null,true);
  renderSteps();markDirty();
}

async function save(){
  var name=(document.getElementById('m-name')||{}).value||'';
  name=name.trim();if(!name){st('Name cannot be empty.','#cc4444');return;}
  var body=new URLSearchParams({id:cur,name:name,steps:serializeSteps(steps)});
  try{
    var r=await fetch('/api/save',{method:'POST',body:body});var j=await r.json();
    if(j.ok){dirty=false;st('Saved!','#aaaaaa');var pv=cur;await load();cur=(pv===-1)?macros.length-1:pv;renderList();}
    else st('Error: '+(j.err||'?'),'#cc4444');
  }catch(e){st('Network error','#cc4444');}
}

async function del(){
  if(cur<0)return;
  if(!confirm('Delete macro "'+macros[cur].name+'"?'))return;
  try{
    var r=await fetch('/api/delete',{method:'POST',body:new URLSearchParams({id:cur})});var j=await r.json();
    if(j.ok){dirty=false;cur=-1;steps=[];await load();document.getElementById('editor').innerHTML='<div class="ph"><div class="ph-i">&#10006;</div><div class="ph-t">Deleted</div></div>';}
    else st('Error: '+(j.err||'?'),'#cc4444');
  }catch(e){st('Network error','#cc4444');}
}

function st(msg,color){
  var el=document.getElementById('st');if(!el)return;
  clearTimeout(stTimer);el.style.opacity='1';el.style.color=color||'#aaaaaa';el.textContent=msg;
  if(color==='#aaaaaa')stTimer=setTimeout(function(){el.style.opacity='0';},2200);
}

async function saveEnergy(){
  var en=document.getElementById('cfg-es-en').checked?'1':'0';
  var to=parseInt(document.getElementById('cfg-es-to').value)||30;
  var db=parseInt(document.getElementById('cfg-es-db').value)||10;
  var ab=parseInt(document.getElementById('cfg-es-ab').value)||128;
  mst('Saving...');
  try{
    var r=await fetch('/api/energy',{method:'POST',body:new URLSearchParams({enabled:en,timeout_s:to,dim_br:db,active_br:ab})});
    var j=await r.json();
    if(j.ok)mst('Energy settings saved!');
    else mst('Error: '+(j.err||'?'));
  }catch(e){mst('Connection error');}
}

document.addEventListener('keydown',function(e){if((e.ctrlKey||e.metaKey)&&e.key==='s'){e.preventDefault();save();}});
load();
</script>
</body>
</html>)rawliteral";

// ── JSON helper functions ─────────────────────────────────────────────────────

static String json_str(const String& s) {
    String out = "\"";
    for (int i = 0; i < (int)s.length(); i++) {
        char c = s[i];
        if (c == '"')       out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "";
        else                out += c;
    }
    out += "\"";
    return out;
}

// ── HTTP handlers ─────────────────────────────────────────────────────────────

static void handle_root() {
    server.send(200, "text/html; charset=utf-8", HTML_PAGE);
}

static void handle_api_macros() {
    String json = "[";
    int cnt = macro_store_count();
    for (int i = 0; i < cnt; i++) {
        const MacroInfo* m = macro_store_get(i);
        if (!m) continue;
        if (i > 0) json += ",";
        json += "{\"id\":" + String(i) +
                ",\"name\":" + json_str(String(m->name)) +
                ",\"steps\":" + json_str(macro_store_get_steps_raw(i)) +
                "}";
    }
    json += "]";
    server.sendHeader("X-Device-IP", web_server_ip());
    server.send(200, "application/json", json);
}

static void handle_api_save() {
    if (!server.hasArg("name") || !server.hasArg("steps") || !server.hasArg("id")) {
        server.send(400, "application/json", "{\"ok\":false,\"err\":\"Missing parameters\"}");
        return;
    }
    int  id    = server.arg("id").toInt();
    String name  = server.arg("name");
    String steps = server.arg("steps");

    name.trim();
    if (name.length() == 0) {
        server.send(400, "application/json", "{\"ok\":false,\"err\":\"Name empty\"}");
        return;
    }

    int result = macro_store_save(id, name, steps);
    if (result < 0) {
        server.send(500, "application/json", "{\"ok\":false,\"err\":\"Save failed\"}");
        return;
    }

    ui_reload();
    server.send(200, "application/json", "{\"ok\":true,\"id\":" + String(result) + "}");
}

static void handle_api_delete() {
    if (!server.hasArg("id")) {
        server.send(400, "application/json", "{\"ok\":false,\"err\":\"Missing ID\"}");
        return;
    }
    int id = server.arg("id").toInt();
    if (!macro_store_delete(id)) {
        server.send(500, "application/json", "{\"ok\":false,\"err\":\"Delete failed\"}");
        return;
    }

    ui_reload();
    server.send(200, "application/json", "{\"ok\":true}");
}

static void handle_api_status() {
    String json = "{";
    json += "\"fs_ok\":"    + String(macro_store_fs_ok() ? "true" : "false");
    json += ",\"count\":"   + String(macro_store_count());
    json += ",\"files\":"   + json_str(macro_store_list_files());
    json += "}";
    server.send(200, "application/json", json);
}

static void handle_api_settings_get() {
    Preferences prefs;
    prefs.begin("wifi", true);
    String ssid = prefs.getString("ssid", WIFI_AP_SSID);
    prefs.end();
    server.send(200, "application/json", "{\"ssid\":" + json_str(ssid) + "}");
}

static void handle_api_settings_post() {
    if (!server.hasArg("ssid")) {
        server.send(400, "application/json", "{\"ok\":false,\"err\":\"Missing parameters\"}");
        return;
    }
    String ssid = server.arg("ssid");
    String pass = server.hasArg("pass") ? server.arg("pass") : "";
    ssid.trim();

    if (ssid.length() == 0) {
        server.send(400, "application/json", "{\"ok\":false,\"err\":\"SSID empty\"}");
        return;
    }
    if (pass.length() > 0 && pass.length() < 8) {
        server.send(400, "application/json", "{\"ok\":false,\"err\":\"Password too short (min. 8 characters)\"}");
        return;
    }

    Preferences prefs;
    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    if (pass.length() >= 8) {
        prefs.putString("pass", pass);
    }
    prefs.end();

    server.send(200, "application/json", "{\"ok\":true}");
    delay(500);
    ESP.restart();
}

static void handle_api_restart() {
    server.send(200, "application/json", "{\"ok\":true}");
    delay(500);
    ESP.restart();
}

static void handle_api_restart_bootloader() {
    server.send(200, "application/json", "{\"ok\":true}");
    delay(500);
    // Sets the FORCE_DOWNLOAD_BOOT flag so the ROM bootloader is active on next start
    REG_WRITE(RTC_CNTL_OPTION1_REG, RTC_CNTL_FORCE_DOWNLOAD_BOOT);
    esp_restart();
}

static void handle_api_energy_get() {
    Preferences prefs;
    prefs.begin("energy", true);
    bool     enabled   = prefs.getBool("enabled",   true);
    uint32_t timeout_s = prefs.getUInt("timeout_s", ENERGY_SAVE_TIMEOUT_DEFAULT);
    uint8_t  dim_br    = prefs.getUInt("dim_br",    ENERGY_SAVE_DIM_BRIGHTNESS);
    uint8_t  active_br = prefs.getUInt("active_br", ENERGY_SAVE_ACTIVE_BRIGHTNESS);
    prefs.end();
    String json = "{\"enabled\":"    + String(enabled ? "true" : "false") +
                  ",\"timeout_s\":"  + String(timeout_s) +
                  ",\"dim_br\":"     + String(dim_br) +
                  ",\"active_br\":"  + String(active_br) + "}";
    server.send(200, "application/json", json);
}

static void handle_api_energy_post() {
    Preferences prefs;
    prefs.begin("energy", false);
    if (server.hasArg("enabled"))
        prefs.putBool("enabled", server.arg("enabled") == "1");
    if (server.hasArg("timeout_s")) {
        int t = server.arg("timeout_s").toInt();
        if (t < 5)   t = 5;
        if (t > 3600) t = 3600;
        prefs.putUInt("timeout_s", (uint32_t)t);
    }
    if (server.hasArg("dim_br"))
        prefs.putUInt("dim_br", (uint32_t)constrain(server.arg("dim_br").toInt(), 0, 255));
    if (server.hasArg("active_br"))
        prefs.putUInt("active_br", (uint32_t)constrain(server.arg("active_br").toInt(), 10, 255));
    prefs.end();

    energy_save_init();   // Apply new settings immediately
    server.send(200, "application/json", "{\"ok\":true}");
}

static void handle_not_found() {
    server.send(404, "text/plain", "Not found");
}

// ── Public API ────────────────────────────────────────────────────────────────

void web_server_init() {
    // Read SSID and password from NVS (fallback: config.h constants)
    Preferences prefs;
    prefs.begin("wifi", true);
    String ssid = prefs.getString("ssid", WIFI_AP_SSID);
    String pass = prefs.getString("pass", WIFI_AP_PASS);
    prefs.end();

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid.c_str(), pass.c_str());

    server.on("/",                       HTTP_GET,  handle_root);
    server.on("/api/macros",             HTTP_GET,  handle_api_macros);
    server.on("/api/status",             HTTP_GET,  handle_api_status);
    server.on("/api/settings",           HTTP_GET,  handle_api_settings_get);
    server.on("/api/save",               HTTP_POST, handle_api_save);
    server.on("/api/delete",             HTTP_POST, handle_api_delete);
    server.on("/api/settings",           HTTP_POST, handle_api_settings_post);
    server.on("/api/restart",            HTTP_POST, handle_api_restart);
    server.on("/api/restart-bootloader", HTTP_POST, handle_api_restart_bootloader);
    server.on("/api/energy",             HTTP_GET,  handle_api_energy_get);
    server.on("/api/energy",             HTTP_POST, handle_api_energy_post);
    server.onNotFound(handle_not_found);

    server.begin();
}

void web_server_handle() {
    server.handleClient();
}

String web_server_ip() {
    return WiFi.softAPIP().toString();
}
